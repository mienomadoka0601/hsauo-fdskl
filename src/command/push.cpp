#include "../include/command/push.h"
#include "../include/command/remote.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/Branch.h"
#include <iostream>
#include <queue>
#include <set>
#include <filesystem>

void pushcommand::push(const std::string &remote_name, const std::string &remote_branch)
{
    // 获取远程仓库路径
    std::string remote_path = remotecommand::getRemotePath(remote_name);
    if (remote_path.empty())
    {
        std::cout << "Remote directory not found." << std::endl;
        exit(1);
    }

    // 检查远程 .gitlite 目录是否存在
    if (!Utils::exists(remote_path))
    {
        std::cout << "Remote directory not found." << std::endl;
        exit(1);
    }

    // 获取当前分支信息
    std::string current_branch = Branch::getCurrentBranch();
    if (current_branch.empty())
    {
        exit(1);
    }

    std::string local_commit = Branch::getBranchCommit(current_branch);
    if (local_commit.empty())
    {
        exit(1);
    }

    // 获取远程分支当前提交
    std::string remote_heads_dir = Utils::join(remote_path, "refs", "heads");
    std::string remote_branch_path = Utils::join(remote_heads_dir, remote_branch);

    std::string remote_commit;
    if (Utils::exists(remote_branch_path))
    {
        remote_commit = Utils::readContentsAsString(remote_branch_path);
        // 清理换行符
        remote_commit.erase(std::remove(remote_commit.begin(), remote_commit.end(), '\n'), remote_commit.end());
        remote_commit.erase(std::remove(remote_commit.begin(), remote_commit.end(), '\r'), remote_commit.end());
    }

    // 检查远程分支是否在本地历史中
    if (!remote_commit.empty())
    {
        if (!isRemoteBranchInLocalHistory(local_commit, remote_commit))
        {
            std::cout << "Please pull down remote changes before pushing." << std::endl;
            exit(1);
        }
    }
    std::vector<std::string> commits_to_copy = getCommitsToCopy(local_commit, remote_commit);
    // 复制提交和相关对象到远程
    for (const auto &commit_sha : commits_to_copy)
    {
        copyCommitToRemote(commit_sha, remote_path);
    }

    // 更新远程分支指针
    Utils::createDirectories(remote_heads_dir);
    Utils::writeContents(remote_branch_path, local_commit + "\n");
}

bool pushcommand::isRemoteBranchInLocalHistory(const std::string &local_commit,
                                               const std::string &remote_commit)
{
    std::string current = local_commit;

    while (!current.empty())
    {
        if (current == remote_commit)
        {
            return true;
        }

        auto parents = Commit::getCommitParents(current);
        if (parents.empty())
        {
            break;
        }
        current = parents[0];
    }

    return false;
}

void pushcommand::copyObjectToRemote(const std::string &object_sha,
                                     const std::string &remote_path)
{
    if (object_sha.empty() || object_sha.length() != 40)
    {
        return;
    }

    std::string local_objects_dir = Repository::getObjectsDir();
    std::string local_object_path = local_objects_dir + "/" +
                                    object_sha.substr(0, 2) + "/" +
                                    object_sha.substr(2);

    if (!Utils::exists(local_object_path))
    {
        return;
    }

    std::string remote_objects_dir = Utils::join(remote_path, "objects");
    std::string remote_object_dir = Utils::join(remote_objects_dir, object_sha.substr(0, 2));
    std::string remote_object_path = Utils::join(remote_object_dir, object_sha.substr(2));

    // 如果远程已存在，跳过
    if (Utils::exists(remote_object_path))
    {
        return;
    }

    // 复制文件
    std::string content = Utils::readContentsAsString(local_object_path);
    Utils::createDirectories(remote_object_dir);
    Utils::writeContents(remote_object_path, content);
}

void pushcommand::copyCommitToRemote(const std::string &commit_sha,
                                     const std::string &remote_path)
{
    copyObjectToRemote(commit_sha, remote_path);

    std::string tree_sha = Commit::getTreeShaFromCommit(commit_sha);
    if (!tree_sha.empty())
    {
        copyObjectToRemote(tree_sha, remote_path);
        // 复制 tree 中的所有 blob
        auto tree_entries = Commit::loadTreeObject(tree_sha);
        for (const auto &[filename, blob_sha] : tree_entries)
        {
            copyObjectToRemote(blob_sha, remote_path);
        }
    }

    // 递归复制父提交
    auto parents = Commit::getCommitParents(commit_sha);
    for (const auto &parent : parents)
    {
        if (!parent.empty())
        {
            copyCommitToRemote(parent, remote_path);
        }
    }
}

std::vector<std::string> pushcommand::getCommitsToCopy(const std::string &local_commit,
                                                       const std::string &remote_commit)
{
    std::vector<std::string> commits_to_copy;

    if (remote_commit.empty())
    {
        // 远程分支不存在，复制整个历史
        std::string current = local_commit;
        while (!current.empty())
        {
            commits_to_copy.push_back(current);

            auto parents = Commit::getCommitParents(current);
            if (parents.empty())
                break;
            current = parents[0];
        }
    }
    else
    {
        // 只复制新提交
        std::string current = local_commit;
        while (!current.empty() && current != remote_commit)
        {
            commits_to_copy.push_back(current);

            auto parents = Commit::getCommitParents(current);
            if (parents.empty())
                break;
            current = parents[0];
        }
    }

    return commits_to_copy;
}