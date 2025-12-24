#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

enum class ObjectType {
    BLOB,
    COMMIT,
    TREE
};

class Object {
public:
    virtual ~Object() = default;
    virtual ObjectType getType() const = 0;
    virtual std::vector<uint8_t> serialize() const = 0;
    template <typename T>
    static std::shared_ptr<T> deserialize(const std::vector<uint8_t>& data);
    std::string getOid() const;
    void save() const;
    static std::shared_ptr<Object> load(const std::string& oid);
protected:
    std::string getObjectPath(const std::string& oid) const;
    std::string computeHash(const std::vector<uint8_t>& data) const;
};

#endif // OBJECT_H