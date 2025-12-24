#include "../include/Commit.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/StagingArea.h"

#include <string>

std::string Commit::getLatestVersionContent(const std::string& filename) {
    std::string current_sha = getCurrentCommitSha();
    if (current_sha.empty()) {
        return "";
    }
    
    auto file_blobs = loadFileBlobs(current_sha);
    auto it = file_blobs.find(filename);
    if (it == file_blobs.end()) {
        return "";
    }
    
    std::string blob_sha = it->second;
    std::string blob_path = Repository::getObjectsDir() + "/" + 
                           blob_sha.substr(0, 2) + "/" + 
                           blob_sha.substr(2);
    
    if (Utils::exists(blob_path)) {
        std::string content = Utils::readContentsAsString(blob_path);
        size_t null_pos = content.find('\0');
        if (null_pos != std::string::npos) {
            return content.substr(null_pos + 1);
        }
    }
    
    return "";
}
Commit::Commit(const std::string& message, const std::string& parent_sha) 
    : message_(message), timestamp_(getCurrentTimeString()) {
    
    if (!parent_sha.empty()) {
        parents_.push_back(parent_sha);
        file_blobs_ = loadFileBlobs(parent_sha);
    }
    
    generateSha1IfNeeded();
}
void Commit::addFileBlob(const std::string& filename, const std::string& blob_sha) {
    file_blobs_[filename] = blob_sha;
}
void Commit::removeFileBlob(const std::string& filename) {
    file_blobs_.erase(filename);
}
std::string Commit::generateSha1() {
    std::ostringstream content;
    for (size_t i = 0; i < parents_.size(); ++i) {
        if (i > 0) content << ",";
        content << parents_[i];
    }
    content << "\n" << timestamp_ << "\n" << message_ << "\n";
    for (const auto& [filename, blob_sha] : file_blobs_) {
        content << filename << " " << blob_sha << "\n";
    }
    return Utils::sha1(content.str());
}
void Commit::generateSha1IfNeeded() {
    if (sha1_.empty()) {
        sha1_ = generateSha1();
    }
}
void Commit::save() {
    //先创建 tree 对象
    std::string tree_sha = createTreeObject(file_blobs_);
    
    //构建提交内容
    std::stringstream commit_content;
    commit_content << "tree " << tree_sha << "\n";
    
    for (const auto& parent : parents_) {
        commit_content << "parent " << parent << "\n";
    }
    
    commit_content << "author " << "Gitlite <gitlite@example.com> " 
                   << timestamp_ << " +0000\n";
    commit_content << "committer " << "Gitlite <gitlite@example.com> " 
                   << timestamp_ << " +0000\n";
    commit_content << "\n";
    commit_content << message_ << "\n";
    
    std::string commit_str = commit_content.str();
    
    //计算提交 SHA
    std::string header = "commit " + std::to_string(commit_str.size());
    std::string full_data = header + '\0' + commit_str;
    sha1_ = Utils::sha1(full_data);
    
    //保存提交对象
    std::string object_dir = Repository::getObjectsDir() + "/" + sha1_.substr(0, 2);
    std::string object_path = object_dir + "/" + sha1_.substr(2);
    
    Utils::createDirectories(object_dir);
    Utils::writeContents(object_path, full_data);
}

