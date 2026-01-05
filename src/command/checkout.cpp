#include "../include/command/checkout.h"
#include "Commit.h"
#include "Utils.h"
#include "../include/command/merge.h"
#include "Repository.h"
#include "GitliteException.h"
#include "StagingArea.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
namespace fs = std::filesystem;

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}
std::string checkoutcommand::resolveCommitSha(const std::string& short_sha) {
    
    if (short_sha.length() == 40) {
        return short_sha;
    }
    
    std::string objects_dir = Repository::getObjectsDir();
    std::vector<std::string> matching_commits;
    
    // 遍历 objects 目录查找匹配的 commit
    for (const auto& dir_entry : std::filesystem::directory_iterator(objects_dir)) {
        if (dir_entry.is_directory()) {
            std::string subdir = dir_entry.path().filename().string();
            // 检查目录名是否以 short_sha 的前2位开头
            if (short_sha.length() >= 2 && subdir != short_sha.substr(0, 2)) {
                continue;
            }
            // 遍历子目录中的文件
            std::vector<std::string> files = Utils::plainFilenamesIn(dir_entry.path().string());
            
            for (const auto& file : files) {
                std::string full_sha = subdir + file;
                // 检查是否是 commit 对象
                std::string object_path = Utils::join(objects_dir, subdir, file);
                
                try {
                    std::string content = Utils::readContentsAsString(object_path);
                    
                    if (content.find("commit ") == 0) {
                        // 检查 SHA 是否匹配缩写
                        if (full_sha.find(short_sha) == 0) {
                            matching_commits.push_back(full_sha);
                        }
                    }
                } catch (...) {
                    // 跳过无法读取的文件
                    continue;
                }
            }
        }
    }
    
    if (matching_commits.empty()) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    
    // 按字母顺序排序，返回第一个
    std::sort(matching_commits.begin(), matching_commits.end());
    return matching_commits[0];
}

// 检查提交是否存在
bool checkoutcommand::commitExists(const std::string& commit_sha) {
    try {
        std::string resolved = resolveCommitSha(commit_sha);
        std::string object_path = Repository::getObjectsDir() + "/" + 
                                 resolved.substr(0, 2) + "/" + 
                                 resolved.substr(2);
        return Utils::exists(object_path);
    } catch (...) {
        return false;
    }
}

// 从提交中获取文件内容
std::string checkoutcommand::getFileFromCommit(const std::string& commit_sha, const std::string& filename) {
    
    std::string resolved_sha = resolveCommitSha(commit_sha);
    
    // 获取提交中的文件列表
    auto file_blobs = Commit::loadFileBlobs(resolved_sha);
    
    
    auto it = file_blobs.find(filename);
    
    if (it == file_blobs.end()) {
        Utils::exitWithMessage("File does not exist in that commit.");
    }
    
    std::string blob_sha = it->second;
    std::string objects_dir = Repository::getObjectsDir();
    std::string blob_path = objects_dir + "/" + blob_sha.substr(0, 2) + "/" + blob_sha.substr(2);
    
    if (!Utils::exists(blob_path)) {
        std::vector<std::string> subdirs = Utils::plainFilenamesIn(objects_dir);
        for (const auto& subdir : subdirs) {
            std::string subdir_path = Utils::join(objects_dir, subdir);
            if (Utils::isDirectory(subdir_path)) {
                std::vector<std::string> files = Utils::plainFilenamesIn(subdir_path);
            }
        }
        Utils::exitWithMessage("File blob not found.");
    }
    
    // 读取 blob 内容
    std::string content;
    content = Utils::readContentsAsString(blob_path);
    
    // 解析 blob 格式
    size_t null_pos = content.find('\0');
    return content.substr(null_pos + 1);
}

// 第一种形式: gitlite checkout [commit_id] -- filename
void checkoutcommand::checkout(const std::string& commit_id, const std::string& filename) {
    // 检查提交是否存在
    if (!commitExists(commit_id)) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    
    // 获取文件内容
    std::string file_content = getFileFromCommit(commit_id, filename);
    
    // 确保父目录存在
    std::filesystem::path filepath(filename);
    if (filepath.has_parent_path()) {
        std::filesystem::create_directories(filepath.parent_path());
    }
    
    // 写入工作目录
    Utils::writeContents(filename, file_content);
}

