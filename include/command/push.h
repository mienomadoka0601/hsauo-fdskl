#ifndef PUSHCOMMAND_H
#define PUSHCOMMAND_H

#include <string>
#include <vector>   

class pushcommand {
public:
    // 执行 push 命令
    static void push(const std::string& remote_name, const std::string& remote_branch);
    
private:
    // 检查远程分支是否在本地历史中
    static bool isRemoteBranchInLocalHistory(const std::string& local_commit,
                                            const std::string& remote_commit);
    
    // 复制对象到远程仓库
    static void copyObjectToRemote(const std::string& object_sha,
                                  const std::string& remote_path);
    
    // 复制提交及其相关对象到远程
    static void copyCommitToRemote(const std::string& commit_sha,
                                  const std::string& remote_path);
    
    // 获取需要复制的提交列表
    static std::vector<std::string> getCommitsToCopy(const std::string& local_commit,
                                                    const std::string& remote_commit);
};

#endif // PUSHCOMMAND_H