#ifndef RESETCOMMAND_H
#define RESETCOMMAND_H

#include <string>
#include <map>

class resetcommand {
public:
    static void reset(const std::string& commit_sha);
    
private:
    static bool wouldOverwriteUntrackedFiles(
        const std::map<std::string, std::string>& current_files,
        const std::map<std::string, std::string>& target_files);
};

#endif // RESETCOMMAND_H