// 第二种形式: gitlite checkout -- filename
void checkoutcommand::checkout(const std::string& filename) {
    // 获取当前 HEAD 的 commit SHA
    std::string head_sha = Commit::getCurrentCommitSha();
    
    
    // 使用 HEAD 提交
    checkout(head_sha, filename);
}
void checkoutcommand::checkoutBranch(const std::string& branch_name) {
    // === 1. 检查仓库是否已初始化 ===
    std::string gitlite_dir = Repository::getGitliteDir();
    if (!Utils::exists(gitlite_dir)) {
        Utils::exitWithMessage("Not in an initialized Gitlite directory.");
    }
    
    // === 2. 检查分支是否存在 ===
    std::string refs_dir = Utils::join(Repository::getGitliteDir(), "refs");
    std::string heads_dir = Utils::join(refs_dir, "heads");
    std::string branch_file = Utils::join(heads_dir, branch_name);
    if (!Utils::exists(branch_file)) {
        Utils::exitWithMessage("No such branch exists.");
    }

    // === 3. 检查是否已经在目标分支 ===
std::string head_path = Utils::join(Repository::getGitliteDir(), "HEAD");
std::string head_content = Utils::readContentsAsString(head_path);


// 清理 head_content
head_content.erase(std::remove(head_content.begin(), head_content.end(), '\n'), head_content.end());
head_content.erase(std::remove(head_content.begin(), head_content.end(), '\r'), head_content.end());


if (head_content.find("ref: refs/heads/") == 0) {
    std::string current_branch = head_content.substr(16); // "ref: refs/heads/".length() = 16
    
    if (current_branch == branch_name) {
        Utils::exitWithMessage("No need to checkout the current branch.");
    }
}

    // === 4. 获取目标提交 SHA ===
    std::string target_commit_sha = Utils::readContentsAsString(branch_file);
    target_commit_sha.erase(std::remove(target_commit_sha.begin(), target_commit_sha.end(), '\n'), target_commit_sha.end());
    target_commit_sha.erase(std::remove(target_commit_sha.begin(), target_commit_sha.end(), '\r'), target_commit_sha.end());
    
    if (!commitExists(target_commit_sha)) {
        Utils::exitWithMessage("Commit referenced by branch does not exist.");
    }

    // === 5. 获取文件列表 ===
    std::string current_commit_sha = Commit::getCurrentCommitSha();
    
    // 处理可能为空的情况（初始状态）
    std::map<std::string, std::string> current_files;
    if (!current_commit_sha.empty()) {
        current_files = Commit::loadFileBlobs(current_commit_sha);
    }
    
    auto target_files = Commit::loadFileBlobs(target_commit_sha);

    // === 6. 递归检查未跟踪文件 ===
    checkUntrackedFilesRecursive(current_files, target_files);

    // === 7. 执行 checkout ===
    performCheckout(current_files, target_files);

    // === 8. 清空暂存区 ===
    StagingArea::clear();

    // === 9. 更新 HEAD ===
    Utils::writeContents(head_path, "ref: refs/heads/" + branch_name + "\n");
    
    // 可选：输出成功信息
    // std::cout << "Switched to branch '" << branch_name << "'" << std::endl;
}

// 辅助函数：递归检查未跟踪文件
void checkoutcommand::checkUntrackedFilesRecursive(
    const std::map<std::string, std::string>& current_files,
    const std::map<std::string, std::string>& target_files) {
    
    for (const auto& entry : fs::recursive_directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;
        
        std::string full_path = entry.path().string();
        // 跳过 .gitlite 目录
        if (full_path.find(".gitlite") != std::string::npos) continue;
        
        // 转换为相对路径
        std::string relative_path = full_path.substr(2); // 去掉 "./"
        
        // 检查是否是未跟踪文件且会在目标提交中被覆盖
        if (current_files.find(relative_path) == current_files.end() &&
            target_files.find(relative_path) != target_files.end()) {
            std::cout << "There is an untracked file in the way; delete it, or add and commit it first." << std::endl;
            exit(1);
        }
    }
}

// 辅助函数：执行实际的 checkout
void checkoutcommand::performCheckout(
    const std::map<std::string, std::string>& current_files,
    const std::map<std::string, std::string>& target_files) {
    
    // 1. 删除在当前提交中存在但在目标提交中不存在的文件
    for (const auto& [filename, _] : current_files) {
        if (target_files.find(filename) == target_files.end()) {
            if (Utils::exists(filename)) {
                Utils::restrictedDelete(filename);
            }
        }
    }
    
    // 2. 写入目标提交中的文件
    for (const auto& [filename, blob_sha] : target_files) {
        // 读取 blob 内容
        std::string blob_content = mergecommand::readBlobContent(blob_sha);
        if (blob_content.empty()) {
            Utils::exitWithMessage("Failed to read blob: " + blob_sha);
        }
        
        // 解析 blob 内容（去掉 header）
        size_t null_pos = blob_content.find('\0');
        std::string file_content = blob_content.substr(null_pos + 1);
        
        // 创建父目录
        fs::path filepath(filename);
        if (filepath.has_parent_path()) {
            try {
                fs::create_directories(filepath.parent_path());
            } catch (const fs::filesystem_error& e) {
                Utils::exitWithMessage("Failed to create directory for: " + filename);
            }
        }
        
        // 写入文件
        try {
            Utils::writeContents(filename, file_content);
        } catch (const GitliteException& e) {
            Utils::exitWithMessage("Failed to write file: " + filename);
        }
    }
}