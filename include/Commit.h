#ifndef COMMIT_H
#define COMMIT_H

#include "object.h"
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
class Commit : public Object {
    private:
    std::string message_;
    std::vector<std::string> parents_;
    std::string timestamp_;
    mutable std::map<std::string, std::string> file_blobs_;
    mutable std::string sha1_;  
    std::string generateSha1();
    std::string getCurrentTimeString(); 
    public:
    std::string getOid() const override;
    void save()const override;
    ObjectType getType() const override { return ObjectType::COMMIT; }
    std::vector<uint8_t> serialize() const override;
    static bool isFileTrackedInCurrentCommit(const std::string& filename);
    static std::string getLatestVersionContent(const std::string& filename);
    Commit(const std::string& message, const std::string& parent_sha);
    void addFileBlob(const std::string& filename, const std::string& blob_sha);
    void removeFileBlob(const std::string& filename);
    static std::string getCurrentCommitSha();
    static std::map<std::string, std::string> loadFileBlobs(const std::string& commit_sha);
    static void updateHead(const std::string& new_commit_sha);
    std::string getSha1() const { return sha1_; }
    void generateSha1IfNeeded();
    static std::vector<std::string> getCommitParents(const std::string& commit_sha);
    static std::string getCommitMessage(const std::string& commit_sha);
    static std::string getCommitTimestamp(const std::string& commit_sha);
    static std::string createTreeObject(const std::map<std::string, std::string>& entries);
    static std::map<std::string, std::string> loadTreeObject(const std::string& tree_sha);
    static std::string getTreeShaFromCommit(const std::string& commit_sha);
    static std::string createNewTreeFromStaging(
    const std::string& parent_commit_sha,
    const std::map<std::string, std::string>& staged_files,
    const std::vector<std::string>& removed_files);
};
template <>
std::shared_ptr<Commit> Object::deserialize<Commit>(const std::vector<uint8_t>& data);

#endif // COMMIT_H
