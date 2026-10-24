#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace starfall {

struct Score {
    std::string name;
    int points = 0;
    int wave = 1;
    int seconds = 0;
    bool victory = false;
};
struct SaveResult {
    bool ok = false;
    int rank = 0;
    std::string error;
};

// Owns the retained leaderboard for one explicit directory. The application
// chooses the existing current-user preference path or its --data-dir override.
class Scores {
public:
    explicit Scores(std::filesystem::path directory);
    void load();
    SaveResult submit(Score score);
    const std::vector<Score>& entries() const { return values; }
    const std::string& warning() const { return loadWarning; }
    static std::string cleanName(const std::string& name);

private:
    std::filesystem::path directory;
    std::vector<Score> values;
    std::string loadWarning;
};

} // namespace starfall
