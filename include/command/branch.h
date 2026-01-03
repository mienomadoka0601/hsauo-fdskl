#ifndef BRANCH_H
#define BRANCH_H

#include <string>
#include <vector>
#include <map>

class Branch {
public:
    // 获取当前分支名
    static std::string getCurrentBranch();
    
    // 创建新分支
    static void create(const std::string& branch_name);
    
    // 删除分支
    static void remove(const std::string& branch_name);
    
    // 获取所有分支列表
    static std::vector<std::string> getAllBranches();
    
    // 检查分支是否存在
    static bool exists(const std::string& branch_name);
    
    // 获取分支指向的提交 SHA
    static std::string getBranchCommit(const std::string& branch_name);
    
    // 切换到指定分支
    static void checkout(const std::string& branch_name);
    
    // 重命名分支
    static void rename(const std::string& old_name, const std::string& new_name);
    
    // 获取当前提交 SHA
    static std::string getCurrentCommitSha();
    
    // 初始化默认分支
    static void initialize();

private:
    // 获取分支引用路径
    static std::string getBranchRefPath(const std::string& branch_name);
    
    // 获取 HEAD 文件路径
    static std::string getHeadPath();
    
    // 获取 refs/heads 目录路径
    static std::string getHeadsDir();
    
    // 创建分支引用文件
    static void createBranchRef(const std::string& branch_name, const std::string& commit_sha);
};

#endif // BRANCH_H