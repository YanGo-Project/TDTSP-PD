#pragma once

#include "path.hpp"

struct MetaParameters {
    int population_size;
    int alpha;
    double beta;
    int nloop;
    int kMax; // <= 4
    double p;
    int max_iter_without_solution;
    size_t max_crossover_candidates;
};

Solution applyTspTDPDP(Solution&& solution, const InputData &inputData, const MetaParameters &params, uint64_t max_time);