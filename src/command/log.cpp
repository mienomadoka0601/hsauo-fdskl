#include "../include/command/log.h"
#include <iomanip>
#include <sstream>

void logcommand::log() {
    std::string current_sha = Commit::getCurrentCommitSha();
    if (current_sha.empty()) {
        std::cout << "No commits yet." << std::endl;
        return;
    }
    while (!current_sha.empty()) {
        printCommitInfo(current_sha);
        std::vector<std::string> parents = Commit::getCommitParents(current_sha);
        current_sha = parents.empty() ? "" : parents[0];
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
    std::cout << message << std::endl << std::endl;
}
std::string logcommand::formatTimestamp(const std::string& raw_timestamp) {
    std::tm tm = {};
    std::istringstream iss(raw_timestamp);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) {
        throw std::runtime_error("Invalid timestamp format: " + raw_timestamp);
    }
    std::time_t time = std::mktime(&tm);
    std::tm local_tm = *std::localtime(&time);
    std::tm gmt_tm = *std::gmtime(&time);
    int offset_sec = std::difftime(std::mktime(&local_tm), std::mktime(&gmt_tm));
    int offset_hh = offset_sec / 3600;
    int offset_mm = (offset_sec % 3600) / 60;
    std::ostringstream oss;
        oss << (offset_hh >= 0 ? "+" : "-")
        << std::abs(offset_hh)
        << std::setw(2) << std::setfill('0') << std::abs(offset_mm);
    std::string offset_str = oss.str();
    if (std::abs(offset_hh) < 10) {
        offset_str.insert(offset_str.find_first_of("+-") + 1, "0");
    }
    if (std::abs(offset_mm) < 10) {
        offset_str += "0";
    }
    std::ostringstream oss1;
    oss1 << std::put_time(&local_tm, "%a %b %d %H:%M:%S %Y ") << offset_str;
    std::string result = oss1.str();
    if (result[8] == ' ') {
        result.insert(8, "0");
    }

    return result;
}