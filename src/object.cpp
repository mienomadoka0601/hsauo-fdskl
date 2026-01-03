#include "object.h"
#include "Utils.h"
#include "Blob.h"
#include "Commit.h"
#include "Repository.h"
#include "GitliteException.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

std::string Object::computeHash(const std::vector<uint8_t>& data) const {
    return Utils::sha1(data);
}

std::string Object::getOid() const {
    auto data = serialize();
    return computeHash(data);
}

void Object::save() const {
    std::string oid = getOid();
    std::string object_path = getObjectPath(oid);
    std::filesystem::path dir_path = std::filesystem::path(object_path).parent_path();
    if (!Utils::createDirectories(dir_path.string())) {
        throw GitliteException("Failed to create object directory: " + dir_path.string());
    }
    std::vector<uint8_t> data = serialize();
    Utils::writeContents(object_path, data);
}
std::shared_ptr<Object> Object::load(const std::string& oid) {
    if (oid.size() != 40) {
        throw GitliteException("Invalid OID: " + oid);
    }
    
    std::string object_path = Object::getObjectPath(oid);
    if (!Utils::isFile(object_path)) {
        throw GitliteException("Object not found: " + oid);
    }
    
    std::vector<uint8_t> data = Utils::readContents(object_path);
    auto null_pos = std::find(data.begin(), data.end(), '\0');
    std::string type_str(reinterpret_cast<const char*>(data.data()), 
                     std::distance(data.begin(), null_pos));
    
    if (type_str == "blob") {
        return deserialize<Blob>(data);
    } else if (type_str == "commit") {
        return deserialize<Commit>(data);
    } else {
        throw GitliteException("Unknown object type: " + type_str);
    }
}

std::string Object::getObjectPath(const std::string& oid){
    if (oid.size() != 40) {
        throw GitliteException("OID must be 40 characters long");
    }
    
    std::string dir = oid.substr(0, 2);
    std::string file = oid.substr(2);
    std::string objects_dir = Utils::join(Repository::getGitliteDir(), "objects");
    std::string dir_path = Utils::join(objects_dir, dir);
    return Utils::join(dir_path, file);
}

template <>
std::shared_ptr<Blob> Object::deserialize<Blob>(const std::vector<uint8_t>& data) {
    size_t null_pos = 0;
    while (null_pos < data.size() && data[null_pos] != '\0') {
        null_pos++;
    }
    
    if (null_pos >= data.size()) {
        Utils::exitWithMessage("Invalid blob format");
    }
    
    std::vector<uint8_t> content(data.begin() + null_pos + 1, data.end());
    return std::make_shared<Blob>(content);
}

template <>
std::shared_ptr<Commit> Object::deserialize<Commit>(const std::vector<uint8_t>& data) {
    // 找到类型头部分离符
    size_t null_pos = 0;
    while (null_pos < data.size() && data[null_pos] != '\0') {
        null_pos++;
    }
    
    if (null_pos >= data.size()) {
        Utils::exitWithMessage("Invalid commit format: missing null separator");
    }
    std::string type_str(data.begin(), data.begin() + null_pos);
    
    // 验证类型
    if (type_str.substr(0, 7) != "commit ") {
        Utils::exitWithMessage("Invalid commit type: " + type_str);
    }
    std::string commit_content(data.begin() + null_pos + 1, data.end());
    
    // 解析提交内容
    std::istringstream iss(commit_content);
    std::string line;
    std::string message;
    std::string timestamp;
    std::vector<std::string> parents;
    
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) {
            break;
        }
        
        if (line.substr(0, 7) == "parent ") {
            parents.push_back(line.substr(7));
        }
    }
    
    // 读取提交消息（可能有多行）
    std::ostringstream message_stream;
    bool first_line = true;
    while (std::getline(iss, line)) {
        // 清理回车符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (!first_line) {
            message_stream << "\n";
        }
        message_stream << line;
        first_line = false;
    }
    
    message = message_stream.str();
    
    std::string parent_sha = parents.empty() ? "" : parents[0];
    auto commit = std::make_shared<Commit>(message, parent_sha);
    
    return commit;
}