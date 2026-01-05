#include "../include/command/fetch.h"
#include "../include/command/remote.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <iostream>
#include <queue>
#include <set>
#include <filesystem>
namespace fs = std::filesystem;

void fetchcommand::fetch(const std::string& remote_name, const std::string& remote_branch) {
    // 1. 获取远程仓库路径
    std::cout << "=== DEBUG FETCH START ===" << std::endl;
    std::cout << "Fetching from remote: " << remote_name << ", branch: " << remote_branch << std::endl;
    
    // 获取远程路径
    std::string remote_path = remotecommand::getRemotePath(remote_name);
    std::cout << "Remote path from config: '" << remote_path << "'" << std::endl;
    
    // 获取当前工作目录
    std::string cwd = fs::current_path().string();
    std::cout << "Current directory: " << cwd << std::endl;
    
    // 转换为绝对路径
    fs::path remote_path_obj(remote_path);
    if (remote_path_obj.is_relative()) {
        fs::path abs_path = fs::absolute(remote_path_obj);
        std::cout << "Absolute path: " << abs_path.string() << std::endl;
        std::cout << "Exists? " << fs::exists(abs_path) << std::endl;
        
        // 检查路径本身
        if (!fs::exists(abs_path)) {
            // 如果路径以 .gitlite 结尾，检查父目录
            if (remote_path.find(".gitlite") != std::string::npos) {
                fs::path parent = abs_path.parent_path();
                std::cout << "Parent directory: " << parent.string() << std::endl;
                std::cout << "Parent exists? " << fs::exists(parent) << std::endl;
            }
        }
    }
    if (remote_path.empty()) {
        std::cout << "Remote directory not found." << std::endl;
        exit(1);
    }
    
    // 2. 检查远程 .gitlite 目录是否存在
    if (!Utils::exists(remote_path)) {
        std::cout << "Remote directory not found." << std::endl;
        exit(1);
    }
    
    // 3. 检查远程分支是否存在
    std::string remote_heads_dir = Utils::join(remote_path, "refs", "heads");
    std::string remote_branch_path = Utils::join(remote_heads_dir, remote_branch);
    
    if (!Utils::exists(remote_branch_path)) {
        std::cout << "That remote does not have that branch." << std::endl;
        exit(1);
    }
    
    // 4. 获取远程分支的最新提交
    std::string remote_commit = Utils::readContentsAsString(remote_branch_path);
    remote_commit.erase(std::remove(remote_commit.begin(), remote_commit.end(), '\n'), remote_commit.end());
    remote_commit.erase(std::remove(remote_commit.begin(), remote_commit.end(), '\r'), remote_commit.end());
    
    if (remote_commit.empty()) {
        exit(1);
    }
    
    // 5. 复制远程提交到本地
    copyCommitFromRemote(remote_commit, remote_path);
    
    // 6. 在本地创建远程跟踪分支
    std::string local_remote_branch = remote_name + "/" + remote_branch;
    std::string refs_dir = Utils::join(Repository::getGitliteDir(), "refs");
    std::string remotes_dir = Utils::join(refs_dir, "remotes");
    std::string branch_file = Utils::join(remotes_dir, remote_name);
    std::string local_remote_refs_dir = Utils::join(remotes_dir, remote_name + "/" + remote_branch);
    
    Utils::createDirectories(local_remote_refs_dir);
    Utils::writeContents(local_remote_refs_dir, remote_commit + "\n");
    
}

bool fetchcommand::objectExistsLocally(const std::string& object_sha) {
    if (object_sha.empty() || object_sha.length() != 40) {
        return false;
    }
    
    std::string local_objects_dir = Repository::getObjectsDir();
    std::string local_object_path = local_objects_dir + "/" + 
                                   object_sha.substr(0, 2) + "/" + 
                                   object_sha.substr(2);
    
    return Utils::exists(local_object_path);
}

void fetchcommand::copyObjectFromRemote(const std::string& object_sha,
                                       const std::string& remote_path) {
    if (object_sha.empty() || object_sha.length() != 40) {
        return;
    }
    
    // 如果本地已存在，跳过
    if (objectExistsLocally(object_sha)) {
        return;
    }
    
    // 远程对象路径
    std::string remote_objects_dir = Utils::join(remote_path, "objects");
    std::string remote_object_path = remote_objects_dir + "/" + 
                                    object_sha.substr(0, 2) + "/" + 
                                    object_sha.substr(2);
    
    if (!Utils::exists(remote_object_path)) {
        return;
    }
    
    // 本地对象路径
    std::string local_objects_dir = Repository::getObjectsDir();
    std::string local_object_dir = local_objects_dir + "/" + object_sha.substr(0, 2);
    std::string local_object_path = local_object_dir + "/" + object_sha.substr(2);
    
    // 复制文件
    std::string content = Utils::readContentsAsString(remote_object_path);
    Utils::createDirectories(local_object_dir);
    Utils::writeContents(local_object_path, content);
}

void fetchcommand::copyCommitFromRemote(const std::string& commit_sha,
                                       const std::string& remote_path) {
    // 复制提交对象
    copyObjectFromRemote(commit_sha, remote_path);
    
    // 读取提交内容以获取 tree
    std::string remote_objects_dir = Utils::join(remote_path, "objects");
    std::string remote_commit_path = remote_objects_dir + "/" + 
                                    commit_sha.substr(0, 2) + "/" + 
                                    commit_sha.substr(2);
    
    if (!Utils::exists(remote_commit_path)) {
        return;
    }
    
    std::string commit_content = Utils::readContentsAsString(remote_commit_path);
    size_t null_pos = commit_content.find('\0');
    if (null_pos == std::string::npos) {
        return;
    }
    
    std::string commit_data = commit_content.substr(null_pos + 1);
    
    // 解析 tree SHA
    std::istringstream iss(commit_data);
    std::string line;
    std::string tree_sha;
    
    while (std::getline(iss, line)) {
        if (line.find("tree ") == 0) {
            tree_sha = line.substr(5);
            break;
        }
    }
    
    // 复制 tree 和 blob
    if (!tree_sha.empty()) {
        copyObjectFromRemote(tree_sha, remote_path);
        
        // 需要解析 tree 来获取 blob SHA
        // 这里简化处理，实际需要解析 tree 对象
    }
    
    // 递归复制父提交
    std::vector<std::string> parents;
    iss.clear();
    iss.seekg(0);
    
    while (std::getline(iss, line)) {
        if (line.find("parent ") == 0) {
            parents.push_back(line.substr(7));
        }
    }
    
    for (const auto& parent : parents) {
        copyCommitFromRemote(parent, remote_path);
    }
}