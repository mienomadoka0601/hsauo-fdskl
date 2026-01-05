#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include <string>
#include <vector>
#include <map>

class remotecommand {
public:
    static void addRemote(const std::string& remote_name, const std::string& remote_path);
    
    static void removeRemote(const std::string& remote_name);
    
    static std::string getRemotePath(const std::string& remote_name);
    
    static bool exists(const std::string& remote_name);
    
    static std::map<std::string, std::string> getAllRemotes();
    
    static void initialize();

    static void push(const std::string& remote_name, const std::string& remote_branch);
    
private:
    static std::string getRemoteConfigPath();
};

#endif // REMOTECOMMAND_H