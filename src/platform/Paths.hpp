#pragma once

#include <filesystem>

namespace starfall {

// Packaged data follows the executable, never the process working directory.
// Save data has a separate current-user preference location.
std::filesystem::path resourceDirectory();

} // namespace starfall
