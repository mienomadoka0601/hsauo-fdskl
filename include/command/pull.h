#ifndef PULLCOMMAND_H
#define PULLCOMMAND_H

#include <string>

class pullcommand {
public:
    static void pull(const std::string& remote_name, const std::string& remote_branch);
};

#endif // PULLCOMMAND_H