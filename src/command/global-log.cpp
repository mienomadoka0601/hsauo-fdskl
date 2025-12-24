#include "../include/command/global-log.h"
#include "Commit.h"
#include "Utils.h"
#include "Repository.h"
#include "GitliteException.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <string>

void global_logcommand::global_log() {
    std::string commits_dir = Repository::getCommitsDir();）
    std::vector<std::string> commit_shas = Utils::plainFilenamesIn(commits_dir);
    if (commit_shas.empty()) {
        std::cout << "No commits yet." << std::endl;
        return;
    }
    for (const std::string& sha : commit_shas) {
        printCommitInfo(sha);
    }
}

void global_logcommand::printCommitInfo(const std::string& commit_sha) {
    std::string message = Commit::getCommitMessage(commit_sha);
    std::string raw_timestamp = Commit::getCommitTimestamp(commit_sha);
    std::vector<std::string> parents = Commit::getCommitParents(commit_sha);
    std::cout << "===" << std::endl;
    std::cout << "commit " << commit_sha << std::endl;
    if (parents.size() >= 2) {
        std::string parent1_short = parents[0].substr(0, 7);
        std::string parent2_short = parents[1].substr(0, 7);
        std::cout << "Merge: " << parent1_short << " " << parent2_short << std::endl;
    }
    std::string formatted_time = formatTimestamp(raw_timestamp);
    std::cout << "Date: " << formatted_time << std::endl;
    std::cout << message << std::endl << std::endl;
}
std::string global_logcommand::formatTimestamp(const std::string& raw_timestamp) {
    std::tm tm = {};
    std::istringstream iss(raw_timestamp);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) {
        throw GitliteException("Invalid timestamp format: " + raw_timestamp);
    }
    std::time_t time = std::mktime(&tm);
    std::tm local_tm = *std::localtime(&time);
    std::tm gmt_tm = *std::gmtime(&time);
    int offset_sec = std::difftime(std::mktime(&local_tm), std::mktime(&gmt_tm));
    int offset_hh = offset_sec / 3600;
    int offset_mm = (offset_sec % 3600) / 60;
    std::ostringstream offset_oss;
    offset_oss << (offset_hh >= 0 ? "+" : "-")
               << std::setw(2) << std::setfill('0') << std::abs(offset_hh)
               << std::setw(2) << std::setfill('0') << std::abs(offset_mm);
    std::string offset_str = offset_oss.str();
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%a %b %d %H:%M:%S %Y ") << offset_str;
    std::string result = oss.str();
    if (result[8] == ' ') {
        result.insert(8, "0");
    }

    return result;
}