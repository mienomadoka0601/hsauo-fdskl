#include "../include/command/commit.h"
#include "../include/Commit.h"
#include "../include/StagingArea.h"
#include "../include/GitliteException.h"
#include <iostream>
#include <vector>
#include <map>

void commitcommand::commit(const std::string& message) {
    //检查消息是否为空
    if (message.empty()) {
        Utils::exitWithMessage("Please enter a commit message.");
    }
    
    //检查暂存区是否为空
    if (StagingArea::isStagingAreaEmpty()) {
        Utils::exitWithMessage("No changes added to the commit.");
    }
    std::string parent_sha = Commit::getCurrentCommitSha();
    auto staged_files_vec = StagingArea::getStagedFiles();
    auto removed_files = StagingArea::getFilesMarkedForDeletion();
    std::map<std::string, std::string> staged_files_map;
    for (const auto& filename : staged_files_vec) {
        std::string blob_sha = StagingArea::getBlobShaForFile(filename);
        if (!blob_sha.empty()) {
            staged_files_map[filename] = blob_sha;
        }
    }
    //创建新的 tree 对象
    std::string new_tree_sha;
    if (parent_sha.empty()) {
        std::map<std::string, std::string> entries;
        for (const auto& [filename, blob_sha] : staged_files_map) {
            entries[filename] = blob_sha;
        }
        for (const auto& filename : removed_files) {
            entries.erase(filename);
        }
        new_tree_sha = Commit::createTreeObject(entries);
    } else {
        new_tree_sha = Commit::createNewTreeFromStaging(parent_sha, staged_files_map, removed_files);
    }
    //创建新的提交对象
    Commit new_commit(message, parent_sha);
    auto new_file_blobs = Commit::loadTreeObject(new_tree_sha);
    for (const auto& [filename, blob_sha] : new_file_blobs) {
        new_commit.addFileBlob(filename, blob_sha);
    }
    new_commit.save();
    Commit::updateHead(new_commit.getSha1());
    StagingArea::clear();
}