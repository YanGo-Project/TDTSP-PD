#include "../include/init_population.hpp"

#ifdef DEBUG
#include "../utils/debug.h"
#endif

void PopulationInitializer::initialize_population(int path_size, int population_size, double beta, int alpha,
                                                  const InputData &inputData, std::vector<Solution> &population) {

    size_t total_iterations = 0;
    size_t random_iterations = 0;
    size_t grasp_iterations = 0;
    while (population.size() < population_size) {

        if (total_iterations == max_iterations) {
            return;
        }

        Solution local_solution(0);
        if (generate_random_double() <= beta) {
#ifdef DEBUG
            ++random_iterations;
#endif
            random_tour(local_solution, path_size, inputData);
        } else {
#ifdef DEBUG
            ++grasp_iterations;
#endif
            grasp_tour(local_solution, path_size, alpha, inputData);
        }
        ++total_iterations;

        if (local_solution.time <= inputData.max_time && local_solution.distance <= inputData.max_distance) {
            // Проверяем, что такого решения еще нет в популяции
            if (std::any_of(population.cbegin(), population.cend(),
                             [&local_solution](const auto &existed_solution) { return existed_solution == local_solution; })) {
#ifdef DEBUG
                std::cout << "Can`t add dublicate tour to population\n";
#endif
                continue;
            }
#ifdef DEBUG
            std::cout << "Added tour to population:\n";
            std::cout << local_solution << std::endl;
#endif
            population.emplace_back(std::move(local_solution));
        }
    }

#ifdef DEBUG
    std::cout << "Generated " << population.size() << " tours\n";
    std::cout << "Random iterations: " << random_iterations << "\nGrasp iterations: " << grasp_iterations << "\n";
    std::cout << "Total iterations: " << total_iterations << std::endl;
#endif
}

void PopulationInitializer::random_tour(Solution &solution, int n, const InputData &input) {
    solution.tour.clear();
    solution.tour.reserve(n);
    // добавляем все кроме депо
    for (int i = 1; i < n - 1; i++) {
        solution.tour.push_back(i);
    }

    std::shuffle(solution.tour.begin(), solution.tour.end(), rng);
    // добавление начального и конечного депо
    solution.tour.insert(solution.tour.begin(), 0);
    solution.tour.push_back(0);

    auto [distance, time, score] = input.get_path_time_distance_score(solution.tour);
    solution.distance = distance, solution.time = time, solution.score = score;
}

void PopulationInitializer::grasp_tour(Solution &solution, int n, int alpha, const InputData &input) {
    std::vector<bool> visited(n - 1, false);
    visited[0] = true;

    solution.tour.resize(n);
    solution.tour[0] = 0;
    solution.time = 0;
    solution.distance = 0;
    solution.score = 0;

    std::vector<std::pair<int64_t, decltype(solution.tour)::value_type >> candidates;
    candidates.reserve(n);

    // проходимся по всем кроме депо
    for (int i = 1; i < n - 1; i++) {
        auto last_vertex = solution.tour[i - 1];

        candidates.clear();
        // не берем в учет депо
        for (uint32_t j = 1; j < n - 1; j++) {
            if (!visited[j]) {
                auto time = input.get_time_dependent_cost(solution.time, last_vertex, j);
                candidates.emplace_back(time, j);
            }
        }

        sort(candidates.begin(), candidates.end());
        int rcl_size = std::min(alpha, static_cast<int>(candidates.size()));
        // потому что мы не можем выбирать депо пока есть еще непосещенные вершины
        int next_vertex = candidates[ rcl_size == 1 ? 0 : generate_random_int(0, rcl_size - 1)].second;

        solution.distance += input.distance_matrix[last_vertex][next_vertex];
        solution.time += input.get_time_dependent_cost(solution.time, last_vertex, next_vertex);
        solution.tour[i] = next_vertex;

        visited[next_vertex] = true;
    }

    // добавление конечного депо
    solution.tour[n - 1] = 0;
    auto [distance, time, score] = input.get_path_time_distance_score(solution.tour);
    solution.distance = distance, solution.time = time, solution.score = score;
}

double PopulationInitializer::generate_random_double() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

int PopulationInitializer::generate_random_int(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}