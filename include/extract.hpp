#pragma once

#include <switch.h>

#include <functional>
#include <set>
#include <string>
#include <vector>

#include "constants.hpp"

namespace extract {

    void extract(
        const std::string& filename, const std::string& workingPath = ROOT_PATH, int overwriteInis = 1, std::function<void()> func = []() { return; });
}  // namespace extract