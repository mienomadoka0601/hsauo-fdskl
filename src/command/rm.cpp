#include "../include/command/rm.h"
#include "StagingArea.h"
#include "Commit.h"
#include "Utils.h"
#include "Repository.h"
#include "GitliteException.h"
#include <iostream>

void rmcommand::rm(const std::string &filename)
{
    // 获取状态
    bool is_staged = StagingArea::isFileStaged(filename);             // 文件是否在暂存区
    bool is_tracked = Commit::isFileTrackedInCurrentCommit(filename); // 文件是否被当前提交跟踪
    if (!is_staged && !is_tracked)
    {
        Utils::exitWithMessage("No reason to remove the file.");
    }
    // 如果文件在当前提交中被跟踪
    if (is_tracked)
    {
        StagingArea::markFileForDeletion(filename);
        if (is_staged)
        {
            StagingArea::unstageFile(filename);
        }

        // 尝试从工作目录移除文件
        if (Utils::exists(filename))
        {
            Utils::restrictedDelete(filename);
        }

        return;
    }

    // 如果文件当前处于暂存区中且未被当前提交跟踪
    if (is_staged && !is_tracked)
    {
        StagingArea::unstageFile(filename);
        return;
    }
}