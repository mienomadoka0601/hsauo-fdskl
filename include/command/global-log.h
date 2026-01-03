#ifndef GLOBAL_LOG_H
#define GLOBAL_LOG_H

#include "../include/Commit.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <string>

class globallogcommand{
    public:
    static void globalLog();
    private:
    static void printCommitInfo(const std::string& commit_sha);
    static std::string formatTimestamp(const std::string& raw_timestamp);
};

#endif //GLOBAL_LOG_H