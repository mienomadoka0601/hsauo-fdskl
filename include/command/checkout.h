#ifndef CHECKOUT_H
#define CHECKOUT_H

#include <string>
#include <map>

class checkoutcommand {
    public:
    static void checkout(const std::string& commit_id, const std::string& filename);
    static void checkout(const std::string& filename);
    static void checkoutBranch(const std::string& branch_name);
    static std::string resolveCommitSha(const std::string& short_sha);
    static std::string trim(const std::string& str);
    private:
    static bool commitExists(const std::string& commit_sha);
    static std::string getFileFromCommit(const std::string& commit_sha, const std::string& filename);
    static void checkUntrackedFilesRecursive(
        const std::map<std::string, std::string>& current_files,
        const std::map<std::string, std::string>& target_files);
    
    static void performCheckout(
        const std::map<std::string, std::string>& current_files,
        const std::map<std::string, std::string>& target_files);
};

#endif