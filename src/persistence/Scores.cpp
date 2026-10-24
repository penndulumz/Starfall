#include "Scores.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace starfall {

namespace {
// These limits describe the existing V1 storage contract, independent of rendering.
constexpr std::size_t MaximumNameLength = 12;
constexpr std::size_t MaximumEntries = 10;
constexpr int MaximumPoints = 1000000000;
constexpr int LastWave = 10;
constexpr int MaximumSeconds = 86400;
constexpr int MaximumLoadedRows = 1000;
constexpr std::size_t MaximumRowLength = 256;
constexpr const char* FileHeader = "STARFALL_SCORES_V1";

bool validScore(const Score& score) {
    return score.points >= 0 && score.points <= MaximumPoints &&
        score.wave >= 1 && score.wave <= LastWave &&
        score.seconds >= 0 && score.seconds <= MaximumSeconds;
}
} // namespace

Scores::Scores(std::filesystem::path path) : directory(std::move(path)) {
    load();
}

std::string Scores::cleanName(const std::string& name) {
    std::string result;
    for (unsigned char ch : name) {
        if (ch >= 'a' && ch <= 'z') ch = static_cast<unsigned char>(ch - 'a' + 'A');
        if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ' ' || ch == '-' || ch == '_') {
            if (result.size() < MaximumNameLength) result += static_cast<char>(ch);
        }
    }
    while (!result.empty() && result.back() == ' ') result.pop_back();
    auto first = result.find_first_not_of(' ');
    if (first == std::string::npos) return "PILOT";
    return result.substr(first);
}

void Scores::load() {
    values.clear();
    loadWarning.clear();
    const auto file = directory / "leaderboard.txt";
    std::error_code error;
    const bool exists = std::filesystem::exists(file, error);
    if (error) {
        loadWarning = "SCORES COULD NOT BE READ";
        return;
    }
    if (!exists) return;
    std::ifstream input(file);
    std::string line;
    if (!input || !std::getline(input, line) || line != FileHeader) {
        loadWarning = "SCORE FILE IS UNREADABLE OR HAS AN UNKNOWN FORMAT";
        return;
    }
    int count = 0;
    while (std::getline(input, line) && count++ < MaximumLoadedRows) {
        if (line.size() > MaximumRowLength) continue;
        std::istringstream row(line);
        // V1 writes quoted names. std::quoted also accepts bare words, so check
        // the opening quote explicitly rather than accepting a malformed row.
        row >> std::ws;
        if (row.peek() != '"') continue;
        Score value;
        int won = 0;
        if (!(row >> std::quoted(value.name) >> value.points >> value.wave >> value.seconds >> won)) continue;
        row >> std::ws;
        if (!row.eof() || !validScore(value) || (won != 0 && won != 1) ||
            value.name.size() > MaximumNameLength) continue;
        value.name = cleanName(value.name);
        value.victory = won == 1;
        values.push_back(value);
    }
    std::stable_sort(values.begin(), values.end(), [](const Score& left, const Score& right) {
        return left.points > right.points;
    });
    if (values.size() > MaximumEntries) values.resize(MaximumEntries);
}

SaveResult Scores::submit(Score value) {
    value.name = cleanName(value.name);
    if (!validScore(value))
        return {false, 0, "INVALID SCORE"};
    auto candidate = values;
    // Equal scores keep their existing order. One new completed run is one entry.
    const auto position = std::find_if(candidate.begin(), candidate.end(), [&](const Score& old) {
        return value.points > old.points;
    });
    const int rank = static_cast<int>(position - candidate.begin()) + 1;
    candidate.insert(position, value);
    if (candidate.size() > MaximumEntries) candidate.resize(MaximumEntries);
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) return {false, 0, "CANNOT CREATE SAVE FOLDER"};
    const auto temporary = directory / "leaderboard.tmp";
    const auto destination = directory / "leaderboard.txt";
    std::ofstream output(temporary, std::ios::trunc);
    if (!output) return {false, 0, "CANNOT WRITE SCORES - CHECK FOLDER PERMISSIONS"};
    output << FileHeader << '\n';
    for (const auto& entry : candidate) {
        output << std::quoted(entry.name) << ' ' << entry.points << ' ' << entry.wave << ' '
               << entry.seconds << ' ' << (entry.victory ? 1 : 0) << '\n';
    }
    // Do not publish either the in-memory candidate or the destination until
    // every write and close succeeds. A failed replacement can safely be retried.
    output.flush();
    if (!output) return {false, 0, "SCORE WRITE FAILED - TRY AGAIN"};
    output.close();
    if (!output) return {false, 0, "SCORE WRITE FAILED - TRY AGAIN"};
#ifdef _WIN32
    const bool replaced = MoveFileExW(temporary.c_str(), destination.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::filesystem::rename(temporary, destination, error);
    const bool replaced = !error;
#endif
    if (!replaced) return {false, 0, "CANNOT REPLACE SCORE FILE - TRY AGAIN"};
    values = std::move(candidate);
    loadWarning.clear();
    return {true, rank <= static_cast<int>(MaximumEntries) ? rank : 0, {}};
}

} // namespace starfall
