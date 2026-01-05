#include "../include/SomeObj.h"
#include "../include/command/init.h"
#include "../include/command/add.h"
#include "../include/command/commit.h"
#include "../include/command/rm.h"
#include "../include/command/log.h"
#include "../include/command/global-log.h"
#include "../include/command/status.h"
#include "../include/command/find.h"
#include "../include/command/rm-remote.h"
#include "../include/command/checkout.h"
#include "../include/command/branch.h"
#include "../include/command/rm-branch.h"
#include "../include/command/reset.h"
#include "../include/command/remote.h"
#include "../include/command/merge.h"
#include "../include/command/pull.h"
#include "../include/command/push.h"
#include "../include/command/fetch.h"
#include <iostream>

void SomeObj::find(const std::string& pattern) {
    findcommand::find(pattern);
}

void SomeObj::addRemote(const std::string& name, const std::string& url) {
    remotecommand::addRemote(name,url);
}

void SomeObj::init() {
    InitCommand::init();
}

void SomeObj::commit(const std::string& message) {
    commitcommand::commit(message);
}

void SomeObj::rmRemote(const std::string& name) {
    remotecommand::removeRemote(name);
}

void SomeObj::add(const std::string& filename) {
    AddCommand::add(filename);
}

void SomeObj::globalLog() {
    globallogcommand::globalLog();
}

void SomeObj::rm(const std::string& filename) {
    rmcommand::rm(filename);
}

void SomeObj::log() {
    logcommand::log();
}
void SomeObj::status(){
    statuscommand::status();
}
void SomeObj::checkoutBranch(const std::string& filename){
    checkoutcommand::checkoutBranch(filename);
}
void SomeObj::checkoutFile(const std::string& filename){
    checkoutcommand::checkout(filename);
}
void SomeObj::checkoutFileInCommit(const std::string& commit_id, const std::string& filename){
    checkoutcommand::checkout(commit_id,filename);
}
void SomeObj::branch(const std::string& branch_name){
    branchcommand::branch(branch_name);
}
void SomeObj::rmBranch(const std::string& branch_name){
    rmbranchcommand::rmBranch(branch_name);
}
void SomeObj::reset(const std::string& commit_id){
    resetcommand::reset(commit_id);
}
void SomeObj::merge(const std::string& branch_name) {
    mergecommand::merge(branch_name);
}
void SomeObj::push(const std::string& remote, const std::string& remote_branch) {
    pushcommand::push(remote, remote_branch);
}
void SomeObj::pull(const std::string& remote, const std::string& remote_branch) {
    pullcommand::pull(remote, remote_branch);
}
void SomeObj::fetch(const std::string& remote, const std::string& remote_branch) {
    fetchcommand::fetch(remote, remote_branch);
}