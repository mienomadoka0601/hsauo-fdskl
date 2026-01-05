#include "../include/command/pull.h"
#include "../include/command/fetch.h"
#include "../include/command/merge.h"
#include "../include/Utils.h"
#include <iostream>

void pullcommand::pull(const std::string& remote_name, const std::string& remote_branch) {
    // 1. 执行 fetch
    try {
        fetchcommand::fetch(remote_name, remote_branch);
    } catch (...) {
        // fetch 失败会自己退出，这里只是捕获异常防止崩溃
        exit(1);
    }
    
    // 2. 获取远程跟踪分支名
    std::string remote_tracking_branch = remote_name + "/" + remote_branch;
    
    // 3. 执行 merge
    try {
        mergecommand::merge(remote_tracking_branch);
    } catch (...) {
        // merge 失败会自己退出
        exit(1);
    }
    
}