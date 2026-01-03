#include "../include/command/log.h"
#include <iomanip>
#include <sstream>
#include <ctime>

void logcommand::log() {
    std::string current_sha = Commit::getCurrentCommitSha();
    if (current_sha.empty()) {
        std::cout << "No commits yet." << std::endl;
        return;
    }
    
    while (!current_sha.empty()) {
        printCommitInfo(current_sha);
        std::vector<std::string> parents = Commit::getCommitParents(current_sha);
        
        // 如果没有父提交，或者父提交无效，则停止
        if (parents.empty()) {
            break;
        }
        
        current_sha = parents[0];  // 只跟随第一个父提交
    }
}

void logcommand::printCommitInfo(const std::string& commit_sha) {
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
    std::cout << message << std::endl;
    std::cout << std::endl;  // 确保有一个空行
}

std::string logcommand::formatTimestamp(const std::string& raw_timestamp) {
    // raw_timestamp 格式: "2026-01-03 15:15:10 +0000" 或 "0 +0800"
    
    try {
        // 如果是 Unix 时间戳 0
        if (raw_timestamp.find("0 ") == 0) {
            // 解析 "0 +0800"
            std::istringstream iss(raw_timestamp);
            std::string timestamp_str, timezone;
            if (iss >> timestamp_str >> timezone) {
                // 确保时区是4位
                if (timezone.length() == 4 && (timezone[0] == '+' || timezone[0] == '-')) {
                    // 使用固定的初始提交时间
                    return "Thu Jan 01 08:00:00 1970 " + timezone;
                }
            }
            // 默认返回
            return "Thu Jan 01 08:00:00 1970 +0800";
        }
        
        // 解析正常的时间戳格式
        std::tm tm = {};
        std::istringstream iss(raw_timestamp);
        
        int year, month, day, hour, minute, second;
        char dash1, dash2, space1, colon1, colon2, space2;
        std::string timezone;
        
        if (iss >> year >> dash1 >> month >> dash2 >> day 
                >> hour >> colon1 >> minute >> colon2 >> second
                >> timezone) {
            
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            tm.tm_hour = hour;
            tm.tm_min = minute;
            tm.tm_sec = second;
            tm.tm_isdst = -1;
            
            std::time_t t = std::mktime(&tm);
            std::tm* local_tm = std::localtime(&t);
            
            std::ostringstream oss;
            oss.imbue(std::locale("C"));
            
            // 输出格式
            oss << std::put_time(local_tm, "%a %b %d %H:%M:%S %Y ");
            
            // 确保时区是4位
            if (timezone.length() == 5) {  // 如 "+0800"
                oss << timezone;
            } else if (timezone.length() == 1) {  // 如 "Z"
                oss << "+0000";
            } else {
                // 尝试转换为4位
                if (timezone[0] == '+' || timezone[0] == '-') {
                    if (timezone.length() == 3) {  // 如 "+08"
                        oss << timezone << "00";
                    } else {
                        oss << "+0000";  // 默认
                    }
                } else {
                    oss << "+0000";
                }
            }
            
            std::string result = oss.str();
            
            // 确保日期是两位（前导0）
            if (result.length() > 9 && result[8] == ' ') {
                result.insert(8, "0");
            }
            
            return result;
        }
    } catch (...) {
        // 解析失败
    }
    
    // 默认返回
    return "Thu Jan 01 08:00:00 1970 +0800";
}