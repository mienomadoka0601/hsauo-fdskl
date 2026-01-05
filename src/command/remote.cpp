#include "../include/command/remote.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

std::string remotecommand::getRemoteConfigPath() {
    std::string gitlite_dir = Repository::getGitliteDir();
    return Utils::join(gitlite_dir, "remotes.txt");
}

void remotecommand::addRemote(const std::string& remote_name, const std::string& remote_path) {
    std::cout << "=== DEBUG add-remote ===" << std::endl;
    std::cout << "remote_name: " << remote_name << std::endl;
    std::cout << "remote_path: " << remote_path << std::endl;
    
    // 1. 检查远程是否已存在
    if (exists(remote_name)) {
        std::cout << "DEBUG: Remote already exists" << std::endl;
        std::cout << "A remote with that name already exists." << std::endl;
        exit(1);
    }
    
    std::cout << "DEBUG: Remote does not exist, proceeding..." << std::endl;
    
    // 2. 读取现有的远程配置
    std::map<std::string, std::string> remotes = getAllRemotes();
    std::cout << "DEBUG: Current remotes count: " << remotes.size() << std::endl;
    
    // 3. 添加新的远程
    remotes[remote_name] = remote_path;
    
    // 4. 写回配置文件
    std::string config_path = getRemoteConfigPath();
    std::cout << "DEBUG: Writing to config: " << config_path << std::endl;
    
    std::ofstream config_file(config_path);
    
    if (!config_file.is_open()) {
        std::cout << "DEBUG: Failed to open config file" << std::endl;
        std::cout << "Failed to write remote configuration." << std::endl;
        exit(1);
    }
    
    std::cout << "DEBUG: Writing remotes to file:" << std::endl;
    for (const auto& [name, path] : remotes) {
        std::cout << "  " << name << "=" << path << std::endl;
        config_file << name << "=" << path << std::endl;
    }
    
    config_file.close();
    std::cout << "DEBUG: add-remote completed successfully" << std::endl;
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
    std::cout << "=== DEBUG getRemotePath ===" << std::endl;
    std::cout << "Looking for remote: " << remote_name << std::endl;
    
    std::map<std::string, std::string> remotes = getAllRemotes();
    std::cout << "DEBUG: Found " << remotes.size() << " remotes" << std::endl;
    
    for (const auto& [name, path] : remotes) {
        std::cout << "  " << name << " -> " << path << std::endl;
    }
    
    auto it = remotes.find(remote_name);
    
    if (it != remotes.end()) {
        std::cout << "DEBUG: Found path: " << it->second << std::endl;
        return it->second;
    }
    
    std::cout << "DEBUG: Remote not found" << std::endl;
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