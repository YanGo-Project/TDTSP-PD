#include "../include/first_step.hpp"

#include <algorithm>
#include <iostream>

namespace {
    constexpr size_t TOP_SOLUTIONS_COUNT = 3;
    constexpr double THRESHOLD = 0.95; 
    
    // вставляем решение в отсортированный список лучших решений, сохраняя только топ N
    void insertTopSolution(std::vector<FirstStepAnswer>& solutions, const FirstStepAnswer& newSolution) {
        if (solutions.empty()) {
            solutions.push_back(newSolution);
            return;
        }

        if (newSolution.value < solutions[0].value * THRESHOLD) {
            return;
        }

        if (std::find(solutions.cbegin(), solutions.cend(), newSolution) != solutions.end()) {
            return;
        }
        
        // находим позицию для вставки (решения отсортированы по убыванию value)
        auto it = std::lower_bound(solutions.begin(), solutions.end(), newSolution,
                                   [](const FirstStepAnswer& a, const FirstStepAnswer& b) {
                                       return a.value > b.value;
                                   });
        
        solutions.insert(it, newSolution);
        
        // оставляем только топ N решений
        if (solutions.size() > TOP_SOLUTIONS_COUNT) {
            solutions.resize(TOP_SOLUTIONS_COUNT);
        }
    }
}

bool FirstStepAnswer::IsVertexInPath(const FirstStepAnswer::points_type id) const {
    if (id == 0) {
        return false;
    }
    return std::any_of(vertexes.begin(), vertexes.end(),
                       [&](auto v_idx) { return id == v_idx; });
}

void FirstStepAnswer::AddVertex(FirstStepAnswer::points_type vertex) {
    vertexes.push_back(vertex);
}

template<bool is_time_dependent>
std::vector<FirstStepAnswer> DoFirstStep(const InputData &input) {

    using score_type = FirstStepAnswer::score_type;
    using points_type = FirstStepAnswer::points_type;

    auto max_dist = input.max_distance;
    auto max_time = input.max_time;
    auto max_load = input.max_load;
    auto min_load = input.min_load;
    auto points_count = input.points_count;

    // dp теперь хранит векторы лучших решений для каждого состояния
    std::vector<std::vector<std::vector<FirstStepAnswer>>> dp(
        input.max_load + 2,
        std::vector<std::vector<FirstStepAnswer>>(points_count)
    );

    // инициализация начального состояния
    FirstStepAnswer initial;
    initial.value = 0;
    initial.time = 0;
    initial.distance = 0;
    initial.vertexes = {0};
    dp[0][0].push_back(initial);

    for (points_type cur_load = 0; cur_load <= max_load; ++cur_load) {
        bool find_update_point = false;

        std::cout << "Cur load: " << cur_load << " of: " << max_load << std::endl;

        for (points_type j = 0; j < points_count; ++j) {

            std::cout << "Iter for j:" << j << " of " << points_count << std::endl;

            std::vector<FirstStepAnswer> candidates;

            for (points_type i = 0; i < points_count; ++i) {

                if (i == 0 && cur_load != 0) {
                    // хотим искать пути из начала только если загрузка 0
                    continue;
                }

                // делаем проверку:
                // 1. самокаты не совпали   
                // 2. есть такой путь с cur_load самокатами, который заканчивается в i
                if (i != j && !dp[cur_load][i].empty()) {
                    // перебираем все решения из dp[cur_load][i]
                    for (const auto& prev_solution : dp[cur_load][i]) {
                        // 3. вершина j еще не была в пути
                        if (prev_solution.value != FirstStepAnswer::default_value &&
                            !prev_solution.IsVertexInPath(j)) {

                            FirstStepAnswer::score_type travel_time;
                            if constexpr (is_time_dependent) {
                                travel_time = input.get_time_dependent_cost(prev_solution.time, i, j);
                            } else {
                                travel_time = input.time_matrix[0][i][j];
                            }

                            auto new_point_score = prev_solution.value + (j == 0 ? 0 : input.point_scores[j - 1]) -
                                                   travel_time;
                            auto new_point_time = prev_solution.time + (j == 0 ? 0 : input.point_service_times[j - 1]) +
                                                  travel_time;
                            auto new_point_dist = prev_solution.distance + input.distance_matrix[i][j];

                            // проверки найденного пути на целевую функцию, максимальное время пути и максимальную дистанцию
                            if (new_point_time <= max_time &&
                                new_point_dist <= max_dist) {

                                FirstStepAnswer new_solution = prev_solution;
                                new_solution.time = new_point_time;
                                new_solution.distance = new_point_dist;
                                new_solution.value = new_point_score;
                                new_solution.AddVertex(j);

                                insertTopSolution(candidates, new_solution);
                            }
                        }
                    }
                }
            }

            if (!candidates.empty()) [[likely]] {
                dp[cur_load + 1][j] = std::move(candidates);
                find_update_point = true;
            }
        }

        if (!find_update_point) {
            // выходим если не смогли обновить ни один из путей для cur_load + 1
            break;
        }
    }

    // собираем все лучшие решения из всех состояний
    std::vector<FirstStepAnswer> all_solutions;

    // +1 потому что у нас ячейка при dp[cur_load][0] заполнялась когда в пути было cur_load - 1
    for (points_type cur_load = min_load + 1; cur_load <= max_load + 1; ++cur_load) {
        for (const auto& solution : dp[cur_load][0]) {
            if (solution.value != FirstStepAnswer::default_value) {
                insertTopSolution(all_solutions, solution);
            }
        }
    }

    return all_solutions;
}

std::ostream &operator<<(std::ostream &os, const FirstStepAnswer &answer) {
    os << "Solution score value: " << answer.value << std::endl;
    os << "Solution path:\n";
    for (auto id: answer.vertexes) {
        os << id << " ";
    }
    os << "\nSolution time: " << answer.time << "\nSolution distance: " << answer.distance << "\nSolution size: "
       << answer.vertexes.size();
    os << std::endl;

    return os;
}

template std::vector<FirstStepAnswer> DoFirstStep<true>(const InputData &input);

template std::vector<FirstStepAnswer> DoFirstStep<false>(const InputData &input);