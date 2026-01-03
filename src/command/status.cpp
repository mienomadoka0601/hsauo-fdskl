#include "../include/command/status.h"
#include "StagingArea.h"
#include "Commit.h"
#include "Repository.h"
#include "Utils.h"
#include <iostream>
#include <vector>
#include <algorithm>

void statuscommand::status() {
    std::cout << "=== Branches ===" << std::endl;
    // 获取当前分支
    std::string headPath = Utils::join(Repository::getGitliteDir(), "HEAD");
    std::string currentBranch = "master";
    if (Utils::exists(headPath)) {
        std::string headContent = Utils::readContentsAsString(headPath);
        if (headContent.find("ref: ") == 0) {
            size_t pos = headContent.find_last_of('/');
            if (pos != std::string::npos) {
                currentBranch = headContent.substr(pos + 1);
                currentBranch.erase(std::remove(currentBranch.begin(), currentBranch.end(), '\n'), currentBranch.end());
                currentBranch.erase(std::remove(currentBranch.begin(), currentBranch.end(), '\r'), currentBranch.end());
            }
        }
    }
    // 输出当前分支
    std::cout << "*" << currentBranch << std::endl;
    // 获取其他分支
    std::string headsDir = Utils::join(Repository::getGitliteDir(), "refs", "heads");
    std::vector<std::string> otherBranches;
    
    if (Utils::exists(headsDir) && Utils::isDirectory(headsDir)) {
        otherBranches = Utils::plainFilenamesIn(headsDir);
        otherBranches.erase(std::remove(otherBranches.begin(), otherBranches.end(), currentBranch), otherBranches.end());
        std::sort(otherBranches.begin(), otherBranches.end());
        
        for (const auto& branch : otherBranches) {
            std::cout << branch << std::endl;
        }
    }
    
    std::cout << std::endl;
    
    std::cout << "=== Staged Files ===" << std::endl;
    std::vector<std::string> stagedFiles = StagingArea::getStagedFiles();
    std::sort(stagedFiles.begin(), stagedFiles.end());
    
    for (const auto& file : stagedFiles) {
        std::cout << file << std::endl;
    }
    
    std::cout << std::endl;
    
    std::cout << "=== Removed Files ===" << std::endl;
    std::vector<std::string> removedFiles = StagingArea::getFilesMarkedForDeletion();
    std::sort(removedFiles.begin(), removedFiles.end());
    
    for (const auto& file : removedFiles) {
        std::cout << file << std::endl;
    }
    
    std::cout << std::endl;
    
    std::cout << "=== Modifications Not Staged For Commit ===" << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Untracked Files ===" << std::endl;
}