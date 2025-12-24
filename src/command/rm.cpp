#include "../include/command/rm.h"
#include "StagingArea.h"
#include "Commit.h"
#include "Utils.h"
#include "Repository.h"
#include <filesystem>
#include <iostream>

void rmcommand::rm(const std::string& filename) {
    // 获取状态
    bool is_staged = StagingArea::isFileStaged(filename);          // 文件是否在暂存区
    bool is_tracked = Commit::isFileTrackedInCurrentCommit(filename); // 文件是否被当前提交跟踪
    std::string workdir_file = Utils::join(Repository::getWorkingDir(), filename); // 工作目录文件路径

    if (!is_staged && !is_tracked) {
        throw GitliteException("No reason to remove the file.");
    }
    // 暂存区有该文件 未被当前提交跟踪
    if (is_staged && !is_tracked) {
        StagingArea::unstageFile(filename);
        std::cout << "Unstaged file: " << filename << std::endl;
        return;
    }

    //被当前提交跟踪 暂存为待删除 移除工作目录文件
    if (is_tracked) {
        StagingArea::markFileForDeletion(filename);
        if (Utils::isFile(workdir_file)) {
            if (Utils::restrictedDelete(workdir_file)) {
                std::cout << "Removed file from working directory: " << filename << std::endl;
            } else {
                throw GitliteException("Failed to remove file from working directory: " + filename);
            }
        }

        std::cout << "Marked file for deletion: " << filename << std::endl;
    }
}