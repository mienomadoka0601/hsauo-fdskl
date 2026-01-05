#include "Blob.h"
#include "Utils.h"
#include "Repository.h"
#include <sstream>
Blob::Blob(const std::vector<uint8_t> &content) : content(content) {}
Blob::Blob(const std::string &content) : content(content.begin(), content.end()) {}

std::vector<uint8_t> Blob::serialize() const
{
    std::string content_str = getContentAsString();
    std::string header = "blob " + std::to_string(content_str.size());
    std::string full_data = header + '\0' + content_str;
    return std::vector<uint8_t>(full_data.begin(), full_data.end());
}
std::string Blob::getContentAsString() const
{
    return std::string(reinterpret_cast<const char *>(content.data()), content.size());
}
std::string Blob::getOid() const
{

    auto data = serialize();

    std::string oid = Utils::sha1(data);

    return oid;
}

void Blob::save() const
{
    auto data = serialize();
    std::string full_data(data.begin(), data.end());

    std::string oid = Utils::sha1(full_data);

    auto serialized = serialize();
    std::string serialized_oid = Utils::sha1(serialized);
    std::string object_dir = Repository::getObjectsDir() + "/" + oid.substr(0, 2);
    std::string object_path = object_dir + "/" + oid.substr(2);
    Utils::createDirectories(object_dir);
    Utils::writeContents(object_path, full_data);
}