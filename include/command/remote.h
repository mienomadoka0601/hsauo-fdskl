#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include <string>
#include <vector>
#include <map>

class remotecommand {
public:
    // 添加远程仓库
    static void addRemote(const std::string& remote_name, const std::string& remote_path);
    
    // 删除远程仓库
    static void removeRemote(const std::string& remote_name);
    
    // 获取远程仓库路径
    static std::string getRemotePath(const std::string& remote_name);
    
    // 检查远程仓库是否存在
    static bool exists(const std::string& remote_name);
    
    // 获取所有远程仓库
    static std::map<std::string, std::string> getAllRemotes();
    
    // 初始化远程配置
    static void initialize();

    static void push(const std::string& remote_name, const std::string& remote_branch);
    
private:
    // 获取远程配置文件路径
    static std::string getRemoteConfigPath();
};

#endif // REMOTECOMMAND_H