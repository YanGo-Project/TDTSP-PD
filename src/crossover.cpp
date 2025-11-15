#include "../include/crossover.hpp"

#include <algorithm>
#include <stdexcept>
#include <map>
#include <set>
#include <unordered_set>

#ifdef DEBUG
#include "../utils/debug.h"
#endif

namespace {
    using verteIdxType = decltype(Solution::tour)::value_type;

    verteIdxType take_random_from_set(const std::unordered_set<verteIdxType>& candidates, std::mt19937& rng) {
        auto idx = std::uniform_int_distribution<int>(0, candidates.size() - 1)(rng);
        size_t i = 0;
        for (auto candidate: candidates) {
            if (i == idx) {
                return candidate;
            }
            ++i;
        }

        throw std::runtime_error("unreachable code");
    }
}

Solution Crossover::EXX(const Solution &first, const Solution &second) {
    size_t path_size = first.tour.size();

    using vertexIdxType = decltype(first.tour)::value_type;
    std::map<vertexIdxType, std::set<vertexIdxType>> mapping;

    auto answer = Solution(0);
    answer.tour.reserve(first.tour.size());

    auto was_not_vertexes = std::unordered_set<vertexIdxType>{};
    // path_size - 2 чтобы не добавлять 0 (депо) как кандидата
    for (size_t i = 0; i < path_size - 2; ++i) {
        was_not_vertexes.insert(i+1);
        mapping[first.tour[i]].insert(first.tour[i + 1]);
        mapping[second.tour[i]].insert(second.tour[i + 1]);
    }

    answer.tour.reserve(path_size);
    answer.tour.push_back(0);

    auto candidates = std::vector<vertexIdxType>{};
    candidates.reserve(2);

    for (size_t i = 1; i < path_size - 1; ++i) {
        vertexIdxType candidate;
        if (mapping[answer.tour[i - 1]].size() == 1) {
            candidate = *mapping[answer.tour[i - 1]].cbegin();
            // в случае если кандидат уже был в пути
            if (was_not_vertexes.find(candidate) == was_not_vertexes.end()) {
                candidate = take_random_from_set(was_not_vertexes, rng);
            }
        } else {
            candidates.clear();
            for (auto idx : mapping[answer.tour[i - 1]]) {
                if (was_not_vertexes.find(idx) != was_not_vertexes.end()) {
                    candidates.push_back(idx);
                }
            }
            if (candidates.size() == 0) {
                // в случае если нет доступных канидатов
                candidate = take_random_from_set(was_not_vertexes, rng);
            } else {
                auto idx = std::uniform_int_distribution<int>(0, candidates.size() - 1)(rng);
                candidate = candidates[idx];
            }
        }
        answer.tour.push_back(candidate);
        was_not_vertexes.erase(candidate);
    }
    answer.tour.push_back(0);

    return answer; // RVO
}

// https://en.wikipedia.org/wiki/Crossover_(evolutionary_algorithm)#Partially_mapped_crossover_(PMX)
Solution Crossover::PMX(const Solution &first, const Solution &second) {

    auto path_size = first.tour.size();
    auto answer = first;

    using verteIdxType = decltype(first.tour)::value_type;

    if (path_size <= 4)[[unlikely]] {
        return answer; // RVO
    }

    int start = std::uniform_int_distribution<int>(1, path_size - 3)(rng);
    int end = std::uniform_int_distribution<int>(start + 1, path_size - 1)(rng);

    std::map<verteIdxType, verteIdxType> first2second;

    for (size_t i = start; i <= end; ++i) {
        first2second[first.tour[i]] = second.tour[i];
    }

    for (size_t i = 1; i < start; ++i) {
        auto candidate = second.tour[i];
        while(first2second.find(candidate) != first2second.end()) {
            candidate = first2second.at(candidate);
        }
        answer.tour[i] = candidate;
    }

    for (size_t i = end + 1; i < path_size - 1; ++i) {
        auto candidate = second.tour[i];
        while(first2second.find(candidate) != first2second.end()) {
            candidate = first2second.at(candidate);
        }
        answer.tour[i] = candidate;
    }

    return answer; // RVO
}

