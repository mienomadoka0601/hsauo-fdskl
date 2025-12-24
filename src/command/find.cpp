#include "../include/command/find.h"
#include "Repository.h"
#include "Commit.h"
#include "Utils.h"
#include "GitliteException.h"
#include <iostream>
#include <vector>
#include <string>

void findcommand::find(const std::string& target_msg) {
    if (target_msg.empty()) {
        std::cerr << "Usage: gitlite find [commit message]" << std::endl;
        return;
    }
    std::vector<std::string> matching_shas = findMatchingCommits(target_msg);
    if (matching_shas.empty()) {
        std::cout << "Found no commit with that message." << std::endl;
    } else {
        for (const std::string& sha : matching_shas) {
            std::cout << sha << std::endl;
        }
    }
}

std::vector<std::string> findcommand::findMatchingCommits(const std::string& target_msg) {
    std::vector<std::string> matching_shas;
    std::string commits_dir = Repository::getCommitsDir();
    std::vector<std::string> all_shas = Utils::plainFilenamesIn(commits_dir);
    for (const std::string& sha : all_shas) {
        std::string commit_msg = Commit::getCommitMessage(sha);
        if (commit_msg.find(target_msg) != std::string::npos) {
            matching_shas.push_back(sha);
        }
    }
    return matching_shas;
}