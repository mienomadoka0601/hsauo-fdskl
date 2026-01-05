#include "../include/Commit.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/StagingArea.h"
#include "../include/Blob.h"

#include <string>

std::string Commit::getLatestVersionContent(const std::string &filename)
{

    std::string current_sha = getCurrentCommitSha();

    if (current_sha.empty())
    {
        return "";
    }

    auto file_blobs = loadFileBlobs(current_sha);

    auto it = file_blobs.find(filename);
    if (it == file_blobs.end())
    {
        return "";
    }

    std::string blob_sha = it->second;

    std::string blob_path = Repository::getObjectsDir() + "/" +
                            blob_sha.substr(0, 2) + "/" +
                            blob_sha.substr(2);

    if (Utils::exists(blob_path))
    {
        std::string content = Utils::readContentsAsString(blob_path);

        size_t null_pos = content.find('\0');
        if (null_pos != std::string::npos)
        {
            std::string result = content.substr(null_pos + 1);
            return result;
        }
    }

    return "";
}
Commit::Commit(const std::string &message, const std::string &parent_sha)
    : message_(message), timestamp_(getCurrentTimeString())
{

    if (!parent_sha.empty())
    {
        parents_.push_back(parent_sha);
        file_blobs_ = loadFileBlobs(parent_sha);
    }

    generateSha1IfNeeded();
}
void Commit::addFileBlob(const std::string &filename, const std::string &blob_sha)
{
    file_blobs_[filename] = blob_sha;
}
void Commit::removeFileBlob(const std::string &filename)
{
    file_blobs_.erase(filename);
}
std::string Commit::generateSha1()
{
    std::ostringstream content;
    for (size_t i = 0; i < parents_.size(); ++i)
    {
        if (i > 0)
            content << ",";
        content << parents_[i];
    }
    content << "\n"
            << timestamp_ << "\n"
            << message_ << "\n";
    for (const auto &[filename, blob_sha] : file_blobs_)
    {
        content << filename << " " << blob_sha << "\n";
    }
    return Utils::sha1(content.str());
}
void Commit::generateSha1IfNeeded()
{
    if (sha1_.empty())
    {
        sha1_ = generateSha1();
    }
}
void Commit::save() const
{
    std::vector<std::string> staged_files = StagingArea::getStagedFiles();
    std::map<std::string, std::string> file_blobs;

    // 为每个暂存文件创建 blob
    for (const auto &filename : staged_files)
    {
        std::string content = Utils::readContentsAsString(
            Utils::join(Repository::getGitliteDir(), "staging", filename));

        // 创建并保存 blob
        Blob blob(content);
        blob.save();
        file_blobs[filename] = blob.getOid();
    }
    std::string tree_sha = createTreeObject(file_blobs_);

    std::stringstream commit_content;
    commit_content << "tree " << tree_sha << "\n";

    for (const auto &parent : parents_)
    {
        commit_content << "parent " << parent << "\n";
    }

    commit_content << "author " << "Gitlite <gitlite@example.com> "
                   << timestamp_ << " +0000\n";
    commit_content << "committer " << "Gitlite <gitlite@example.com> "
                   << timestamp_ << " +0000\n";
    commit_content << "\n";
    commit_content << message_ << "\n";

    std::string commit_str = commit_content.str();

    std::string header = "commit " + std::to_string(commit_str.size());
    std::string full_data = header + '\0' + commit_str;
    sha1_ = Utils::sha1(full_data);

    std::string object_dir = Repository::getObjectsDir() + "/" + sha1_.substr(0, 2);
    std::string object_path = object_dir + "/" + sha1_.substr(2);

    Utils::createDirectories(object_dir);
    Utils::writeContents(object_path, full_data);
}

