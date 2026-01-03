#ifndef TREE_H
#define TREE_H
#include "object.h"
#include <string>
#include <map>
#include <vector>

class Tree{
private:
    std::map<std::string, std::string> entries_;
    std::string sha1_;
    
    std::string generateSha1() const;
    
public:
    Tree();
    Tree(const std::map<std::string, std::string>& entries);
    
    void addEntry(const std::string& filename, const std::string& blob_sha);
    void removeEntry(const std::string& filename);
    
    std::string getBlobSha(const std::string& filename) const;
    std::map<std::string, std::string> getEntries() const { return entries_; }
    
    std::vector<uint8_t> serialize() const;
    static Tree deserialize(const std::vector<uint8_t>& data);
    void save();
    static Tree load(const std::string& tree_sha);
    std::string getSha1() const { return sha1_; }
    
    static std::string createTreeFromParentAndStaging(
        const std::string& parent_tree_sha,
        const std::map<std::string, std::string>& staged_files,
        const std::vector<std::string>& removed_files);
};

#endif // TREE_H