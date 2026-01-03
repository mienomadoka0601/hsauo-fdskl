#ifndef FIND_H
#define FIND_H

#include "../include/object.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/StagingArea.h"
#include <string>
#include <vector>
#include <map>

class findcommand{
    public:
    static void find(const std::string& target_msg);
    private:
    static std::vector<std::string> findMatchingCommits(const std::string& target_msg);
};

#endif //FIND_H