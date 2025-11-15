#pragma once

#include "path.hpp"
#include "../utils/problem_arguments.hpp"

#include <random>
#include <algorithm>

class PopulationInitializer {
private:
    std::mt19937 rng;
    static constexpr size_t max_iterations = 1000;

public:
    PopulationInitializer() : rng(std::random_device{}()) {}

    void initialize_population(int path_size,
                               int population_size,
                               double beta,
                               int alpha,
                               const InputData &inputData,
                               std::vector<Solution> &population);

private:
    double generate_random_double();

    int generate_random_int(int min, int max);

    void random_tour(Solution &solution, int n, const InputData &input);

    void grasp_tour(Solution &solution, int n, int alpha, const InputData &input);
};
