#include "../include/command/merge.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/StagingArea.h"
#include "../include/Branch.h"
#include "../include/GitliteException.h"

#include <iostream>
#include <queue>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <ctime>
#include <algorithm>
#include <climits>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

std::vector<std::string> mergecommand::getCommitAncestors(const std::string &commit_sha)
{
    std::vector<std::string> ancestors;
    std::unordered_set<std::string> visited;

    std::queue<std::string> q;
    q.push(commit_sha);

    while (!q.empty())
    {
        std::string current = q.front();
        q.pop();

        if (visited.find(current) != visited.end())
        {
            continue;
        }

        visited.insert(current);
        ancestors.push_back(current);

        std::vector<std::string> parents = Commit::getCommitParents(current);
        for (const auto &parent : parents)
        {
            if (!parent.empty())
            {
                q.push(parent);
            }
        }
    }

    return ancestors;
}

std::string mergecommand::findSplitPoint(const std::string &commit1, const std::string &commit2)
{
    if (commit1.empty() || commit2.empty())
        return "";

    // 获取 commit1 的所有祖先
    auto ancestors1 = getCommitAncestors(commit1);
    std::unordered_set<std::string> ancestors1_set(ancestors1.begin(), ancestors1.end());

    // 查找 commit2 的祖先中最近的公共祖先
    std::queue<std::pair<std::string, int>> q;
    std::unordered_set<std::string> visited;

    q.push({commit2, 0});

    std::string best_split = "";
    int best_distance = INT_MAX;

    while (!q.empty())
    {
        auto [current, distance] = q.front();
        q.pop();

        if (visited.find(current) != visited.end())
        {
            continue;
        }
        visited.insert(current);

        // 如果是公共祖先且距离更近
        if (ancestors1_set.find(current) != ancestors1_set.end())
        {
            if (distance < best_distance)
            {
                best_split = current;
                best_distance = distance;
            }
        }

        // 继续向上查找父提交
        std::vector<std::string> parents = Commit::getCommitParents(current);
        for (const auto &parent : parents)
        {
            if (!parent.empty())
            {
                q.push({parent, distance + 1});
            }
        }
    }

    return best_split;
}

std::string mergecommand::readBlobContent(const std::string &blob_sha)
{
    if (blob_sha.empty())
        return "";

    std::string object_path = Repository::getObjectsDir() + "/" +
                              blob_sha.substr(0, 2) + "/" + blob_sha.substr(2);

    if (!Utils::exists(object_path))
    {
        return "";
    }

    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos)
    {
        return "";
    }

    return content.substr(null_pos + 1);
}

bool mergecommand::wouldMergeOverwriteUntrackedFiles(
    const std::map<std::string, std::string> &current_files,
    const std::map<std::string, std::string> &given_files,
    const std::map<std::string, std::string> &split_files)
{

    for (const auto &entry : fs::recursive_directory_iterator("."))
    {
        if (!entry.is_regular_file())
            continue;

        std::string full_path = entry.path().string();
        if (full_path.find(".gitlite") != std::string::npos)
            continue;

        std::string relative_path = full_path.substr(2);

        // 检查文件是否未跟踪
        bool is_untracked = (current_files.find(relative_path) == current_files.end());

        if (is_untracked)
        {
            // 检查文件在split点和given分支的状态
            bool in_split = (split_files.find(relative_path) != split_files.end());
            bool in_given = (given_files.find(relative_path) != given_files.end());

            std::string split_sha = in_split ? split_files.at(relative_path) : "";
            std::string given_sha = in_given ? given_files.at(relative_path) : "";
            if (in_split && (!in_given || given_sha != split_sha))
            {
                return true;
            }
            if (!in_split && in_given)
            {
                return true;
            }
        }
    }

    return false;
}

std::string mergecommand::getCurrentBranchName()
{
    std::string head_path = Utils::join(Repository::getGitliteDir(), "HEAD");
    if (!Utils::exists(head_path))
        return "";

    std::string content = Utils::readContentsAsString(head_path);
    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

    if (content.find("ref: refs/heads/") == 0)
    {
        return content.substr(16);
    }
    return "";
}

