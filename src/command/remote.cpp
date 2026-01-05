#include "../include/command/remote.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
std::string remotecommand::getRemoteConfigPath() {
    std::string gitlite_dir = Repository::getGitliteDir();
    std::string path = Utils::join(gitlite_dir, "remotes.txt");
    return path;
}

void remotecommand::addRemote(const std::string& remote_name, const std::string& remote_path) {
    
    // 1. 检查远程是否已存在
    if (exists(remote_name)) {
        std::cout << "A remote with that name already exists." << std::endl;
        exit(1);
    }
    
    // 2. 读取现有的远程配置
    std::map<std::string, std::string> remotes = getAllRemotes();
    
    // 3. 添加新的远程
    remotes[remote_name] = remote_path;
    
    // 4. 写回配置文件
    std::string config_path = getRemoteConfigPath();
    
    // 确保.gitlite目录存在
    std::string gitlite_dir = Repository::getGitliteDir();
    Utils::createDirectories(gitlite_dir);
    
    std::ofstream config_file(config_path);
    
    // 检查文件是否成功打开
    if (!config_file.is_open()) {
        perror("Error details");  // 打印系统错误
        exit(1);
    }
    
    // 写入内容
    for (const auto& [name, path] : remotes) {
        config_file << name << "=" << path << std::endl;
    }
    
    config_file.close();
    
    // 验证文件内容
    if (Utils::exists(config_path)) {
        std::string content = Utils::readContentsAsString(config_path);
    }
    
}

bool remotecommand::exists(const std::string& remote_name) {
    std::map<std::string, std::string> remotes = getAllRemotes();
    return remotes.find(remote_name) != remotes.end();
}

std::map<std::string, std::string> remotecommand::getAllRemotes() {
    std::map<std::string, std::string> remotes;
    std::string config_path = getRemoteConfigPath();
    
    if (!Utils::exists(config_path)) {
        return remotes;
    }
    
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        return remotes;
    }
    
    std::string line;
    while (std::getline(config_file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        size_t equals_pos = line.find('=');
        if (equals_pos != std::string::npos) {
            std::string name = line.substr(0, equals_pos);
            std::string path = line.substr(equals_pos + 1);
            
            // 去除可能的空白字符
            name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
            name.erase(std::remove(name.begin(), name.end(), '\t'), name.end());
            name.erase(std::remove(name.begin(), name.end(), '\r'), name.end());
            
            path.erase(std::remove(path.begin(), path.end(), ' '), path.end());
            path.erase(std::remove(path.begin(), path.end(), '\t'), path.end());
            path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
            
            if (!name.empty() && !path.empty()) {
                remotes[name] = path;
            }
        }
    }
    
    config_file.close();
    return remotes;
}

std::string remotecommand::getRemotePath(const std::string& remote_name) {
    
    std::map<std::string, std::string> remotes = getAllRemotes();
    
    auto it = remotes.find(remote_name);
    
    if (it != remotes.end()) {
        return it->second;
    }
    return "";
}

void remotecommand::removeRemote(const std::string& remote_name) {
    if (!exists(remote_name)) {
        std::cout << "A remote with that name does not exist." << std::endl;
        exit(1);
    }
    
    std::map<std::string, std::string> remotes = getAllRemotes();
    remotes.erase(remote_name);
    
    // 写回配置文件
    std::string config_path = getRemoteConfigPath();
    std::ofstream config_file(config_path);
    
    if (!config_file.is_open()) {
        std::cout << "Failed to update remote configuration." << std::endl;
        exit(1);
    }
    
    for (const auto& [name, path] : remotes) {
        config_file << name << "=" << path << std::endl;
    }
    
    config_file.close();
}

void remotecommand::initialize() {
    std::string config_path = getRemoteConfigPath();
    
    // 如果配置文件不存在，创建空文件
    if (!Utils::exists(config_path)) {
        std::ofstream config_file(config_path);
        if (config_file.is_open()) {
            config_file.close();
        }
    }
}