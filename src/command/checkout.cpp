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

std::string checkoutcommand::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}
std::string checkoutcommand::resolveCommitSha(const std::string &short_sha)
{

    if (short_sha.length() == 40)
    {
        return short_sha;
    }

    std::string objects_dir = Repository::getObjectsDir();
    std::vector<std::string> matching_commits;

    // 遍历 objects 目录查找匹配的 commit
    for (const auto &dir_entry : std::filesystem::directory_iterator(objects_dir))
    {
        if (dir_entry.is_directory())
        {
            std::string subdir = dir_entry.path().filename().string();
            // 检查目录名是否以 short_sha 的前2位开头
            if (short_sha.length() >= 2 && subdir != short_sha.substr(0, 2))
            {
                continue;
            }
            // 遍历子目录中的文件
            std::vector<std::string> files = Utils::plainFilenamesIn(dir_entry.path().string());

            for (const auto &file : files)
            {
                std::string full_sha = subdir + file;
                // 检查是否是 commit 对象
                std::string object_path = Utils::join(objects_dir, subdir, file);

                try
                {
                    std::string content = Utils::readContentsAsString(object_path);

                    if (content.find("commit ") == 0)
                    {
                        // 检查 SHA 是否匹配缩写
                        if (full_sha.find(short_sha) == 0)
                        {
                            matching_commits.push_back(full_sha);
                        }
                    }
                }
                catch (...)
                {
                    // 跳过无法读取的文件
                    continue;
                }
            }
        }
    }

    if (matching_commits.empty())
    {
        Utils::exitWithMessage("No commit with that id exists.");
    }

    std::sort(matching_commits.begin(), matching_commits.end());
    return matching_commits[0];
}

// 检查提交是否存在
bool checkoutcommand::commitExists(const std::string &commit_sha)
{
    try
    {
        std::string resolved = resolveCommitSha(commit_sha);
        std::string object_path = Repository::getObjectsDir() + "/" +
                                  resolved.substr(0, 2) + "/" +
                                  resolved.substr(2);
        return Utils::exists(object_path);
    }
    catch (...)
    {
        return false;
    }
}

// 从提交中获取文件内容
std::string checkoutcommand::getFileFromCommit(const std::string &commit_sha, const std::string &filename)
{

    std::string resolved_sha = resolveCommitSha(commit_sha);

    // 获取提交中的文件列表
    auto file_blobs = Commit::loadFileBlobs(resolved_sha);

    auto it = file_blobs.find(filename);

    if (it == file_blobs.end())
    {
        Utils::exitWithMessage("File does not exist in that commit.");
    }

    std::string blob_sha = it->second;
    std::string objects_dir = Repository::getObjectsDir();
    std::string blob_path = objects_dir + "/" + blob_sha.substr(0, 2) + "/" + blob_sha.substr(2);

    if (!Utils::exists(blob_path))
    {
        std::vector<std::string> subdirs = Utils::plainFilenamesIn(objects_dir);
        for (const auto &subdir : subdirs)
        {
            std::string subdir_path = Utils::join(objects_dir, subdir);
            if (Utils::isDirectory(subdir_path))
            {
                std::vector<std::string> files = Utils::plainFilenamesIn(subdir_path);
            }
        }
        Utils::exitWithMessage("File blob not found.");
    }

    // 读取 blob 内容
    std::string content;
    content = Utils::readContentsAsString(blob_path);

    size_t null_pos = content.find('\0');
    return content.substr(null_pos + 1);
}

