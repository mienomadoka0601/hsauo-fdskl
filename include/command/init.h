#ifndef INIT_H
#define INIT_H

#include "../include/Utils.h"
#include "../include/GitliteException.h"
#include "../include/Repository.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>
#include <iomanip>

class InitCommand {
    public:
        static void init();
};

#endif // INIT_H