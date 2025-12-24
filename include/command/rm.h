#ifndef RM_H
#define RM_H

#include "../include/object.h"
#include "../include/Utils.h"
#include "../include/Repository.h"
#include "../include/StagingArea.h"
#include <string>
class rmcommand : Object {
    public:
        static void rm(const std::string& filename);
};

#endif // RM_H