#ifndef FETCHCOMMAND_H
#define FETCHCOMMAND_H

#include <string>

class fetchcommand {
public:
    static void fetch(const std::string& remote_name, const std::string& remote_branch);
    
private:
    static void copyObjectFromRemote(const std::string& object_sha,
                                    const std::string& remote_path);
    
    static void copyCommitFromRemote(const std::string& commit_sha,
                                    const std::string& remote_path);
    
    static bool objectExistsLocally(const std::string& object_sha);
    static void copyTreeAndBlobs(const std::string& tree_sha,const std::string& remote_path);
};

#endif // FETCHCOMMAND_H