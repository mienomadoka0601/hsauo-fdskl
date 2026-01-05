#include "../include/Branch.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/GitliteException.h"

#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>

std::string Branch::getHeadsDir() {
    std::string gitlite_dir = Repository::getGitliteDir();
    return Utils::join(gitlite_dir, "refs", "heads");
}

std::string Branch::getHeadPath() {
    std::string gitlite_dir = Repository::getGitliteDir();
    return Utils::join(gitlite_dir, "HEAD");
}

std::string Branch::getBranchRefPath(const std::string& branch_name) {
    return Utils::join(getHeadsDir(), branch_name);
}

std::string Branch::getCurrentBranch() {
    std::string head_path = getHeadPath();
    
    if (!Utils::exists(head_path)) {
        // 如果 HEAD 文件不存在，返回空字符串
        return "";
    }
    
    std::string head_content = Utils::readContentsAsString(head_path);
    // 清理换行符
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\n'), head_content.end());
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\r'), head_content.end());
    
    // 检查是否是分支引用格式
    if (head_content.find("ref: refs/heads/") == 0) {
        // 提取分支名
        return head_content.substr(16); // "ref: refs/heads/".length() = 16
    }
    
    // 如果直接是提交 SHA，说明处于 detached HEAD 状态
    return "";
}

bool Branch::exists(const std::string& branch_name) {
    std::string branch_path = getBranchRefPath(branch_name);
    return Utils::exists(branch_path);
}

void Branch::createBranchRef(const std::string& branch_name, const std::string& commit_sha) {
    std::string heads_dir = getHeadsDir();
    if (!Utils::createDirectories(heads_dir)) {
        Utils::exitWithMessage("Failed to create refs/heads directory");
    }
    
    std::string branch_path = getBranchRefPath(branch_name);
    Utils::writeContents(branch_path, commit_sha + "\n");
}

void Branch::create(const std::string& branch_name) {
    // 检查分支名是否有效
    if (branch_name.empty()) {
        Utils::exitWithMessage("Branch name cannot be empty");
    }
    
    // 检查分支是否已存在
    if (exists(branch_name)) {
        Utils::exitWithMessage("A branch with that name already exists.");
    }
    
    // 获取当前提交的 SHA
    std::string current_commit_sha = getCurrentCommitSha();
    if (current_commit_sha.empty()) {
        Utils::exitWithMessage("No current commit to branch from");
    }
    
    // 创建分支引用
    createBranchRef(branch_name, current_commit_sha);
    
}

std::string Branch::getCurrentCommitSha() {
    return Commit::getCurrentCommitSha();
}

std::vector<std::string> Branch::getAllBranches() {
    std::vector<std::string> branches;
    std::string heads_dir = getHeadsDir();
    
    if (!Utils::exists(heads_dir)) {
        return branches;
    }
    
    // 获取 heads 目录下的所有文件
    DIR* dir = opendir(heads_dir.c_str());
    if (dir == nullptr) {
        return branches;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        // 跳过 . 和 ..
        if (name == "." || name == "..") {
            continue;
        }
        
        // 检查是否是普通文件
        std::string full_path = Utils::join(heads_dir, name);
        if (Utils::isFile(full_path)) {
            branches.push_back(name);
        }
    }
    
    closedir(dir);
    
    // 按字母顺序排序
    std::sort(branches.begin(), branches.end());
    
    return branches;
}

std::string Branch::getBranchCommit(const std::string& branch_name) {
    if (!exists(branch_name)) {
        return "";
    }
    
    std::string branch_path = getBranchRefPath(branch_name);
    std::string content = Utils::readContentsAsString(branch_path);
    
    // 清理换行符
    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());
    
    return content;
}

void Branch::remove(const std::string& branch_name) {
    // 不能删除当前分支
    std::string current_branch = getCurrentBranch();
    if (current_branch == branch_name) {
        Utils::exitWithMessage("Cannot remove the current branch.");
    }
    
    // 检查分支是否存在
    if (!exists(branch_name)) {
        Utils::exitWithMessage("A branch with that name does not exist.");
    }
    
    // 删除分支引用文件
    std::string branch_path = getBranchRefPath(branch_name);
    if (!Utils::restrictedDelete(branch_path)) {
        throw GitliteException("Failed to delete branch: " + branch_name);
    }
}

void Branch::checkout(const std::string& branch_name) {
    // 检查分支是否存在
    if (!exists(branch_name)) {
        Utils::exitWithMessage("No such branch exists.");
    }
    
    // 获取分支指向的提交 SHA
    std::string commit_sha = getBranchCommit(branch_name);
    if (commit_sha.empty()) {
        Utils::exitWithMessage("Branch has no valid commit reference.");
    }
    
    // 检查提交是否存在
    std::string commit_path = Repository::getObjectsDir() + "/" + 
                             commit_sha.substr(0, 2) + "/" + 
                             commit_sha.substr(2);
    if (!Utils::exists(commit_path)) {
        Utils::exitWithMessage("Commit referenced by branch does not exist.");
    }
    
    // 更新 HEAD 指向新分支
    std::string head_content = "ref: refs/heads/" + branch_name + "\n";
    Utils::writeContents(getHeadPath(), head_content);
}

void Branch::rename(const std::string& old_name, const std::string& new_name) {
    // 检查旧分支是否存在
    if (!exists(old_name)) {
        Utils::exitWithMessage("A branch with that name does not exist.");
    }
    
    // 检查新分支名是否已存在
    if (exists(new_name)) {
        Utils::exitWithMessage("A branch with that name already exists.");
    }
    
    // 获取旧分支的提交 SHA
    std::string commit_sha = getBranchCommit(old_name);
    
    // 创建新分支
    createBranchRef(new_name, commit_sha);
    
    // 如果重命名的是当前分支，更新 HEAD
    std::string current_branch = getCurrentBranch();
    if (current_branch == old_name) {
        std::string head_content = "ref: refs/heads/" + new_name + "\n";
        Utils::writeContents(getHeadPath(), head_content);
    }
    
    // 删除旧分支
    std::string old_branch_path = getBranchRefPath(old_name);
    Utils::restrictedDelete(old_branch_path);
}

void Branch::initialize() {
    // 初始化 refs/heads 目录
    std::string heads_dir = getHeadsDir();
    if (!Utils::exists(heads_dir)) {
        Utils::createDirectories(heads_dir);
    }
    
    // 如果没有 HEAD 文件，创建并指向 master
    std::string head_path = getHeadPath();
    if (!Utils::exists(head_path)) {
        Utils::writeContents(head_path, "ref: refs/heads/master\n");
    }
}
