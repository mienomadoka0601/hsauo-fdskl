#ifndef PUSHCOMMAND_H
#define PUSHCOMMAND_H

#include <string>
#include <vector>   

class pushcommand {
public:
    static void push(const std::string& remote_name, const std::string& remote_branch);
    
private:
    static bool isRemoteBranchInLocalHistory(const std::string& local_commit,
                                            const std::string& remote_commit);
    
    static void copyObjectToRemote(const std::string& object_sha,
                                  const std::string& remote_path);
    
    static void copyCommitToRemote(const std::string& commit_sha,
                                  const std::string& remote_path);
    
    static std::vector<std::string> getCommitsToCopy(const std::string& local_commit,
                                                    const std::string& remote_commit);
};

#endif // PUSHCOMMAND_H