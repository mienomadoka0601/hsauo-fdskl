#include "../include/command/init.h"
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <cstring>
#include <ctime>
#include <chrono>
#include <cstdint>
void InitCommand::init()
{
    const std::string gitliteDir = Repository::getGitliteDir();
    if (Utils::isDirectory(gitliteDir))
    {
        Utils::exitWithMessage("A Gitlite version-control system already exists in the current directory.");
    }

    // Create .gitlite directory structure
    if (!Utils::createDirectories(Utils::join(gitliteDir, "branches")) ||
        !Utils::createDirectories(Utils::join(gitliteDir, "objects")) ||
        !Utils::createDirectories(Utils::join(gitliteDir, "refs", "heads")) ||
        !Utils::createDirectories(Utils::join(gitliteDir, "refs", "remotes")))
    {
        Utils::exitWithMessage("Failed to create Gitlite directory structure.");
    }
    std::stringstream commitData;
    commitData << "author Gitlite <gitlite@example.com> Thu Jan 01 00:00:00 1970 +0000\n"
               << "committer Gitlite <gitlite@example.com> Thu Jan 01 00:00:00 1970 +0000\n"
               << "\n"
               << "initial commit";
    std::string commitStr = commitData.str();
    std::string commitId = Utils::sha1("commit " + std::to_string(commitStr.size()) + "\0" + commitStr);
    std::string objectDir = Utils::join(gitliteDir, "objects", commitId.substr(0, 2));
    std::string objectPath = Utils::join(objectDir, commitId.substr(2));
    if (!Utils::createDirectories(objectDir))
    {
        Utils::exitWithMessage("Failed to create object directory: " + objectDir);
    }
    Utils::writeContents(objectPath, commitStr);
    const std::string headPath = Utils::join(gitliteDir, "HEAD");
    Utils::writeContents(headPath, "ref: refs/heads/master\n");
    const std::string masterRefPath = Utils::join(
        Utils::join(gitliteDir, "refs"),
        Utils::join("heads", "master"));
    Utils::writeContents(masterRefPath, commitId + "\n");
}