void mergecommand::merge(const std::string &branch_name)
{
    // 检查暂存区是否为空
    if (!StagingArea::isStagingAreaEmpty())
    {
        Utils::exitWithMessage("You have uncommitted changes.");
    }
    // 检查分支是否存在
    std::string refs_heads = Utils::join(Repository::getGitliteDir(), "refs", "heads");
    std::string branch_file = Utils::join(refs_heads, branch_name);
    if (!Utils::exists(branch_file))
    {
        Utils::exitWithMessage("A branch with that name does not exist.");
    }
    std::string given_sha = Utils::readContentsAsString(branch_file);
    given_sha.erase(std::remove(given_sha.begin(), given_sha.end(), '\n'), given_sha.end());
    given_sha.erase(std::remove(given_sha.begin(), given_sha.end(), '\r'), given_sha.end());
    std::string current_sha = Commit::getCurrentCommitSha();
    // 检查是否合并自身
    std::string current_branch = getCurrentBranchName();
    if (!current_branch.empty() && current_branch == branch_name)
    {
        Utils::exitWithMessage("Cannot merge a branch with itself.");
    }
    std::string split = findSplitPoint(current_sha, given_sha);
    if (split == given_sha)
    {
        std::cout << "Given branch is an ancestor of the current branch." << std::endl;
        return;
    }

    if (split == current_sha)
    {
        if (!current_branch.empty())
        {
            std::string branch_path = Branch::getBranchRefPath(current_branch);
            Utils::writeContents(branch_path, given_sha + "\n");
        }
        else
        {
            Commit::updateHead(given_sha);
        }
        std::cout << "Current branch fast-forwarded." << std::endl;
        return;
    }

    auto split_map = Commit::loadFileBlobs(split);
    auto curr_map = Commit::loadFileBlobs(current_sha);
    auto given_map = Commit::loadFileBlobs(given_sha);

    // 检查未跟踪文件
    if (wouldMergeOverwriteUntrackedFiles(curr_map, given_map, split_map))
    {
        Utils::exitWithMessage("There is an untracked file in the way; delete it, or add and commit it first.");
    }

    std::set<std::string> all_files;
    for (const auto &p : split_map)
        all_files.insert(p.first);
    for (const auto &p : curr_map)
        all_files.insert(p.first);
    for (const auto &p : given_map)
        all_files.insert(p.first);

    bool conflict = false;

    for (const auto &filename : all_files)
    {
        std::string s = split_map.count(filename) ? split_map[filename] : "";
        std::string c = curr_map.count(filename) ? curr_map[filename] : "";
        std::string g = given_map.count(filename) ? given_map[filename] : "";

        bool c_changed = (c != s);
        bool g_changed = (g != s);

        // 只在给定分支中修改
        if (!c_changed && g_changed)
        {
            if (g.empty())
            {
                if (Utils::exists(filename))
                {
                    Utils::restrictedDelete(filename);
                }
                StagingArea::markFileForDeletion(filename);
            }
            else
            {
                std::string content = readBlobContent(g);
                fs::path filepath(filename);
                if (filepath.has_parent_path())
                {
                    fs::create_directories(filepath.parent_path());
                }
                Utils::writeContents(filename, content);
                StagingArea::stageFile(filename);
            }
            continue;
        }

        // 只在当前分支中修改 保持不变
        if (c_changed && !g_changed)
        {
            continue;
        }

        // 都没有修改 保持不变
        if (!c_changed && !g_changed)
        {
            continue;
        }

        // 在两个分支中都修改了
        if (c_changed && g_changed)
        {
            if (c == g)
            {
                continue;
            }

            // 冲突：生成合并冲突内容
            std::string c_content = c.empty() ? "" : readBlobContent(c);
            std::string g_content = g.empty() ? "" : readBlobContent(g);

            std::ostringstream merged;
            merged << "<<<<<<< HEAD\n";
            merged << c_content;
            if (!c_content.empty() && c_content.back() != '\n')
                merged << "\n";
            merged << "=======\n";
            merged << g_content;
            if (!g_content.empty() && g_content.back() != '\n')
                merged << "\n";
            merged << ">>>>>>>\n";
            fs::path filepath(filename);
            if (filepath.has_parent_path())
            {
                fs::create_directories(filepath.parent_path());
            }
            Utils::writeContents(filename, merged.str());
            StagingArea::stageFile(filename);

            conflict = true;
        }
    }

    // 如果有冲突，停止
    if (conflict)
    {
        std::cout << "Encountered a merge conflict." << std::endl;
        return;
    }

    // 检查是否有更改需要提交
    if (StagingArea::isStagingAreaEmpty())
    {
        Utils::exitWithMessage("No changes added to the commit.");
    }

    // 创建合并提交
    auto staged_vec = StagingArea::getStagedFiles();
    auto removed_vec = StagingArea::getFilesMarkedForDeletion();

    std::map<std::string, std::string> staged_map;
    for (const auto &f : staged_vec)
    {
        staged_map[f] = StagingArea::getBlobShaForFile(f);
    }

    std::string new_tree = Commit::createNewTreeFromStaging(current_sha, staged_map, removed_vec);
    std::time_t now = std::time(nullptr);
    char time_buf[128];
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    std::ostringstream commit_ss;
    commit_ss << "tree " << new_tree << "\n";
    commit_ss << "parent " << current_sha << "\n";
    commit_ss << "parent " << given_sha << "\n";
    commit_ss << "author Gitlite <gitlite@example.com> " << time_buf << " +0000\n";
    commit_ss << "committer Gitlite <gitlite@example.com> " << time_buf << " +0000\n";
    commit_ss << "\n";
    commit_ss << "Merged " << branch_name << " into " << current_branch << "\n";

    std::string commit_str = commit_ss.str();
    std::string header = "commit " + std::to_string(commit_str.size());
    std::string full_content = header + '\0' + commit_str;

    // 计算SHA并保存
    std::string commit_sha = Utils::sha1(full_content);
    std::string obj_dir = Repository::getObjectsDir() + "/" + commit_sha.substr(0, 2);
    Utils::createDirectories(obj_dir);
    std::string obj_path = obj_dir + "/" + commit_sha.substr(2);
    Utils::writeContents(obj_path, full_content);

    // 更新HEAD并清空暂存区
    Commit::updateHead(commit_sha);
    StagingArea::clear();
}
