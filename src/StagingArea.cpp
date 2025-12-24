#include "../include/StagingArea.h"
#include "../include/Utils.h"
#include "../include/Repository.h"

#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
static std::string getStagingRootDir() {
    return Utils::join(Repository::getGitliteDir(), "staging");
}
static std::string getStagingDeleteDir() {
    return Utils::join(getStagingRootDir(), "delete");
}
// 暂存文件
void StagingArea::stageFile(const std::string& filename) {
    // 创建暂存区根目录
    std::string staging_root = getStagingRootDir();
    if (!Utils::createDirectories(staging_root)) {
        throw std::runtime_error("Failed to create staging directory: " + staging_root);
    }
    removeDeletionMark(filename);

    // 读取工作目录文件内容，写入暂存区
    std::string dest_path = Utils::join(staging_root, filename);
    std::string content = Utils::readContentsAsString(filename);
    Utils::writeContents(dest_path, content);
}
// 取消暂存文件
void StagingArea::unstageFile(const std::string& filename) {
    std::string dest_path = Utils::join(getStagingRootDir(), filename);
    Utils::restrictedDelete(dest_path);
}
// 检查文件是否已暂存
bool StagingArea::isFileStaged(const std::string& filename) {
    std::string dest_path = Utils::join(getStagingRootDir(), filename);
    return Utils::isFile(dest_path);
}

// 标记文件为待删除
void StagingArea::markFileForDeletion(const std::string& filename) {
    std::string delete_dir = getStagingDeleteDir();
    if (!Utils::createDirectories(delete_dir)) {
        throw std::runtime_error("Failed to create delete directory: " + delete_dir);
    }
    unstageFile(filename);
    std::string delete_mark_path = Utils::join(delete_dir, filename);
    Utils::writeContents(delete_mark_path, "");
}
// 检查文件是否被标记为待删除
bool StagingArea::isFileMarkedForDeletion(const std::string& filename) {
    std::string delete_mark_path = Utils::join(getStagingDeleteDir(), filename);
    return Utils::isFile(delete_mark_path);
}
// 移除文件的删除标记
void StagingArea::removeDeletionMark(const std::string& filename) {
    std::string delete_mark_path = Utils::join(getStagingDeleteDir(), filename);
    Utils::restrictedDelete(delete_mark_path);
}
// 获取所有已暂存的文件列表
std::vector<std::string> StagingArea::getStagedFiles() {
    std::vector<std::string> staged_files;
    std::string staging_root = getStagingRootDir();

    if (!Utils::isDirectory(staging_root)) {
        return staged_files;
    }
    for (const auto& entry : std::filesystem::directory_iterator(staging_root)) {
        if (entry.path().filename() == "delete") {
            continue;
        }
        if (Utils::isFile(entry.path().string())) {
            staged_files.push_back(entry.path().filename().string());
        }
    }

    return staged_files;
}

//获取所有标记为待删除的文件列表
std::vector<std::string> StagingArea::getFilesMarkedForDeletion() {
    std::vector<std::string> deleted_files;
    std::string delete_dir = getStagingDeleteDir();
    if (!Utils::isDirectory(delete_dir)) {
        return deleted_files;
    }
    for (const auto& entry : std::filesystem::directory_iterator(delete_dir)) {
        if (Utils::isFile(entry.path().string())) {
            deleted_files.push_back(entry.path().filename().string());
        }
    }

    return deleted_files;
}
// 检查暂存区是否为空
bool StagingArea::isStagingAreaEmpty() {
    if (!getStagedFiles().empty()) {
        return false;
    }
    if (!getFilesMarkedForDeletion().empty()) {
        return false;
    }
    return true;
}

// 清空暂存区
void StagingArea::clear() {
    std::string staging_root = getStagingRootDir();
    if (Utils::isDirectory(staging_root)) {
        for (const auto& entry : std::filesystem::directory_iterator(staging_root)) {
            if (entry.path().filename() == "delete") {
                for (const auto& del_entry : std::filesystem::directory_iterator(entry.path())) {
                    Utils::restrictedDelete(del_entry.path().string());
                }
            } else {
                Utils::restrictedDelete(entry.path().string());
            }
        }
    }
}

// 获取暂存文件对应的 Blob SHA
std::string StagingArea::getBlobShaForFile(const std::string& filename) {
    std::string staged_file_path = Utils::join(getStagingRootDir(), filename);
    if (!Utils::isFile(staged_file_path)) {
        throw std::runtime_error("File not staged: " + filename);
    }
    std::string content = Utils::readContentsAsString(staged_file_path);
    return Utils::sha1(content);
}