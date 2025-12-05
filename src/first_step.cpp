#include "../include/first_step.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <bitset>

constexpr size_t TOP_SOLUTIONS_COUNT = 3;

namespace {
    using score_type = FirstStepAnswer::score_type;
    using points_type = FirstStepAnswer::points_type;

    struct Candidate {
        static constexpr size_t MAX_POINTS = std::numeric_limits<points_type>::max();

        // посещения в пути
        std::bitset<MAX_POINTS> visited;

        // информация о метриках
        score_type value;
        score_type time;
        score_type distance;

        // для восстановления из dp[cur_load][last_vertex][candidate_idx]
        size_t candidate_idx;
        points_type last_vertex;
        size_t load;

        static Candidate CraeteCandidateFromTour(
            const FirstStepAnswer solution,
            size_t candidate_idx,
            size_t cur_load
        ) {
            Candidate candidate;

            for (auto v : solution.vertexes) {
                candidate.visited.set(v);
            }

            candidate.candidate_idx = candidate_idx;
            candidate.last_vertex = solution.vertexes.back();

            candidate.load = cur_load;

            return candidate; // RVO
        }

        FirstStepAnswer CreateTourFromCandidate(
            const std::vector<std::vector<std::vector<FirstStepAnswer>>>& dp
        ) const {
            FirstStepAnswer answer;
            answer.vertexes = dp[load][last_vertex][candidate_idx].vertexes;

            answer.value = value;
            answer.distance = distance;
            answer.time = time;

            return answer; // RVO
        }
    };

    inline bool isCandidateGood(const std::vector<Candidate>& candidates, score_type value) {
        if (candidates.empty() || candidates.size() < TOP_SOLUTIONS_COUNT) [[unlikely]] {
            return true;
        }

        return value > candidates.back().value;
    }
    
    inline void insertTopCandidate(std::vector<Candidate>& candidates, Candidate&& newCandidate) {

        bool duplicated = false;
        for (auto& existed_candidate : candidates) {
            if (existed_candidate.visited == newCandidate.visited) {
                if (existed_candidate.value < newCandidate.value) {
                    std::swap(existed_candidate, newCandidate);
                } else {
                    // уже есть такой путь и у него score лучше
                    return;
                }
                duplicated = true;
                break;
            }
        }

        if (!duplicated) {
            candidates.emplace_back(std::move(newCandidate));
        }

        std::sort(candidates.begin(), candidates.end(), 
            [](auto& first, auto& second) {
                return first.value > second.value;
            }    
        );

        if (candidates.size() == TOP_SOLUTIONS_COUNT + 1) [[likely]] {
            candidates.resize(TOP_SOLUTIONS_COUNT);
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


    std::vector<Candidate> candidates;
    candidates.reserve(TOP_SOLUTIONS_COUNT);

    for (points_type cur_load = 0; cur_load <= max_load; ++cur_load) {
        bool find_update_point = false;
    
        for (points_type j = 0; j < points_count; ++j) {

            candidates.clear();

            for (points_type i = 0; i < points_count; ++i) {

                if (i == 0 && cur_load != 0) [[unlikely]] {
                    // хотим искать пути из начала только если загрузка 0
                    continue;
                }

                // делаем проверку:
                // 1. вершины не совпали   
                // 2. есть такой путь с cur_load вершинами, который заканчивается в i
                if (i != j && !dp[cur_load][i].empty()) {
                    // перебираем все решения из dp[cur_load][i]
                    for (size_t candidate_idx = 0; candidate_idx < dp[cur_load][i].size(); ++candidate_idx) {
                        const auto& prev_solution = dp[cur_load][i][candidate_idx];

                        // 3. вершина j еще не была в пути
                        if (prev_solution.value != FirstStepAnswer::default_value &&
                            !prev_solution.IsVertexInPath(j)) [[likely]] {

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

                                if (isCandidateGood(candidates, new_point_score)) {
                                    auto new_candidate = Candidate::CraeteCandidateFromTour(prev_solution, candidate_idx, cur_load);
                                    new_candidate.time = new_point_time;
                                    new_candidate.distance = new_point_dist;
                                    new_candidate.value = new_point_score;
                                    insertTopCandidate(candidates, std::move(new_candidate));
                                }
                            }
                        }
                    }
                }
            }

            if (!candidates.empty()) [[likely]] {
                // восстанавливаем пути по кандидатам
                std::vector<FirstStepAnswer> solutions;
                solutions.reserve(candidates.size());
                for (const auto& candidate : candidates) {
                    solutions.emplace_back(candidate.CreateTourFromCandidate(dp));
                    // добавляем текущую вешинку 
                    // upd. там уже хранятся корректные метрики в кандидате
                    solutions.back().vertexes.emplace_back(j);
                }
                dp[cur_load + 1][j] = std::move(solutions);
                find_update_point = true;
            }
        }

        if (!find_update_point) {
            // выходим если не смогли обновить ни один из путей для cur_load + 1
            break;
        }
    }

    // собираем все лучшие решения из всех допустимых состояний
    std::vector<Candidate> answer_candidates;
    answer_candidates.reserve(TOP_SOLUTIONS_COUNT);

    for (points_type cur_load = min_load + 1; cur_load <= max_load + 1; ++cur_load) {
        for (size_t candidate_idx = 0; candidate_idx < dp[cur_load][0].size(); ++candidate_idx) {
            const auto& solution = dp[cur_load][0][candidate_idx];

            if (isCandidateGood(answer_candidates, solution.value)) {
                auto answer_candidate =  Candidate::CraeteCandidateFromTour(solution, candidate_idx, cur_load);
                answer_candidate.time = solution.time;
                answer_candidate.distance = solution.distance;
                answer_candidate.value = solution.value;
                insertTopCandidate(answer_candidates, std::move(answer_candidate));
            }
        }
    }

    std::vector<FirstStepAnswer> answer_solutions;
    answer_solutions.reserve(answer_candidates.size());

    // восстанавливаем лучшие решения из кандидатов
    for (const auto& candidate: answer_candidates) {
        answer_solutions.emplace_back(candidate.CreateTourFromCandidate(dp));
    }
    return answer_solutions;
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