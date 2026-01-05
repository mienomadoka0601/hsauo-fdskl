#include "../include/command/add.h"
#include "../include/Commit.h"

void AddCommand::add(const std::string &filename)
{
    if (!Utils::isFile(filename))
    {
        Utils::exitWithMessage("File does not exist.");
    }
    std::string current_content = Utils::readContentsAsString(filename);
    std::string header = "blob " + std::to_string(current_content.size());
    std::string full_blob_data = header + '\0' + current_content;
    std::string current_sha = Utils::sha1(full_blob_data);
    std::string current_commit_sha = Commit::getCurrentCommitSha();

    if (!current_commit_sha.empty())
    {
        // 从当前提交获取文件列表
        auto tracked_files = Commit::loadFileBlobs(current_commit_sha);
        auto it = tracked_files.find(filename);

        if (it != tracked_files.end())
        {
            // 文件已被跟踪，比较 SHA
            if (current_sha == it->second)
            {
                // 内容相同，如果已在暂存区则取消暂存
                if (StagingArea::isFileStaged(filename))
                {
                    StagingArea::unstageFile(filename);
                }
                return;
            }
        }
    }
    StagingArea::stageFile(filename);
}