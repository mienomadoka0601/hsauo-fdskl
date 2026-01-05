#ifndef BRANCHCOMMAND_H
#define BRANCHCOMMAND_H

#include <string>
#include <vector>
#include <map>
#include "Branch.h"

class branchcommand {
public:
    static void branch(const std::string& branch_name);
};

#endif // BRANCHCOMMAND_H