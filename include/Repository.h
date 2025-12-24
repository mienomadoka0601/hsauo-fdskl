#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include "../include/Utils.h"
#include "../include/GitliteException.h"


class Repository{
    public:
    static std::string getGitliteDir();
    static std::string getWorkingDir();
    static std::string getCommitsDir();
    static std::string getObjectsDir();
    private:
    static void createDirectoryIfNotExists(const std::string& dir_path);
};

#endif // REPOSITORY_H
