#ifndef STAGINGAREA_H
#define STAGINGAREA_H

#include <string>
#include <vector>

class StagingArea {
    public:
        static void stageFile(const std::string& filename);
        static void unstageFile(const std::string& filename);
        static bool isFileStaged(const std::string& filename);
        static bool isFileMarkedForDeletion(const std::string& filename);
        static void markFileForDeletion(const std::string& filename);
        static void removeDeletionMark(const std::string& filename);
        static bool isStagingAreaEmpty();
        static std::vector<std::string> getStagedFiles();
        static std::vector<std::string> getFilesMarkedForDeletion();
        static std::string getBlobShaForFile(const std::string& filename);
        static void clear();
};

#endif // STAGINGAREA_H