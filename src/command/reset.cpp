#include "../include/command/reset.h"
#include "../include/command/checkout.h"
#include "../include/Commit.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/StagingArea.h"
#include "../include/Branch.h"
#include <iostream>
namespace fs = std::filesystem;
bool resetcommand::wouldOverwriteUntrackedFiles(
    const std::map<std::string, std::string>& current_files,
    const std::map<std::string, std::string>& target_files) {
    
    for (const auto& entry : fs::recursive_directory_iterator(".")) {
        if (!entry.is_regular_file()) continue;
        
        std::string full_path = entry.path().string();
        if (full_path.find(".gitlite") != std::string::npos) continue;
        
        std::string relative_path = full_path.substr(2); // 去掉 "./"
        
        // 如果文件在当前提交中未跟踪，但在目标提交中存在
        if (current_files.find(relative_path) == current_files.end() &&
            target_files.find(relative_path) != target_files.end()) {
            
            return true;
        }
    }
    return false;
}

void resetcommand::reset(const std::string& commit_sha) {
    // 1. 验证提交存在（根据要求：不需支持缩写功能）
    
    // 检查长度是否为40
    if (commit_sha.length() != 40) {
        std::cout << "No commit with that id exists." << std::endl;
        exit(1);
    }
    
    // 检查对象是否存在
    std::string object_path = Repository::getObjectsDir() + "/" + 
                             commit_sha.substr(0, 2) + "/" + commit_sha.substr(2);
    
    if (!Utils::exists(object_path)) {
        std::cout << "No commit with that id exists." << std::endl;
        exit(1);
    }
    
    // 验证是commit对象
    std::string content = Utils::readContentsAsString(object_path);
    if (content.find("commit ") != 0) {
        std::cout << "No commit with that id exists." << std::endl;
        exit(1);
    }
    
    // 2. 检查当前分支
    std::string current_branch = Branch::getCurrentBranch();
    if (current_branch.empty()) {
        // 在detached HEAD状态下，reset不做任何操作（无输出）
        return;
    }
    
    // 3. 获取文件列表
    std::string current_sha = Commit::getCurrentCommitSha();
    auto current_files = Commit::loadFileBlobs(current_sha);
    auto target_files = Commit::loadFileBlobs(commit_sha);
    
    // 4. 检查未跟踪文件
    if (wouldOverwriteUntrackedFiles(current_files, target_files)) {
        std::cout << "There is an untracked file in the way; delete it, or add and commit it first." << std::endl;
        return;
    }
    
    // 5. 删除在当前提交中存在但在目标提交中不存在的文件
    for (const auto& [filename, _] : current_files) {
        if (target_files.find(filename) == target_files.end()) {
            if (Utils::exists(filename)) {
                Utils::restrictedDelete(filename);
            }
        }
    }
    
    // 6. 恢复目标提交中的文件
    for (const auto& [filename, blob_sha] : target_files) {
        checkoutcommand::checkout(commit_sha, filename);
    }
    
    // 7. 更新分支引用
    std::string branch_path = Branch::getBranchRefPath(current_branch);
    Utils::writeContents(branch_path, commit_sha + "\n");
    
    // 8. 清空暂存区
    StagingArea::clear();
}