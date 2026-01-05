#include "../include/command/branch.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/GitliteException.h"
#include "../include/Branch.h"

#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>


void branchcommand::branch(const std::string& branch_name){
    Branch::create(branch_name);

}