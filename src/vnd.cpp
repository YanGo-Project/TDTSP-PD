#include "../include/vnd.hpp"

#include <cassert>
#include <utility>
#include <random>
#include <vector>
#include <initializer_list>

#ifdef DEBUG
#include "../utils/debug.h"
#endif

namespace {
    // Генератор для DoubleBridge
    thread_local std::mt19937 rng_gen(std::random_device{}());

    // Время работы O(n) из-за проблем со вставкой в вектор
    // TODO() подумать над оптимизацией
    Solution shift_move(const Solution &solution, uint32_t i, uint32_t j) {
        Solution temp(0);
        temp.tour.reserve(solution.tour.size());
        auto vertex = solution.tour[i];
        
        if (i < j) {
            // Перемещаем элемент вправо: копируем [0..i), затем [i+1..j), затем vertex, затем [j..end)
            temp.tour.insert(temp.tour.end(), solution.tour.begin(), solution.tour.begin() + i);
            temp.tour.insert(temp.tour.end(), solution.tour.begin() + i + 1, solution.tour.begin() + j);
            temp.tour.push_back(vertex);
            temp.tour.insert(temp.tour.end(), solution.tour.begin() + j, solution.tour.end());
        } else {
            // Перемещаем элемент влево: копируем [0..j), затем vertex, затем [j..i), затем [i+1..end)
            temp.tour.insert(temp.tour.end(), solution.tour.begin(), solution.tour.begin() + j);
            temp.tour.push_back(vertex);
            temp.tour.insert(temp.tour.end(), solution.tour.begin() + j, solution.tour.begin() + i);
            temp.tour.insert(temp.tour.end(), solution.tour.begin() + i + 1, solution.tour.end());
        }

        return temp; // RVO
    }

    // Время работы O(n^3)
    Solution Shift(const Solution &solution, const InputData &inputData) {
        auto path_size = solution.tour.size();
        auto best = solution;

        // не учитываем начальную и конечную вершину - депо
        for (auto i = 1; i < path_size - 1; ++i) {
            for (auto j = 1; j < path_size - 1; ++j) {
                if (i != j) {
                    auto temp = shift_move(solution, i, j);
                    auto [distance, time, score] = inputData.get_path_time_distance_score(temp.tour);
                    temp.time = time, temp.distance = distance, temp.score = score;

                    if (best.score < temp.score && temp.distance <= inputData.max_distance && temp.time <= inputData.max_time) {
                        best = std::move(temp);
                    }
                }
            }
        }

        return best; // RVO
    }

    // Время работы O(n^2)
    Solution SwapAdjacent(const Solution &solution, const InputData &inputData) {
        auto path_size = solution.tour.size();
        auto best = solution;

        // лучшая позиция для swap, если такой нет, то останется нулем
        size_t best_i = 0;
        auto best_score = best.score;
        // меняем только внутренние вершины без первой и последней тк это депо
        for (size_t i = 1; i < path_size - 2; ++i) {
            best.tour[i] = std::exchange(best.tour[i + 1], best.tour[i]);
            auto [distance, time, score] = inputData.get_path_time_distance_score(best.tour);
            // возврат пути в прежнее состояние
            best.tour[i] = std::exchange(best.tour[i + 1], best.tour[i]);
            if (best_score < score && time <= inputData.max_time &&  distance <= inputData.max_distance) {
                // в случае если не было улучшений возвращаем все как было
                best_i = i, best_score = score;
            }
        }

        if (best_i) {
            best.tour[best_i] = std::exchange(best.tour[best_i + 1], best.tour[best_i]);
            auto [distance, time, score] = inputData.get_path_time_distance_score(best.tour);
            best.distance = distance, best.time = time, best.score = score;
        }

        return best; // RVO
    }