void checkoutcommand::checkout(const std::string &commit_id, const std::string &filename)
{
    // 检查提交是否存在
    if (!commitExists(commit_id))
    {
        Utils::exitWithMessage("No commit with that id exists.");
    }

    std::string file_content = getFileFromCommit(commit_id, filename);
    std::filesystem::path filepath(filename);
    if (filepath.has_parent_path())
    {
        std::filesystem::create_directories(filepath.parent_path());
    }
    Utils::writeContents(filename, file_content);
}
void checkoutcommand::checkout(const std::string &filename)
{
    std::string head_sha = Commit::getCurrentCommitSha();

    checkout(head_sha, filename);
}
void checkoutcommand::checkoutBranch(const std::string &branch_name)
{
    std::string actual_branch_name = branch_name;
    
    std::string gitlite_dir = Repository::getGitliteDir();
    if (!Utils::exists(gitlite_dir))
    {
        Utils::exitWithMessage("Not in an initialized Gitlite directory.");
    }

    //检查分支是否存在
    std::string refs_dir = Utils::join(Repository::getGitliteDir(), "refs");
    std::string heads_dir = Utils::join(refs_dir, "heads");
    std::string branch_file = Utils::join(heads_dir, actual_branch_name);
    bool is_from_remote = false;
    if (!Utils::exists(branch_file) && actual_branch_name.find('/') != std::string::npos)
    {
        // 尝试作为远程分支处理
        size_t slash_pos = actual_branch_name.find('/');
        std::string remote_name = actual_branch_name.substr(0, slash_pos);
        std::string remote_branch = actual_branch_name.substr(slash_pos + 1);

        std::string remotes_dir = Utils::join(refs_dir, "remotes");
        std::string remote_ref = Utils::join(remotes_dir, remote_name, remote_branch);

        if (Utils::exists(remote_ref))
        {
            std::string local_branch = actual_branch_name;
            std::string local_branch_file = Utils::join(heads_dir, local_branch);

            std::string remote_commit = Utils::readContentsAsString(remote_ref);
            remote_commit.erase(std::remove(remote_commit.begin(), remote_commit.end(), '\n'), remote_commit.end());
            remote_commit.erase(std::remove(remote_commit.begin(), remote_commit.end(), '\r'), remote_commit.end());

            Utils::writeContents(local_branch_file, remote_commit + "\n");

            // 使用远程分支名继续
            actual_branch_name = local_branch;
            branch_file = local_branch_file;

            // 标记这是从远程分支创建的
            is_from_remote = true;
        }
    }
    if (!Utils::exists(branch_file))
    {
        Utils::exitWithMessage("No such branch exists.");
    }

    //检查是否已经在目标分支
    std::string head_path = Utils::join(Repository::getGitliteDir(), "HEAD");
    std::string head_content = Utils::readContentsAsString(head_path);

    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\n'), head_content.end());
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\r'), head_content.end());

    if (head_content.find("ref: refs/heads/") == 0)
    {
        std::string current_branch = head_content.substr(16); // "ref: refs/heads/".length() = 16

        if (!is_from_remote && current_branch == actual_branch_name)
        {
            Utils::exitWithMessage("No need to checkout the current branch.");
        }
    }

    //获取目标提交 SHA
    std::string target_commit_sha = Utils::readContentsAsString(branch_file);
    target_commit_sha.erase(std::remove(target_commit_sha.begin(), target_commit_sha.end(), '\n'), target_commit_sha.end());
    target_commit_sha.erase(std::remove(target_commit_sha.begin(), target_commit_sha.end(), '\r'), target_commit_sha.end());

    if (!commitExists(target_commit_sha))
    {
        Utils::exitWithMessage("Commit referenced by branch does not exist.");
    }
    std::string current_commit_sha = Commit::getCurrentCommitSha();

    std::map<std::string, std::string> current_files;
    if (!current_commit_sha.empty())
    {
        current_files = Commit::loadFileBlobs(current_commit_sha);
    }

    auto target_files = Commit::loadFileBlobs(target_commit_sha);
    checkUntrackedFilesRecursive(current_files, target_files);
    performCheckout(current_files, target_files);
    StagingArea::clear();
    Utils::writeContents(head_path, "ref: refs/heads/" + actual_branch_name + "\n");
}

//递归检查未跟踪文件
void checkoutcommand::checkUntrackedFilesRecursive(
    const std::map<std::string, std::string> &current_files,
    const std::map<std::string, std::string> &target_files)
{

    for (const auto &entry : fs::recursive_directory_iterator("."))
    {
        if (!entry.is_regular_file())
            continue;

        std::string full_path = entry.path().string();
        if (full_path.find(".gitlite") != std::string::npos)
            continue;
        std::string relative_path = full_path.substr(2);

        if (current_files.find(relative_path) == current_files.end() &&
            target_files.find(relative_path) != target_files.end())
        {
            std::cout << "There is an untracked file in the way; delete it, or add and commit it first." << std::endl;
            exit(1);
        }
    }
}

//执行实际的 checkout
void checkoutcommand::performCheckout(
    const std::map<std::string, std::string> &current_files,
    const std::map<std::string, std::string> &target_files)
{

    //删除在当前提交中存在但在目标提交中不存在的文件
    for (const auto &[filename, _] : current_files)
    {
        if (target_files.find(filename) == target_files.end())
        {
            if (Utils::exists(filename))
            {
                Utils::restrictedDelete(filename);
            }
        }
    }
    //写入目标提交中的文件
    for (const auto &[filename, blob_sha] : target_files)
    {
        std::string blob_content = mergecommand::readBlobContent(blob_sha);
        if (blob_content.empty())
        {
            std::string objects_dir = Repository::getObjectsDir();
            std::string blob_path = objects_dir + "/" + blob_sha.substr(0, 2) + "/" + blob_sha.substr(2);

            if (Utils::exists(blob_path))
            {
                std::string raw_content = Utils::readContentsAsString(blob_path);
            }

            Utils::exitWithMessage("Failed to read blob: " + blob_sha);
        }
        size_t null_pos = blob_content.find('\0');
        std::string file_content = blob_content.substr(null_pos + 1);

        fs::path filepath(filename);
        if (filepath.has_parent_path())
        {
            try
            {
                fs::create_directories(filepath.parent_path());
            }
            catch (const fs::filesystem_error &e)
            {
                Utils::exitWithMessage("Failed to create directory for: " + filename);
            }
        }

        // 写入文件
        try
        {
            Utils::writeContents(filename, file_content);
        }
        catch (const GitliteException &e)
        {
            Utils::exitWithMessage("Failed to write file: " + filename);
        }
    }
}