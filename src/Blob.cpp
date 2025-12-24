#include "Blob.h"
#include "Utils.h"
#include <sstream>
Blob::Blob(const std::vector<uint8_t>& content) : content(content) {}
Blob::Blob(const std::string& content) : content(content.begin(), content.end()) {}

std::vector<uint8_t> Blob::serialize() const {
    std::vector<uint8_t> data;
    std::string type = "blob";
    data.insert(data.end(), type.begin(), type.end());
    data.push_back('\0');
    data.insert(data.end(), content.begin(), content.end());
    return data;
}
std::string Blob::getContentAsString() const {
    return std::string(reinterpret_cast<const char*>(content.data()), content.size());
}
template <>
std::shared_ptr<Blob> Object::deserialize<Blob>(const std::vector<uint8_t>& data) {
    return std::make_shared<Blob>(data);
}