std::string Commit::getCurrentCommitSha() {
    std::string head_file = Utils::join(Repository::getGitliteDir(), "HEAD");
    if (!Utils::isFile(head_file)) {
        return "";
    }
    return Utils::readContentsAsString(head_file);
}
std::map<std::string, std::string> Commit::loadFileBlobs(const std::string& commit_sha) {
    std::string tree_sha = getTreeShaFromCommit(commit_sha);
    return loadTreeObject(tree_sha);
}
void Commit::updateHead(const std::string& new_commit_sha) {
    std::string head_file = Utils::join(Repository::getGitliteDir(), "HEAD");
    Utils::writeContents(head_file, new_commit_sha);
}
std::string Commit::getCurrentTimeString() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}
bool Commit::isFileTrackedInCurrentCommit(const std::string& filename) {
    std::string current_sha = getCurrentCommitSha();
    if (current_sha.empty()) {
        return false;
    }
    auto tracked_files = loadFileBlobs(current_sha);
    return tracked_files.find(filename) != tracked_files.end();
}
std::vector<std::string> Commit::getCommitParents(const std::string& commit_sha) {
    std::vector<std::string> parents;
    std::string commit_file = Utils::join(Repository::getGitliteDir(), "commits", commit_sha);
    if (!Utils::isFile(commit_file)) {
        throw std::runtime_error("Commit not found: " + commit_sha);
    }

    std::string content = Utils::readContentsAsString(commit_file);
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.substr(0, 8) == "parents: ") {
            std::string parents_str = line.substr(8);
            std::istringstream parents_iss(parents_str);
            std::string parent_sha;
            while (std::getline(parents_iss, parent_sha, ',')) {
                if (!parent_sha.empty()) {
                    parents.push_back(parent_sha);
                }
            }
            break;
        }
    }
    return parents;
}
std::string Commit::getCommitMessage(const std::string& commit_sha) {
    std::string commit_file = Utils::join(Repository::getGitliteDir(), "commits", commit_sha);
    if (!Utils::isFile(commit_file)) {
        throw std::runtime_error("Commit not found: " + commit_sha);
    }

    std::string content = Utils::readContentsAsString(commit_file);
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.substr(0, 8) == "message: ") {
            return line.substr(8);
        }
    }
    return "";
}
std::string Commit::getCommitTimestamp(const std::string& commit_sha) {
    std::string commit_file = Utils::join(Repository::getGitliteDir(), "commits", commit_sha);
    if (!Utils::isFile(commit_file)) {
        throw std::runtime_error("Commit not found: " + commit_sha);
    }
    std::string content = Utils::readContentsAsString(commit_file);
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.substr(0, 10) == "timestamp: ") {
            return line.substr(10);
        }
    }
    return "";
}    
std::vector<uint8_t> Commit::serialize() const {
    std::vector<uint8_t> data;
    data.insert(data.end(), message_.begin(), message_.end());
    data.push_back('\0');
    uint32_t parent_count = static_cast<uint32_t>(parents_.size());
    uint8_t* parent_count_bytes = reinterpret_cast<uint8_t*>(&parent_count);
    data.insert(data.end(), parent_count_bytes, parent_count_bytes + sizeof(uint32_t));
    for (const auto& parent : parents_) {
        data.insert(data.end(), parent.begin(), parent.end());
        data.push_back('\0');
    }
    data.insert(data.end(), timestamp_.begin(), timestamp_.end());
    data.push_back('\0');
    uint32_t file_count = static_cast<uint32_t>(file_blobs_.size());
    uint8_t* file_count_bytes = reinterpret_cast<uint8_t*>(&file_count);
    data.insert(data.end(), file_count_bytes, file_count_bytes + sizeof(uint32_t));
    for (const auto& [filename, blob_sha] : file_blobs_) {
        data.insert(data.end(), filename.begin(), filename.end());
        data.push_back('\0');
        data.insert(data.end(), blob_sha.begin(), blob_sha.end());
    }
    if (!sha1_.empty()) {
        data.insert(data.end(), sha1_.begin(), sha1_.end());
        data.push_back('\0');
    }
    return data;
}
std::string Commit::createTreeObject(const std::map<std::string, std::string>& entries) {
    std::string tree_content;
    for (const auto& [filename, blob_sha] : entries) {
        tree_content += filename + ":" + blob_sha + "\n";
    }
    std::string header = "tree " + std::to_string(tree_content.size());
    std::string full_data = header + '\0' + tree_content;
    std::string tree_sha = Utils::sha1(full_data);
    std::string object_dir = Repository::getObjectsDir() + "/" + tree_sha.substr(0, 2);
    std::string object_path = object_dir + "/" + tree_sha.substr(2);
    
    Utils::createDirectories(object_dir);
    Utils::writeContents(object_path, full_data);
    
    return tree_sha;
}
std::map<std::string, std::string> Commit::loadTreeObject(const std::string& tree_sha) {
    std::map<std::string, std::string> entries;
    
    if (tree_sha.empty()) {
        return entries;
    }
    std::string object_path = Repository::getObjectsDir() + "/" + 
                             tree_sha.substr(0, 2) + "/" + 
                             tree_sha.substr(2);
    
    if (!Utils::exists(object_path)) {
        return entries;
    }
    
    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos) {
        return entries;
    }
    
    std::string tree_content = content.substr(null_pos + 1);
    std::istringstream iss(tree_content);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string filename = line.substr(0, colon_pos);
            std::string blob_sha = line.substr(colon_pos + 1);
            entries[filename] = blob_sha;
        }
    }
    
    return entries;
}
std::string Commit::getTreeShaFromCommit(const std::string& commit_sha) {
    if (commit_sha.empty()) {
        return "";
    }
    
    std::string object_path = Repository::getObjectsDir() + "/" + 
                             commit_sha.substr(0, 2) + "/" + 
                             commit_sha.substr(2);
    
    if (!Utils::exists(object_path)) {
        return "";
    }
    
    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos) {
        return "";
    }
    
    std::string commit_content = content.substr(null_pos + 1);
    std::istringstream iss(commit_content);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.empty()) break;
        
        if (line.substr(0, 5) == "tree ") {
            return line.substr(5);
        }
    }
    
    return "";
}

std::string Commit::createNewTreeFromStaging(
    const std::string& parent_commit_sha,
    const std::map<std::string, std::string>& staged_files,
    const std::vector<std::string>& removed_files) {
    //获取父提交的 tree
    std::string parent_tree_sha = getTreeShaFromCommit(parent_commit_sha);
    std::map<std::string, std::string> entries;
    
    if (!parent_tree_sha.empty()) {
        entries = loadTreeObject(parent_tree_sha);
    }
    //应用暂存区的更改
    for (const auto& [filename, blob_sha] : staged_files) {
        entries[filename] = blob_sha;
    }
    //应用删除
    for (const auto& filename : removed_files) {
        entries.erase(filename);
    }
    //创建新的 tree 对象
    if (entries.empty()) {
        return "4b825dc642cb6eb9a060e54bf8d69288fbee4904";
    }
    
    return createTreeObject(entries);
}
