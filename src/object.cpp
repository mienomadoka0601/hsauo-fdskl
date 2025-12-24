#include "Object.h"
#include "Utils.h"
#include "Repository.h"
#include "GitliteException.h"

#include <filesystem>
#include <fstream>
#include <sstream>
std::string Object::getOid() const {
    return computeHash(serialize());
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
    std::string object_path = Object().getObjectPath(oid);
    if (!Utils::isFile(object_path)) {
        throw GitliteException("Object not found: " + oid);
    }
    std::vector<uint8_t> data = Utils::readContents(object_path);
    std::string type_str(reinterpret_cast<const char*>(data.data()), data.find('\0'));
    ObjectType type;
    if (type_str == "blob") type = ObjectType::BLOB;
    else if (type_str == "commit") type = ObjectType::COMMIT;
    else if (type_str == "tree") type = ObjectType::TREE;
    else throw GitliteException("Unknown object type: " + type_str);
    std::vector<uint8_t> payload(data.begin() + type_str.size() + 1, data.end());
    switch (type) {
        case ObjectType::BLOB:
            return Object::deserialize<Blob>(payload);
        case ObjectType::COMMIT:
            return Object::deserialize<Commit>(payload);
        case ObjectType::TREE:
            return Object::deserialize<Tree>(payload);
        default:
            throw GitliteException("Unsupported object type");
    }
}
std::string Object::getObjectPath(const std::string& oid) const {
    if (oid.size() != 40) {
        throw GitliteException("OID must be 40 characters long");
    }
    std::string dir = oid.substr(0, 2);
    std::string file = oid.substr(2);
    return Utils::join(Repository::getGitliteDir(), "objects", dir, file);
}
std::string Object::computeHash(const std::vector<uint8_t>& data) const {
    return Utils::sha1(std::string(reinterpret_cast<const char*>(data.data()), data.size()));
}
template <typename T>
std::shared_ptr<T> Object::deserialize(const std::vector<uint8_t>& data) {
    throw GitliteException("Unsupported object type for deserialization");
}
template std::shared_ptr<Blob> Object::deserialize<Blob>(const std::vector<uint8_t>&);
template std::shared_ptr<Commit> Object::deserialize<Commit>(const std::vector<uint8_t>&);
template std::shared_ptr<Tree> Object::deserialize<Tree>(const std::vector<uint8_t>&);