    // Время работы O(n^3)
    Solution SwapAny(const Solution &solution, const InputData &inputData) {
        const auto path_size = solution.tour.size();
        auto best = solution;

        // аналогично как в SwapAdjacent
        size_t best_i = 0, best_j = 0;
        auto best_score = best.score;
        // меняем только внутренние вершины без первой и последней тк это депо
        for (size_t i = 1; i < path_size - 1; ++i) {
            // чтобы менять только пары вида (i, j): i < j
            for (size_t j = i + 1; j < path_size - 1; ++j) {

                best.tour[i] = std::exchange(best.tour[j], best.tour[i]);
                auto [distance, time, score] = inputData.get_path_time_distance_score(best.tour);
                // возврат к прежнему состоянию пути
                best.tour[i] = std::exchange(best.tour[j], best.tour[i]);
                if (best_score < score && time <= inputData.max_time && distance <= inputData.max_distance) {
                    best_i = i, best_j = j, best_score = score;
                }
            }
        }

        if (best_i && best_j) {
            // возвращаем улучшенный путь если нашли такой
            best.tour[best_i] = std::exchange(best.tour[best_j], best.tour[best_i]);
            auto [distance, time, score] = inputData.get_path_time_distance_score(best.tour);
            best.distance = distance, best.time = time, best.score = score;
        }

        return best; // RVO
    }

    // Время работы O(n^3)
    Solution TwoOpt(const Solution &solution, const InputData &inputData) {
        const auto path_size = solution.tour.size();
        auto best = solution;
        auto best_score = best.score;

        size_t best_i = 0, best_j = 0;
        for (size_t i = 1; i < path_size - 1; ++i) {
            for (size_t j = i + 1; j < path_size - 1; ++j) {

                std::reverse(best.tour.begin() + i, best.tour.begin() + j);
                auto [distance, time, score] = inputData.get_path_time_distance_score(best.tour);
                // возврат пути к исходному состоянию
                std::reverse(best.tour.begin() + i, best.tour.begin() + j);

                if (best_score < score && time <= inputData.max_time && distance <= inputData.max_distance) {
                    best_i = i, best_j = j, best_score = score;
                };
            }
        }

        if (best_i && best_j) {
            std::reverse(best.tour.begin() + best_i, best.tour.begin() + best_j);
            auto [distance, time, score] = inputData.get_path_time_distance_score(best.tour);
            best.distance = distance, best.time = time, best.score = score;
        }
        return best; // RVO
    }

    Solution OrOpt(const Solution &solution, const InputData &inputData, size_t opt_size) {
        const auto path_size = solution.tour.size();
        auto temp = solution;
        auto best_score = solution.score;
        size_t best_i = 0, best_j = 0;

        if (path_size < 4) {
            return solution;
        }

        opt_size = std::min(opt_size, path_size - 2);

        using vertexType = decltype(solution.tour)::value_type;

        for (size_t i = 1; i + opt_size < path_size; ++i) {
            temp = solution;
            std::vector<vertexType> swap_elements(temp.tour.begin() + i, 
                                                   temp.tour.begin() + i + opt_size);
            temp.tour.erase(temp.tour.begin() + i, temp.tour.begin() + i + opt_size);
            
            for (size_t j = 1; j < temp.tour.size(); ++j) {
                temp.tour.insert(temp.tour.begin() + j, swap_elements.begin(), swap_elements.end());
                auto [distance, time, score] = inputData.get_path_time_distance_score(temp.tour);
                // возврат к старому положению
                temp.tour.erase(temp.tour.begin() + j, temp.tour.begin() + j + opt_size);
                if (best_score < score && time <= inputData.max_time && distance <= inputData.max_distance) {
                    best_i = i, best_j = j, best_score = score;
                };
            }
        }

        temp = solution;
        if (best_i && best_j) {
            std::vector<vertexType> swap_elements(temp.tour.begin() + best_i, 
                                                  temp.tour.begin() + best_i + opt_size);
            temp.tour.erase(temp.tour.begin() + best_i, temp.tour.begin() + best_i + opt_size);
            temp.tour.insert(temp.tour.begin() + best_j, swap_elements.begin(), swap_elements.end());

            auto [distance, time, score] = inputData.get_path_time_distance_score(temp.tour);
            temp.distance = distance, temp.time = time, temp.score = score;
        }

        return temp; // RVO
    }

