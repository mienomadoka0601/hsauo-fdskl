#include "../include/command/log.h"
#include <iomanip>
#include <sstream>
#include <ctime>

void logcommand::log()
{
    std::string current_sha = Commit::getCurrentCommitSha();
    if (current_sha.empty())
    {
        std::cout << "No commits yet." << std::endl;
        return;
    }

    while (!current_sha.empty())
    {
        printCommitInfo(current_sha);
        std::vector<std::string> parents = Commit::getCommitParents(current_sha);

        // 如果没有父提交，或者父提交无效，则停止
        if (parents.empty())
        {
            break;
        }

        current_sha = parents[0]; // 只跟随第一个父提交
    }
}

void logcommand::printCommitInfo(const std::string &commit_sha)
{
    std::string message = Commit::getCommitMessage(commit_sha);
    std::string raw_timestamp = Commit::getCommitTimestamp(commit_sha);
    std::vector<std::string> parents = Commit::getCommitParents(commit_sha);

    std::cout << "===" << std::endl;
    std::cout << "commit " << commit_sha << std::endl;

    if (parents.size() >= 2)
    {
        std::string parent1_short = parents[0].substr(0, 7);
        std::string parent2_short = parents[1].substr(0, 7);
        std::cout << "Merge: " << parent1_short << " " << parent2_short << std::endl;
    }

    std::string formatted_time = formatTimestamp(raw_timestamp);
    std::cout << "Date: " << formatted_time << std::endl;
    std::cout << message << std::endl;
    std::cout << std::endl;
}

std::string logcommand::formatTimestamp(const std::string &raw_timestamp)
{
    // 解析日期时间
    std::tm tm = {};
    std::istringstream iss(raw_timestamp);

    int year, month, day, hour, minute, second;
    char dash1, dash2, space1, colon1, colon2, space2;
    std::string timezone;

    if (iss >> year >> dash1 >> month >> dash2 >> day >> hour >> colon1 >> minute >> colon2 >> second >> timezone)
    {

        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1;

        std::time_t t = std::mktime(&tm);
        std::tm *local_tm = std::localtime(&t);
        std::ostringstream oss;
        oss.imbue(std::locale("C"));
        oss << std::put_time(local_tm, "%a %b %d %H:%M:%S %Y ");
        oss << timezone;
        std::string result = oss.str();
        size_t month_end = result.find(' ', 4);
        if (month_end != std::string::npos)
        {
            size_t day_start = month_end + 1;
            size_t day_end = result.find(' ', day_start);
            if (day_end != std::string::npos)
            {
                std::string day_str = result.substr(day_start, day_end - day_start);
                if (day_str.length() == 1)
                {
                    result.replace(day_start, 1, "0" + day_str);
                }
            }
        }

        return result;
    }
    return "Thu Jan 01 08:00:00 1970 +0800";
}