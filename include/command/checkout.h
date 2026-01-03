#ifndef CHECKOUT_H
#define CHECKOUT_H

#include <string>

class checkoutcommand {
    public:
    static void checkout(const std::string& commit_id, const std::string& filename);
    static void checkout(const std::string& filename);  // 使用 HEAD 的版本
    private:
    static std::string resolveCommitSha(const std::string& short_sha);
    static bool commitExists(const std::string& commit_sha);
    static std::string getFileFromCommit(const std::string& commit_sha, const std::string& filename);
};

#endif