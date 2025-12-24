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

class findcommand : Object{
    public:
    void find();
    private:
    std::vector<std::string> findMatchingCommits(const std::string& target_msg);
};

#endif //FIND_H