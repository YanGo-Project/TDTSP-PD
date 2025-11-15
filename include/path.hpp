#pragma once

#include <vector>
#include <cstdint>
#include <ostream>

#include "first_step.hpp"

using Vertex = FirstStepAnswer::points_type;
using Path = std::vector<Vertex>;

struct Solution {

    using score_type = FirstStepAnswer::score_type;

    Path tour;
    uint32_t distance;
    uint32_t time;
    int64_t score;

    Solution(int n) : tour(n), distance(0), time(0), score(0) {};

    Solution(const Path &path, uint32_t d = 0, uint32_t t = 0, int64_t v = 0) : tour(path), distance(d), time(t), score(v) {};

    Solution(Path &&path, uint32_t d = 0, uint32_t t = 0, int64_t v = 0) : tour(std::move(path)), distance(d), time(t), score(v) {};

    bool operator==(const Solution &other) const {
        if (time != other.time) {
            return false;
        }
        if (distance != other.distance) {
            return false;
        }
        if (score != other.score) {
            return false;
        }
        if (tour.size() != other.tour.size()) {
            return false;
        }

        for (size_t i = 0; i < tour.size(); ++i) {
            if (tour[i] != other.tour[i]) {
                return false;
            }
        }
        return true;
    }
};

inline std::ostream &operator<<(std::ostream &os, const Solution &solution) {
    os << "Solution size: " << solution.tour.size() << "\n";
    os << "Solution score: " << solution.score << "\n";
    os << "Solution time: " << solution.time << "\n";
    os << "Solution dist: " << solution.distance << "\n";

    os << "Tour:\n";
    os << "[";
    auto path_size = solution.tour.size();
    for (size_t i = 0; i < path_size; ++i) {
        os << solution.tour[i] << (i == path_size - 1 ? "]\n" : ", ");
    }
    return os;
}