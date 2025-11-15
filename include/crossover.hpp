#pragma once

#include "path.hpp"
#include <random>

class Crossover {
private:
    std::mt19937 rng;
public:

    Crossover() : rng(std::random_device{}()) {};

    Solution crossover(const Solution &first, const Solution &second, const InputData &inputData);

private:
    Solution PMX(const Solution &first, const Solution &second);

    Solution EXX(const Solution &first, const Solution &second);

    Solution SC(const Solution &first, const Solution &second);
};
