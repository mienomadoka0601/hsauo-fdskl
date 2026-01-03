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

void globallogcommand::globalLog() {
    std::string objects_dir = Repository::getObjectsDir();
    
    if (!Utils::exists(objects_dir)) {
        std::cout << "No commits yet." << std::endl;
        return;
    }
    
    std::vector<std::string> all_commits;
    
    // 遍历 objects 目录的所有子目录
    for (const auto& dir_entry : std::filesystem::directory_iterator(objects_dir)) {
        if (dir_entry.is_directory()) {
            std::string subdir = dir_entry.path().string();
            
            // 获取子目录中的文件
            std::vector<std::string> files = Utils::plainFilenamesIn(subdir);
            
            for (const auto& file : files) {
                // 组合完整的 SHA：前2位（目录名）+ 后38位（文件名）
                std::string dir_name = dir_entry.path().filename().string();
                std::string sha = dir_name + file;
                
                // 检查是否是 commit 对象
                std::string object_path = Utils::join(objects_dir, dir_name, file);
                
                try {
                    std::string content = Utils::readContentsAsString(object_path);
                    // commit 对象以 "commit " 开头
                    if (content.find("commit ") == 0) {
                        all_commits.push_back(sha);
                    }
                } catch (...) {
                    // 跳过无法读取的文件
                    continue;
                }
            }
        }
    }
    
    if (all_commits.empty()) {
        std::cout << "No commits yet." << std::endl;
        return;
    }
    
    for (const std::string& sha : all_commits) {
        printCommitInfo(sha);
    }
}

void globallogcommand::printCommitInfo(const std::string& commit_sha) {
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
std::string globallogcommand::formatTimestamp(const std::string& raw_timestamp) {
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