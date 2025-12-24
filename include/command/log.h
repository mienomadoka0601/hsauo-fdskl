#ifndef LOG_H
#define LOG_H

#include "../include/Commit.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <string>
class logcommand : Object {
    public:
    static void log();
    private:
    static void printCommitInfo(const std::string& commit_sha);
    static std::string formatTimestamp(const std::string& raw_timestamp);
};

#endif // LOG_H