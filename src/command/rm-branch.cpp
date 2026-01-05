// RmBranchCommand.cpp
#include "../include/command/rm-branch.h"
#include "../include/Branch.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <iostream>
#include <filesystem>

void rmbranchcommand::rmBranch(const std::string& branch_name) {
    // 检查分支是否存在
    if (!Branch::exists(branch_name)) {
        std::cout << "A branch with that name does not exist." << std::endl;
        exit(1);
    }
    
    // 检查是否是当前分支
    std::string current_branch = Branch::getCurrentBranch();
    std::string trimmed_current = current_branch;
    std::string trimmed_target = branch_name;
    trimmed_current.erase(std::remove(trimmed_current.begin(), trimmed_current.end(), '\n'), trimmed_current.end());
    trimmed_current.erase(std::remove(trimmed_current.begin(), trimmed_current.end(), '\r'), trimmed_current.end());
    trimmed_target.erase(std::remove(trimmed_target.begin(), trimmed_target.end(), '\n'), trimmed_target.end());
    trimmed_target.erase(std::remove(trimmed_target.begin(), trimmed_target.end(), '\r'), trimmed_target.end());
    if (trimmed_current == trimmed_target) {
        std::cout << "Cannot remove the current branch." << std::endl;
        exit(1);
    }
    
    // 获取分支文件路径并删除
    std::string refs_dir = Utils::join(Repository::getGitliteDir(), "refs");
    std::string heads_dir = Utils::join(refs_dir, "heads");
    std::string branch_path = Utils::join(heads_dir, branch_name);
    
    // 删除分支文件
    if (!Utils::restrictedDelete(branch_path)) {
        exit(1);
    }
}

bool rmbranchcommand::canRemoveBranch(const std::string& branch_name, std::string& error_message) {
    // 检查分支是否存在
    if (!Branch::exists(branch_name)) {
        error_message = "A branch with that name does not exist.";
        return false;
    }
    
    // 检查是否是当前分支
    std::string current_branch = Branch::getCurrentBranch();
    if (current_branch == branch_name) {
        error_message = "Cannot remove the current branch.";
        return false;
    }
    
    return true;
}