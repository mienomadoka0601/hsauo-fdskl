#include "../include/Tree.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include <sstream>

Tree::Tree() {}

Tree::Tree(const std::map<std::string, std::string>& entries) 
    : entries_(entries) {
    sha1_ = generateSha1();
}

std::string Tree::generateSha1() const {
    std::string serialized_data;
    auto data = serialize();
    serialized_data.assign(data.begin(), data.end());
    return Utils::sha1("tree " + std::to_string(serialized_data.size()) + 
                       '\0' + serialized_data);
}

void Tree::addEntry(const std::string& filename, const std::string& blob_sha) {
    entries_[filename] = blob_sha;
    sha1_ = generateSha1();
}

void Tree::removeEntry(const std::string& filename) {
    entries_.erase(filename);
    sha1_ = generateSha1();
}

std::string Tree::getBlobSha(const std::string& filename) const {
    auto it = entries_.find(filename);
    if (it != entries_.end()) {
        return it->second;
    }
    return "";
}

std::vector<uint8_t> Tree::serialize() const {
    std::vector<uint8_t> data;
    
    for (const auto& [filename, blob_sha] : entries_) {
        std::string entry = filename + ":" + blob_sha + "\n";
        data.insert(data.end(), entry.begin(), entry.end());
    }
    
    return data;
}

Tree Tree::deserialize(const std::vector<uint8_t>& data) {
    std::map<std::string, std::string> entries;
    std::string content(data.begin(), data.end());
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string filename = line.substr(0, colon_pos);
            std::string blob_sha = line.substr(colon_pos + 1);
            entries[filename] = blob_sha;
        }
    }
    
    return Tree(entries);
}

void Tree::save(){
    std::string content(serialize().begin(), serialize().end());
    std::string header = "tree " + std::to_string(content.size());
    std::string full_data = header + '\0' + content;
    
    std::string sha = Utils::sha1(full_data);
    std::string object_dir = Repository::getObjectsDir() + "/" + sha.substr(0, 2);
    std::string object_path = object_dir + "/" + sha.substr(2);
    
    Utils::createDirectories(object_dir);
    Utils::writeContents(object_path, full_data);
}

Tree Tree::load(const std::string& tree_sha) {
    std::string object_path = Repository::getObjectsDir() + "/" + 
                             tree_sha.substr(0, 2) + "/" + 
                             tree_sha.substr(2);
    
    std::string content = Utils::readContentsAsString(object_path);
    
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos) {
        throw std::runtime_error("Invalid tree object format");
    }
    
    std::string tree_content = content.substr(null_pos + 1);
    std::vector<uint8_t> data(tree_content.begin(), tree_content.end());
    
    return deserialize(data);
}

std::string Tree::createTreeFromParentAndStaging(
    const std::string& parent_tree_sha,
    const std::map<std::string, std::string>& staged_files,
    const std::vector<std::string>& removed_files) {
    
    Tree parent_tree = Tree::load(parent_tree_sha);
    auto entries = parent_tree.getEntries();
    
    for (const auto& [filename, blob_sha] : staged_files) {
        entries[filename] = blob_sha;
    }
    
    for (const auto& filename : removed_files) {
        entries.erase(filename);
    }
    
    Tree new_tree(entries);
    new_tree.save();
    
    return new_tree.getSha1();
}