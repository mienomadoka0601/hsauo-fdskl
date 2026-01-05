#ifndef BRANCH_H
#define BRANCH_H

#include <string>
#include <vector>
#include <map>

class Branch {
public:
    static std::string getCurrentBranch();
    static void create(const std::string& branch_name);
    static void remove(const std::string& branch_name);
    static std::vector<std::string> getAllBranches();
    static bool exists(const std::string& branch_name);
    static std::string getBranchCommit(const std::string& branch_name);
    static void checkout(const std::string& branch_name);
    static void rename(const std::string& old_name, const std::string& new_name);
    static std::string getCurrentCommitSha();
    static void initialize();
    static std::string getBranchRefPath(const std::string& branch_name);

private:
    
    static std::string getHeadPath();
    
    static std::string getHeadsDir();
    
    static void createBranchRef(const std::string& branch_name, const std::string& commit_sha);
};

#endif // BRANCH_H