#ifndef FETCHCOMMAND_H
#define FETCHCOMMAND_H

#include <string>

class fetchcommand {
public:
    // 执行 fetch 命令
    static void fetch(const std::string& remote_name, const std::string& remote_branch);
    
private:
    // 从远程复制对象到本地
    static void copyObjectFromRemote(const std::string& object_sha,
                                    const std::string& remote_path);
    
    // 从远程复制提交到本地
    static void copyCommitFromRemote(const std::string& commit_sha,
                                    const std::string& remote_path);
    
    // 检查对象是否在本地存在
    static bool objectExistsLocally(const std::string& object_sha);
};

#endif // FETCHCOMMAND_H