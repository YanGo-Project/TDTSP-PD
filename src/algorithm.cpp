#include "../include/algorithm.hpp"

#include "../include/init_population.hpp"
#include "../include/vns.hpp"
#include "../include/crossover.hpp"

#include <random>
#include <algorithm>
#include <chrono>

#ifdef DEBUG
#include "../utils/debug.h"
#endif

namespace {
    bool is_time_limit(decltype(std::chrono::steady_clock::now()) start, uint64_t max_time) {
        auto current = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            current - start
        ).count();

        return elapsed > max_time;
    }

#ifdef SAVE_STEPS
    uint64_t get_elapsed_seconds(decltype(std::chrono::steady_clock::now()) start) {
        auto current = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(
            current - start
        ).count();
    }

    void log_best_solution_if_needed(
        decltype(std::chrono::steady_clock::now()) start,
        const Solution& best_solution,
        Context& ctx,
        uint64_t& last_logged_time
    ) {
        uint64_t elapsed = get_elapsed_seconds(start);

        if (elapsed >= last_logged_time + 5) {

            last_logged_time = (elapsed / 5) * 5;
                        
            ctx.time_iterations.push_back(
                {
                    .distance = best_solution.distance,
                    .score = static_cast<uint32_t>(best_solution.score),
                    .time = best_solution.time,
                    .timestamp = last_logged_time
                }
            );
        }
    }
#endif
}

Solution applyTspTDPDP(Solution&& solution, const InputData &inputData, Context& ctx) {

    auto start = std::chrono::steady_clock::now();
    const auto& params = ctx.args.meta;
    auto max_time = ctx.args.time;
#ifdef SAVE_STEPS
    uint64_t last_logged_time = 0;
#endif

    auto populationInitializer = PopulationInitializer();
    auto crossover = Crossover();

    auto path_size = solution.tour.size();

    std::vector<Solution> population;
    population.reserve(params.population_size);
    population.emplace_back(std::move(solution));
    populationInitializer.initialize_population(population[0].tour.size(),
                                                params.population_size,
                                                params.beta,
                                                params.alpha,
                                                inputData,
                                                population);
#ifdef SAVE_STEPS
    std::sort(population.begin(), population.end(), 
              [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });
    log_best_solution_if_needed(start, population[0], ctx, last_logged_time);
#endif

    for (size_t i = 0; i < population.size(); ++i) {
        // проверка что выписываемся в ограничения по времени
        if (is_time_limit(start, max_time)) [[unlikely]] {
            std::sort(population.begin(), population.end(), 
              [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });
              
            return population[0];
        }
#ifdef SAVE_STEPS
    std::sort(population.begin(), population.begin() + i, 
              [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });
    log_best_solution_if_needed(start, population[0], ctx, last_logged_time);
#endif

        population[i] = VNS(population[i], inputData, params.nloop, params.kMax, params.p);
    }
    
#ifdef SAVE_STEPS
    std::sort(population.begin(), population.end(), 
              [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });
    log_best_solution_if_needed(start, population[0], ctx, last_logged_time);
#endif

    // удаляем повторы после оптимизации чтобы на вход кроссовера не шли два одинаковых пути
    // и мы не получали тот же после него
    std::vector<Solution> seen;
    seen.reserve(population.size());
    for (auto& solution: population) {
        if (std::find(seen.begin(), seen.end(), solution) == seen.end()) {
            seen.emplace_back(std::move(solution));
        }
    }
    population = std::move(seen);

#ifdef DEBUG
    std::cout << "Population size after VNS optimization and delete dubplicates:" << population.size() << std::endl;
#endif

    uint64_t iters = 0;
    std::mt19937 rng;
    auto candidates_size = std::min(population.size(), params.max_crossover_candidates);
    int iter_without_solution = 0;

    while (iter_without_solution < params.max_iter_without_solution) {

#ifdef SAVE_STEPS
        std::sort(population.begin(), population.end(), 
              [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });
        log_best_solution_if_needed(start, population[0], ctx, last_logged_time);
#endif

        // проверка что выписываемся в ограничения по времени
        if (is_time_limit(start, max_time)) [[unlikely]] {
            break;
        }

        // среди случайных candidates_size туров выбираем два лучших
        std::shuffle(population.begin(), population.end(), rng);
        std::sort(population.begin(), population.begin() + candidates_size,
                    [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });

        const auto &TC = population[0];
        const auto &TP = population.size() > 1 ? population[1] : population[0];

        auto crossoverSolution = crossover.crossover(TC, TP, inputData);
        crossoverSolution = VNS(crossoverSolution, inputData, params.nloop, params.kMax, params.p);

        if (crossoverSolution.time > inputData.max_time || crossoverSolution.distance > inputData.max_distance) {
#ifdef DEBUG
            std::cout << "Can`t add crossover solution becuase of tour time:  " 
                      << crossoverSolution.time << " vs input max_time: " << inputData.max_time << " "
                      << crossoverSolution.distance << " vs input max_distance "  << inputData.max_distance << std::endl;
#endif
            ++iter_without_solution;
            continue;
        }

        if (std::any_of(population.cbegin(), population.cend(),
                [&crossoverSolution](const auto &existed_solution) { return existed_solution == crossoverSolution; })) {
#ifdef DEBUG
            std::cout << "This solution in population, skip:\n" << crossoverSolution << std::endl;
#endif
            ++iter_without_solution;
            continue;
        }

        std::sort(population.begin(), population.end(),
                    [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });

        // если решение лучше текущего худшего, то заменяем его
        if (crossoverSolution.score > population[population.size() - 1].score) {
#ifdef DEBUG
            std::cout << "Created new solution:\n" << crossoverSolution << std::endl;
#endif
            population[population.size() - 1] = std::move(crossoverSolution);
        }

        // в случае если не улучшили лучшее решение
        if (crossoverSolution.score > population[0].score) {
            iter_without_solution = 0;
#ifdef DEBUG
            std::cout << "Updated best solution\n";
#endif
        } else {
            ++iter_without_solution;
#ifdef DEBUG
            std::cout << "Can`t update best solution for: " << iter_without_solution << std::endl;
#endif
        }
    }

    std::sort(population.begin(), population.end(), 
              [](const auto &sol1, const auto &sol2) { return sol1.score > sol2.score; });
            
    return population[0];
}