Solution Crossover::SC(const Solution &first, const Solution &second) {

    size_t path_size = first.tour.size();
    std::vector<std::pair<decltype(first.tour)::value_type, int>> idx(path_size - 1);

    // считаем сумму позиций вершин для всех кроме депо
    for (size_t i = 1; i < path_size - 1; ++i) {
        idx[first.tour[i]].first = first.tour[i];
        idx[first.tour[i]].second += static_cast<int>(i);

        idx[second.tour[i]].first = second.tour[i];
        idx[second.tour[i]].second += static_cast<int>(i);
    }

    // сдвинули начало на 1 чтобы не учитывать индекс депо - 0
    std::sort(idx.begin() + 1, idx.end(),
              [](const auto &a, const auto &b) { return a.second < b.second; });

    Solution answer(path_size);

    // депо всегда первое
    answer.tour[0] = 0;
    for (size_t i = 1; i < path_size - 1; ++i) {
        answer.tour[i] = idx[i].first;
    }
    // депо всегда завершает маршрут
    answer.tour[path_size - 1] = 0;

    return answer; // RVO
}

Solution Crossover::crossover(const Solution &first, const Solution &second, const InputData &inputData) {

#ifdef DEBUG
    std::cout << "Crossover\nFirst tour:\n";
    std::cout << first << std::endl;
    std::cout << "---------------------------------\nSecond tour:\n";
    std::cout << second << std::endl;
#endif

    if (first.tour.size() != second.tour.size()) [[unlikely]] {
        throw std::runtime_error("Different tours sizes");
    }

    Solution result(0);
    auto type = std::uniform_int_distribution<int>(0,2)(rng);
    switch (type) {
        case 0: {
            result = EXX(first, second);
            break;
        }
        case 1: {
            result = PMX(first, second);
            break;
        }
        default: {
            result = SC(first, second);
            break;
        }
    }

    auto [distance, time, score] = inputData.get_path_time_distance_score(result.tour);
    result.time = time, result.distance = distance, result.score = score;

#ifdef DEBUG
    std::cout << "Tour after ";
    switch (type) {
        case 0: {
            std::cout << "EXX";
            break;
        }
        case 1: {
            std::cout << "PMX";
            break;
        }
        default: {
            std::cout << "SC";
            break;
        }
    }
    std::cout << ":\n";
    std::cout << result << std::endl;

    // проверка размера маршрута
    if (result.tour.size() != first.tour.size()) {
        throw std::runtime_error("Crossover produced invalid tour size");
    }
    
    // проверка депо в начале и конце маршрута
    if (result.tour.front() != 0 || result.tour.back() != 0) {
        throw std::runtime_error("Crossover produced invalid tour: depot not at start/end");
    }
    
    // проверка наличия всех вершин (кроме депо)
    std::unordered_set<decltype(first.tour)::value_type> vertices_in_result;
    for (size_t i = 1; i < result.tour.size() - 1; ++i) {
        if (vertices_in_result.find(result.tour[i]) != vertices_in_result.end()) { 
            // нашли дубликат вершины
            std::cout << result.tour[i] << std::endl;
            throw std::runtime_error("Crossover produced invalid tour: duplicate vertices");
        }
        vertices_in_result.insert(result.tour[i]);
    }
    
    // проверка что набор вершин тот же что и в родительском маршруте
    std::unordered_set<decltype(first.tour)::value_type> vertices_in_first;
    for (size_t i = 1; i < first.tour.size() - 1; ++i) {
        vertices_in_first.insert(first.tour[i]);
    }
    
    if (vertices_in_result != vertices_in_first) {
        throw std::runtime_error("Crossover produced invalid tour: missing or extra vertices");
    }
#endif
    
    return result; // RVO
}