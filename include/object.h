#ifndef OBJECT_H
#define OBJECT_H

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class Blob;
class Commit;
enum class ObjectType {
    BLOB,
    COMMIT
};

class Object {
public:
    virtual ~Object() = default;
    
    std::string computeHash(const std::vector<uint8_t>& data)const;
    virtual std::string getOid() const = 0;
    virtual void save() const = 0;
    virtual ObjectType getType() const = 0;
    virtual std::vector<uint8_t> serialize() const = 0;
    
    static std::shared_ptr<Object> load(const std::string& oid);
    static std::string getObjectPath(const std::string& oid);  // 去掉 const 修饰符
    
protected:
    template<typename T>
    static std::shared_ptr<T> deserialize(const std::vector<uint8_t>& data);
};

template<>
std::shared_ptr<Blob> Object::deserialize<Blob>(const std::vector<uint8_t>& data);

template<>
std::shared_ptr<Commit> Object::deserialize<Commit>(const std::vector<uint8_t>& data);

#endif