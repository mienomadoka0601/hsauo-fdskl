#include "../include/command/add.h"
#include "../include/Commit.h"

void AddCommand::add(const std::string& filename) {
    // Implementation of the add command goes here
    if(!Utils::isFile(filename)) {
        Utils::exitWithMessage("File does not exist: " + filename);
    }

    std::string current_content=Utils::readContentsAsString(filename);
    std::string staged_path=Utils::join(Repository::getGitliteDir(), "staging", filename);
    std::string staged_content=Utils::isFile(staged_path) ? Utils::readContentsAsString(staged_path) : "";
    std::string committed_content=Commit::getLatestVersionContent(filename);

    //如果此时（文件的当前版本与当前提交中的版本相同）文件已在暂存区，则将其从暂存区中移除
    if(StagingArea::isFileStaged(filename) && current_content==staged_content) {
        StagingArea::unstageFile(filename);
        return;
    }

    //如果文件的当前版本与当前提交中的版本相同，则不要将其暂存以待添加
    if(committed_content==current_content) return;

    //如果文件处于暂存状态且标记为待删除（参见命令`rm`），则将其待删除状态移除
    if(StagingArea::isFileStaged(filename) && StagingArea::isFileMarkedForDeletion(filename)){
        StagingArea::removeDeletionMark(filename);
    }

    StagingArea::stageFile(filename);

}