#include "../include/vns.hpp"

Solution VNS(const Solution &solution, const InputData &inputData, int nloop, int kMax, double p) {

    Solution best = solution;

    Solution current = VND(solution, kMax, inputData);
    auto level = 1;

    while (level < nloop) {

        auto temp = Perturbation(current, level, p, inputData);
        temp = VND(temp, kMax, inputData);
        bool updated = false;

        if (temp.time < current.time && temp.distance <= inputData.max_distance) {
            current = std::move(temp);
            updated = true;

            if (current.score > best.score) {
                best = current; // только копирование
            }
        }

        ++level;
    }
    return best;
}