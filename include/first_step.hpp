#pragma once

#include <cstdint>
#include <vector>
#include <ostream>
#include <unordered_set>
#include <limits>

#include "../utils/problem_arguments.hpp"

struct FirstStepAnswer {

    using score_type = int64_t;
    using points_type = InputData::points_type;
    static constexpr score_type default_value = std::numeric_limits<score_type>::min() + 1;

    /// значение целевой функции
    score_type value = default_value;
    score_type distance = 0;
    score_type time = 0;
    /// список вершин в порядке обхода в пути
    std::vector<points_type> vertexes{0};

    bool IsVertexInPath(points_type id);

    void AddVertex(points_type vertex);
};

template<bool is_time_dependent = false>
FirstStepAnswer DoFirstStep(const InputData &input);

std::ostream &operator<<(std::ostream &os, const FirstStepAnswer &answer);

