#ifndef PULLCOMMAND_H
#define PULLCOMMAND_H

#include <string>

class pullcommand {
public:
    // 执行 pull 命令
    static void pull(const std::string& remote_name, const std::string& remote_branch);
};

#endif // PULLCOMMAND_H