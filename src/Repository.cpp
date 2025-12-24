#include "../include/Repository.h"
#include "../include/Commit.h"
#include "../include/Utils.h"

#include <string>
#include <filesystem>

std::string getGitliteDir(){
    const std::string gitlite_dir_name=".gitlite";
    std::filesystem::path current_work_dir=std::filesystem::current_path();
    std::filesystem::path gitlite_dir = current_work_dir / gitlite_dir_name;
    return gitlite_dir.string();
}
std::string Repository::getWorkingDir() {
    std::string gitlite_dir = getGitliteDir();
    return Utils::join(gitlite_dir, "..");
}

std::string Repository::getCommitsDir() {
    const std::string commits_dir_name="commits";
    return Utils::join(getGitliteDir(),commits_dir_name);
}
void Repository::createDirectoryIfNotExists(const std::string& dir_path) {
    if (!Utils::isDirectory(dir_path)) {
        if (!Utils::createDirectories(dir_path)) {
            throw GitliteException("Failed to create directory: " + dir_path);
        }
    }
}
std::string Repository::getGitliteDir() {
    return ".gitlite";
}
std::string Repository::getObjectsDir() {
    std::string gitlite_dir = getGitliteDir();
    if (!Utils::isDirectory(gitlite_dir)) {
        return "";
    }
    return Utils::join(gitlite_dir, "objects");
}
