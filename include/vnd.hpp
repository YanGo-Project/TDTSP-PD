#pragma once

#include "path.hpp"
#include "../utils/problem_arguments.hpp"

#include <random>
#include <algorithm>

enum class OptimizationType: int {
    Shift = 1,
    SwapAdjacent,
    SwapAny,
    TwoOpt,
    OrOpt,
};

Solution VND(Solution solution, int kMax, const InputData &inputData);

Solution Perturbation(const Solution& solution, int level, double p, const InputData& inputData);