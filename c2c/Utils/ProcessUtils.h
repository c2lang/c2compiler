#ifndef UTILS_PROCESS_UTILS_H
#define UTILS_PROCESS_UTILS_H

#include <string>

namespace C2 {

class ProcessUtils {
public:
    static void run(const std::string& path, const std::string& cmd);
};

}

#endif

