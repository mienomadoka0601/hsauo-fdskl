#ifndef MERGECOMMAND_H
#define MERGECOMMAND_H

#include <string>
#include <map>
#include <vector>

class mergecommand {
public:
    static void merge(const std::string& branch_name);
    static std::string readBlobContent(const std::string& blob_sha);
    
private:
    // 辅助函数
    static std::string findSplitPoint(const std::string& commit1, const std::string& commit2);
    static bool wouldMergeOverwriteUntrackedFiles(
        const std::map<std::string, std::string>& current_files,
        const std::map<std::string, std::string>& given_files,
        const std::map<std::string, std::string>& split_files);
    static std::string getCurrentBranchName();
    static std::vector<std::string> getCommitAncestors(const std::string& commit_sha);
};

#endif // MERGECOMMAND_H