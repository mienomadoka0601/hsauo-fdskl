// RmBranchCommand.h
#ifndef RMBRANCHCOMMAND_H
#define RMBRANCHCOMMAND_H

#include <string>

class rmbranchcommand {
public:
    static void rmBranch(const std::string& branch_name);
    
    static bool canRemoveBranch(const std::string& branch_name, std::string& error_message);
};

#endif // RMBRANCHCOMMAND_H