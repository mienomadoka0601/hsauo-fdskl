#include "../include/command/checkout.h"
#include "Commit.h"
#include "Utils.h"
#include "Repository.h"
#include "GitliteException.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>

std::string checkoutcommand::resolveCommitSha(const std::string& short_sha) {
    
    if (short_sha.length() == 40) {
        return short_sha;
    }
    
    std::string objects_dir = Repository::getObjectsDir();
    std::vector<std::string> matching_commits;
    
    // 遍历 objects 目录查找匹配的 commit
    for (const auto& dir_entry : std::filesystem::directory_iterator(objects_dir)) {
        if (dir_entry.is_directory()) {
            std::string subdir = dir_entry.path().filename().string();
            // 检查目录名是否以 short_sha 的前2位开头
            if (short_sha.length() >= 2 && subdir != short_sha.substr(0, 2)) {
                continue;
            }
            // 遍历子目录中的文件
            std::vector<std::string> files = Utils::plainFilenamesIn(dir_entry.path().string());
            
            for (const auto& file : files) {
                std::string full_sha = subdir + file;
                // 检查是否是 commit 对象
                std::string object_path = Utils::join(objects_dir, subdir, file);
                
                try {
                    std::string content = Utils::readContentsAsString(object_path);
                    
                    if (content.find("commit ") == 0) {
                        // 检查 SHA 是否匹配缩写
                        if (full_sha.find(short_sha) == 0) {
                            matching_commits.push_back(full_sha);
                        }
                    }
                } catch (...) {
                    // 跳过无法读取的文件
                    continue;
                }
            }
        }
    }
    
    if (matching_commits.empty()) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    
    // 按字母顺序排序，返回第一个
    std::sort(matching_commits.begin(), matching_commits.end());
    return matching_commits[0];
}

// 检查提交是否存在
bool checkoutcommand::commitExists(const std::string& commit_sha) {
    try {
        std::string resolved = resolveCommitSha(commit_sha);
        std::string object_path = Repository::getObjectsDir() + "/" + 
                                 resolved.substr(0, 2) + "/" + 
                                 resolved.substr(2);
        return Utils::exists(object_path);
    } catch (...) {
        return false;
    }
}

// 从提交中获取文件内容
std::string checkoutcommand::getFileFromCommit(const std::string& commit_sha, const std::string& filename) {
    
    std::string resolved_sha = resolveCommitSha(commit_sha);
    
    // 获取提交中的文件列表
    auto file_blobs = Commit::loadFileBlobs(resolved_sha);
    
    for (const auto& [fname, fsha] : file_blobs) {
        std::cout << "  " << fname << " -> " << fsha << std::endl;
    }
    
    auto it = file_blobs.find(filename);
    
    if (it == file_blobs.end()) {
        Utils::exitWithMessage("File does not exist in that commit.");
    }
    
    std::string blob_sha = it->second;
    std::string objects_dir = Repository::getObjectsDir();
    std::string blob_path = objects_dir + "/" + blob_sha.substr(0, 2) + "/" + blob_sha.substr(2);
    
    if (!Utils::exists(blob_path)) {
        std::vector<std::string> subdirs = Utils::plainFilenamesIn(objects_dir);
        for (const auto& subdir : subdirs) {
            std::string subdir_path = Utils::join(objects_dir, subdir);
            if (Utils::isDirectory(subdir_path)) {
                std::vector<std::string> files = Utils::plainFilenamesIn(subdir_path);
            }
        }
        Utils::exitWithMessage("File blob not found.");
    }
    
    // 读取 blob 内容
    std::string content;
    content = Utils::readContentsAsString(blob_path);
    
    // 解析 blob 格式
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos) {
        Utils::exitWithMessage("Invalid blob format.");
    }
    return content.substr(null_pos + 1);
}

// 第一种形式: gitlite checkout [commit_id] -- filename
void checkoutcommand::checkout(const std::string& commit_id, const std::string& filename) {
    // 检查提交是否存在
    if (!commitExists(commit_id)) {
        Utils::exitWithMessage("No commit with that id exists.");
    }
    
    // 获取文件内容
    std::string file_content = getFileFromCommit(commit_id, filename);
    
    // 确保父目录存在
    std::filesystem::path filepath(filename);
    if (filepath.has_parent_path()) {
        std::filesystem::create_directories(filepath.parent_path());
    }
    
    // 写入工作目录
    Utils::writeContents(filename, file_content);
}

// 第二种形式: gitlite checkout -- filename
void checkoutcommand::checkout(const std::string& filename) {
    // 获取当前 HEAD 的 commit SHA
    std::string head_sha = Commit::getCurrentCommitSha();
    
    
    // 使用 HEAD 提交
    checkout(head_sha, filename);
}