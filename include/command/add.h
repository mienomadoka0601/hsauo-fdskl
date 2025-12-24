#ifndef ADD_H
#define ADD_H

#include "../include/Utils.h"
#include "../include/GitliteException.h"
#include "../include/Repository.h"
#include "../include/StagingArea.h"
#include <string>
#include <vector>

class AddCommand {
    public:
        static void add(const std::string& filename);
};

#endif // ADD_H