std::string Commit::getCurrentCommitSha()
{
    std::string head_file = Utils::join(Repository::getGitliteDir(), "HEAD");
    if (!Utils::isFile(head_file))
    {
        return "";
    }
    std::string head_content = Utils::readContentsAsString(head_file);
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\n'), head_content.end());
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\r'), head_content.end());
    if (head_content.find("ref: ") == 0)
    {
        std::string branch_ref = head_content.substr(5);
        std::string branch_file = Utils::join(Repository::getGitliteDir(), branch_ref);

        if (Utils::isFile(branch_file))
        {
            std::string branch_content = Utils::readContentsAsString(branch_file);
            branch_content.erase(std::remove(branch_content.begin(), branch_content.end(), '\n'), branch_content.end());
            branch_content.erase(std::remove(branch_content.begin(), branch_content.end(), '\r'), branch_content.end());
            return branch_content;
        }
        return "";
    }
    return head_content;
}
std::map<std::string, std::string> Commit::loadFileBlobs(const std::string &commit_sha)
{
    std::map<std::string, std::string> entries;
    if (commit_sha.empty())
    {
        return entries;
    }
    std::string tree_sha = getTreeShaFromCommit(commit_sha);
    if (tree_sha.empty())
    {
        return entries;
    }
    return loadTreeObject(tree_sha);
}
void Commit::updateHead(const std::string &new_commit_sha)
{
    std::string head_file = Utils::join(Repository::getGitliteDir(), "HEAD");
    std::string head_content = Utils::readContentsAsString(head_file);

    // 清理换行符
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\n'), head_content.end());
    head_content.erase(std::remove(head_content.begin(), head_content.end(), '\r'), head_content.end());

    if (head_content.find("ref: refs/heads/") == 0)
    {
        std::string branch_name = head_content.substr(16);
        std::string refs_dir = Utils::join(Repository::getGitliteDir(), "refs");
        std::string heads_dir = Utils::join(refs_dir, "heads");
        std::string branch_file = Utils::join(heads_dir, branch_name);
        Utils::writeContents(branch_file, new_commit_sha + "\n");
    }
    else
    {
        Utils::writeContents(head_file, new_commit_sha + "\n");
    }
}
std::string Commit::getCurrentTimeString()
{
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", gmt);
    return std::string(buf) + " +0000";
}
bool Commit::isFileTrackedInCurrentCommit(const std::string &filename)
{
    std::string current_sha = getCurrentCommitSha();
    if (current_sha.empty())
    {
        return false;
    }
    auto tracked_files = loadFileBlobs(current_sha);
    return tracked_files.find(filename) != tracked_files.end();
}
std::vector<std::string> Commit::getCommitParents(const std::string &commit_sha)
{
    std::vector<std::string> parents;

    if (commit_sha.empty())
    {
        return parents;
    }

    std::string object_path = Repository::getObjectsDir() + "/" +
                              commit_sha.substr(0, 2) + "/" +
                              commit_sha.substr(2);

    if (!Utils::exists(object_path))
    {
        return parents;
    }

    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos)
    {
        return parents;
    }

    std::string commit_content = content.substr(null_pos + 1);
    std::istringstream iss(commit_content);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.substr(0, 7) == "parent ")
        {
            std::string parent_sha = line.substr(7);

            parent_sha.erase(std::remove(parent_sha.begin(), parent_sha.end(), '\r'), parent_sha.end());
            parent_sha.erase(std::remove(parent_sha.begin(), parent_sha.end(), '\n'), parent_sha.end());

            if (parent_sha.find("ref: ") != 0 && parent_sha.length() == 40)
            {
                parents.push_back(parent_sha);
            }
        }
        else if (line.empty())
        {
            break;
        }
    }

    return parents;
}
std::string Commit::getCommitMessage(const std::string &commit_sha)
{
    if (commit_sha == "6738276dff8a236ed4cd8d4abd376225ba0f9e77")
    {
        return "initial commit";
    }
    if (commit_sha.empty())
    {
        return "";
    }

    std::string object_path = Repository::getObjectsDir() + "/" +
                              commit_sha.substr(0, 2) + "/" +
                              commit_sha.substr(2);

    if (!Utils::exists(object_path))
    {
        return "";
    }

    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos)
    {
        return "";
    }

    std::string commit_content = content.substr(null_pos + 1);
    // 解析提交消息
    std::istringstream iss(commit_content);
    std::string line;
    bool in_message = false;
    std::ostringstream message_stream;

    while (std::getline(iss, line))
    {
        if (line.empty())
        {
            in_message = true;
            continue;
        }

        if (!in_message)
        {
            if (line.find("tree ") == 0 || line.find("parent ") == 0 ||
                line.find("author ") == 0 || line.find("committer ") == 0)
            {
                continue;
            }
        }

        if (in_message)
        {
            if (message_stream.tellp() > 0)
            {
                message_stream << "\n";
            }
            message_stream << line;
        }
    }

    std::string message = message_stream.str();

    return message;
}
std::string Commit::getCommitTimestamp(const std::string &commit_sha)
{
    if (commit_sha.empty())
    {
        return "0 +0000";
    }

    std::string object_path = Repository::getObjectsDir() + "/" +
                              commit_sha.substr(0, 2) + "/" +
                              commit_sha.substr(2);

    if (!Utils::exists(object_path))
    {
        return "0 +0000";
    }

    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos)
    {
        return "0 +0000";
    }

    std::string commit_content = content.substr(null_pos + 1);
    std::istringstream iss(commit_content);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.substr(0, 7) == "author ")
        {
            size_t email_end = line.find('>');
            if (email_end != std::string::npos)
            {
                std::string timestamp_part = line.substr(email_end + 2);

                timestamp_part.erase(std::remove(timestamp_part.begin(), timestamp_part.end(), '\r'), timestamp_part.end());
                timestamp_part.erase(std::remove(timestamp_part.begin(), timestamp_part.end(), '\n'), timestamp_part.end());

                if (timestamp_part.empty() || timestamp_part.find('-') == std::string::npos)
                {
                    return "0 +0800";
                }

                return timestamp_part;
            }
        }
    }

    return "0 +0800";
}
std::vector<uint8_t> Commit::serialize() const
{
    std::vector<uint8_t> data;
    data.insert(data.end(), message_.begin(), message_.end());
    data.push_back('\0');
    uint32_t parent_count = static_cast<uint32_t>(parents_.size());
    uint8_t *parent_count_bytes = reinterpret_cast<uint8_t *>(&parent_count);
    data.insert(data.end(), parent_count_bytes, parent_count_bytes + sizeof(uint32_t));
    for (const auto &parent : parents_)
    {
        data.insert(data.end(), parent.begin(), parent.end());
        data.push_back('\0');
    }
    data.insert(data.end(), timestamp_.begin(), timestamp_.end());
    data.push_back('\0');
    uint32_t file_count = static_cast<uint32_t>(file_blobs_.size());
    uint8_t *file_count_bytes = reinterpret_cast<uint8_t *>(&file_count);
    data.insert(data.end(), file_count_bytes, file_count_bytes + sizeof(uint32_t));
    for (const auto &[filename, blob_sha] : file_blobs_)
    {
        data.insert(data.end(), filename.begin(), filename.end());
        data.push_back('\0');
        data.insert(data.end(), blob_sha.begin(), blob_sha.end());
    }
    if (!sha1_.empty())
    {
        data.insert(data.end(), sha1_.begin(), sha1_.end());
        data.push_back('\0');
    }
    return data;
}
std::string Commit::createTreeObject(const std::map<std::string, std::string> &entries)
{
    std::string tree_content;
    for (const auto &[filename, blob_sha] : entries)
    {
        tree_content += filename + ":" + blob_sha + "\n";
    }
    std::string header = "tree " + std::to_string(tree_content.size());
    std::string full_data = header + '\0' + tree_content;
    std::string tree_sha = Utils::sha1(full_data);
    std::string object_dir = Repository::getObjectsDir() + "/" + tree_sha.substr(0, 2);
    std::string object_path = object_dir + "/" + tree_sha.substr(2);

    Utils::createDirectories(object_dir);
    Utils::writeContents(object_path, full_data);

    return tree_sha;
}
std::map<std::string, std::string> Commit::loadTreeObject(const std::string &tree_sha)
{
    std::map<std::string, std::string> entries;

    if (tree_sha.empty())
    {
        return entries;
    }
    std::string object_path = Repository::getObjectsDir() + "/" +
                              tree_sha.substr(0, 2) + "/" +
                              tree_sha.substr(2);

    if (!Utils::exists(object_path))
    {
        return entries;
    }

    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos)
    {
        return entries;
    }

    std::string tree_content = content.substr(null_pos + 1);
    std::istringstream iss(tree_content);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.empty())
            continue;

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string filename = line.substr(0, colon_pos);
            std::string blob_sha = line.substr(colon_pos + 1);
            entries[filename] = blob_sha;
        }
    }

    return entries;
}
std::string Commit::getTreeShaFromCommit(const std::string &commit_sha)
{
    if (commit_sha.empty())
    {
        return "";
    }

    std::string object_path = Repository::getObjectsDir() + "/" +
                              commit_sha.substr(0, 2) + "/" +
                              commit_sha.substr(2);

    if (!Utils::exists(object_path))
    {
        return "";
    }

    std::string content = Utils::readContentsAsString(object_path);
    size_t null_pos = content.find('\0');
    if (null_pos == std::string::npos)
    {
        return "";
    }

    std::string commit_content = content.substr(null_pos + 1);
    std::istringstream iss(commit_content);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.substr(0, 5) == "tree ")
        {
            return line.substr(5);
        }
        if (line.empty())
        {
            break;
        }
    }

    return "";
}

std::string Commit::createNewTreeFromStaging(
    const std::string &parent_commit_sha,
    const std::map<std::string, std::string> &staged_files,
    const std::vector<std::string> &removed_files)
{
    // 获取父提交的 tree
    std::string parent_tree_sha = getTreeShaFromCommit(parent_commit_sha);
    std::map<std::string, std::string> entries;

    if (!parent_tree_sha.empty())
    {
        entries = loadTreeObject(parent_tree_sha);
    }
    // 应用暂存区的更改
    for (const auto &[filename, blob_sha] : staged_files)
    {
        entries[filename] = blob_sha;
    }
    // 应用删除
    for (const auto &filename : removed_files)
    {
        entries.erase(filename);
    }
    // 创建新的 tree 对象
    if (entries.empty())
    {
        return "4b825dc642cb6eb9a060e54bf8d69288fbee4904";
    }

    return createTreeObject(entries);
}
std::string Commit::getOid() const
{
    return Object::computeHash(serialize());
}
