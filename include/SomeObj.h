#ifndef SOMEOBJ_H
#define SOMEOBJ_H

#include <string>

class SomeObj {
public:
    static void find(const std::string& pattern);
    static void addRemote(const std::string& name, const std::string& url);
    static void init();
    static void commit(const std::string& message);
    static void rmRemote(const std::string& name);
    static void add(const std::string& filename);
    static void globalLog();
    static void rm(const std::string& filename);
    static void log();
    static void status();
    static void checkoutBranch(const std::string& filename);
    static void checkoutFile(const std::string& filename);
    static void checkoutFileInCommit(const std::string& commit_id, const std::string& filename);
};

#endif // SOMEOBJ_H
