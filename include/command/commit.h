#ifndef COMMITCOMMAND_H
#define COMMITCOMMAND_H

#include "../include/object.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/StagingArea.h"
#include <string>
#include <vector>
#include <map>

class commitcommand : public Commit{
    public:
        commitcommand(const std::string& message, const std::string& parent_sha)
        : Commit(message, parent_sha) {} 
        static void commit(const std::string& message);
};

#endif // COMMITCOMMAND_H