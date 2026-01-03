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
#include <iostream>

void SomeObj::find(const std::string& pattern) {
    findcommand::find(pattern);
}

//void SomeObj::addRemote(const std::string& name, const std::string& url) {
//}

void SomeObj::init() {
    InitCommand::init();
}

void SomeObj::commit(const std::string& message) {
    commitcommand::commit(message);
}

//void SomeObj::rmRemote(const std::string& name) {
//    rmRemotecommand::rmRemote(name);
//}

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
    checkoutcommand::checkout(filename);
}
void SomeObj::checkoutFile(const std::string& filename){
    checkoutcommand::checkout(filename);
}
void SomeObj::checkoutFileInCommit(const std::string& commit_id, const std::string& filename){
    checkoutcommand::checkout(commit_id,filename);
}