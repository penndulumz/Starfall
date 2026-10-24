#pragma once

#include "persistence/Scores.hpp"
#include <fstream>
#include <iterator>

namespace starfall::testing {

inline bool sameScores(const std::vector<Score>& left, const std::vector<Score>& right) {
    if (left.size() != right.size()) return false;
    for (std::size_t index = 0; index < left.size(); ++index) {
        const auto& first = left[index];
        const auto& second = right[index];
        if (first.name != second.name || first.points != second.points || first.wave != second.wave ||
            first.seconds != second.seconds || first.victory != second.victory) return false;
    }
    return true;
}

inline std::string scoreFileBytes(const std::filesystem::path& file) {
    std::ifstream input(file, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

inline std::vector<Score> legacyScores() {
    return {{"LEGACY-ACE", 50000, 10, 250, true},
            {"EARLY-TIE", 2000, 4, 95, false},
            {"LATER-TIE", 2000, 6, 180, false},
            {"OLD-PILOT", 100, 1, 12, false}};
}

} // namespace starfall::testing