    Solution DoubleBridge(const Solution &solution) {
        auto path_size = solution.tour.size();

        if (path_size < 8) {
            // слишком маленький маршрут
            return solution;
        }

        std::uniform_int_distribution<size_t> dist_a(1, path_size / 4);
        std::uniform_int_distribution<size_t> dist_b(1, path_size / 4);
        std::uniform_int_distribution<size_t> dist_c(1, path_size / 4);
        
        auto a = dist_a(rng_gen);
        auto b = a + 1 + dist_b(rng_gen);
        auto c = b + 1 + dist_c(rng_gen);
        
        // ограничиваем значения, чтобы не выйти за границы
        if (c >= path_size - 1) {
            c = path_size - 2;
        }
        if (b >= c) {
            b = c - 1;
        }
        if (a >= b) {
            a = b - 1;
        }

        Solution temp(0);
        temp.tour.reserve(path_size);

        // сегмент 1 (начальное депо остается на своем месте)
        temp.tour.insert(temp.tour.end(), solution.tour.begin(), solution.tour.begin() + a);
        // сегмент 3
        temp.tour.insert(temp.tour.end(), solution.tour.begin() + b, solution.tour.begin() + c);
        // сегмент 2
        temp.tour.insert(temp.tour.end(), solution.tour.begin() + a, solution.tour.begin() + b);
        // сегмент 4 (конечное депо остается на своем месте)
        temp.tour.insert(temp.tour.end(), solution.tour.begin() + c, solution.tour.end());

        return temp; // RVO
    }
}

Solution VND(Solution solution, int maxLevel, const InputData &inputData) {

    auto best = solution;
    auto current = solution;

    int level = 1;

    do {
        Solution temp(0);
        temp.tour.reserve(solution.tour.size());

        OptimizationType levelType = OptimizationType(level);
        switch (levelType) {
            case OptimizationType::Shift: {
                temp = Shift(current, inputData);
                break;
            }
            case OptimizationType::SwapAdjacent: {
                temp = SwapAdjacent(current, inputData);
                break;
            }
            case OptimizationType::SwapAny: {
                temp = SwapAny(current, inputData);
                break;
            }
            case OptimizationType::TwoOpt: {
                temp = TwoOpt(current, inputData);
                break;
            }
            default:
            case OptimizationType::OrOpt: {
                size_t opt_size = 3 + (level > 5 ? level - 5 : 0);
                temp = OrOpt(current, inputData, opt_size);
                break;
            }
        }

        auto [distance, time, score] = inputData.get_path_time_distance_score(temp.tour);
        temp.time = time, temp.distance = distance, temp.score = score;

        if (temp.score > current.score && temp.distance <= inputData.max_distance) {
            current = std::move(temp);
            level = 1;
            if (current.score > best.score) {
                best = current;
            }
        } else {
            level++;
        }
    } while (level < maxLevel);

    return best;
}

Solution Perturbation(const Solution &solution, int maxLevel, double p, const InputData &inputData) {
    Solution best = solution;

    for (int k = 0; k <= maxLevel; ++k) {

        auto temp = DoubleBridge(best);
        auto [distance, time, score] = inputData.get_path_time_distance_score(temp.tour);
        temp.time = time, temp.distance = distance, temp.score = score;

        OptimizationType levelType = OptimizationType(k);
        switch (levelType) {
            case OptimizationType::Shift: {
                temp = Shift(temp, inputData);
                break;
            }
            case OptimizationType::SwapAdjacent: {
                temp = SwapAdjacent(temp, inputData);
                break;
            }
            case OptimizationType::SwapAny: {
                temp = SwapAny(temp, inputData);
                break;
            }
            case OptimizationType::TwoOpt: {
                temp = TwoOpt(temp, inputData);
                break;
            }
            default:
            case OptimizationType::OrOpt: {
                size_t opt_size = 3 + (k > 5 ? k - 5 : 0);
                temp = OrOpt(temp, inputData, opt_size);
                break;
            }
        }

        if (temp.time * (1.0 - p) < best.time && temp.distance <= inputData.max_distance) {
            // допускаем небольшое ухудшение времени
            best = std::move(temp);
            break;
        }
    }

    return best; // RVO
}