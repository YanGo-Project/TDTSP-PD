#pragma once

#include "path.hpp"

struct MetaParameters {
    int population_size;
    int alpha;
    double beta;
    int nloop;
    int kMax;
    double p;
    int max_iter_without_solution;
    size_t max_crossover_candidates;
};

struct IterInfo {
    uint32_t distance;
    uint32_t score;
    uint64_t time;
    uint64_t timestamp; // время в секундах, когда была сделана запись
};

struct Context {
    const ProgramArguments& args;
    const MetaParameters& params;
    std::vector<IterInfo> time_iterations;
};

Solution applyTspTDPDP(Solution&& solution, const InputData &inputData, Context& ctx);