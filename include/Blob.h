#ifndef BLOB_H
#define BLOB_H

#include "object.h"

class Blob : public Object {
private:
    std::vector<uint8_t> content;
public:
    explicit Blob(const std::vector<uint8_t>& content);
    explicit Blob(const std::string& content);
    ObjectType getType() const override { return ObjectType::BLOB; }
    std::vector<uint8_t> serialize() const override;
    std::vector<uint8_t> getContent() const { return content; }
    std::string getContentAsString() const;
    std::string getOid() const override;
    void save()const override;
    
};
template <>
std::shared_ptr<Blob> Object::deserialize<Blob>(const std::vector<uint8_t>& data);
#endif // BLOB_H