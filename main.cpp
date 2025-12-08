#include "utils/json_parser.hpp"

#include "include/first_step.hpp"
#include "include/algorithm.hpp"

#ifdef DEBUG
#include "utils/debug.h"
#endif

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <optional>

using points_type = FirstStepAnswer::points_type;

Solution Optimize(const FirstStepAnswer& firstStepAnswer, InputData& input, const ProgramArguments& args) {

    // новый маршрут будет иметь вид 0 -> 1 -> 2 -> ... -> n -> 0
    std::vector<points_type> tour(firstStepAnswer.vertexes.size());
    // отображение из новых вершин в старые индексы
    std::map<points_type, points_type> from_new_to_old;
    tour[0] = 0;
    from_new_to_old[0] = 0;
    for (size_t i = 1; i < firstStepAnswer.vertexes.size() - 1; ++i) {
        from_new_to_old[i] = firstStepAnswer.vertexes[i];
        tour[i] = i;
    }
    tour[firstStepAnswer.vertexes.size() - 1] = 0;

    input.from_new_to_old = std::move(from_new_to_old);
    input.is_mapped = true;
    
    Solution solution(std::move(tour), firstStepAnswer.distance, firstStepAnswer.time, firstStepAnswer.value);

    auto ctx = Context {
        .args = args
    };

    auto answer = applyTspTDPDP(std::move(solution), input, ctx);
    
    for (size_t i = 0; i < answer.tour.size(); ++i) {
        answer.tour[i] = input.from_new_to_old[answer.tour[i]];
    }

    return answer;
}

Solution Solve(InputData &&input, const ProgramArguments& args) {

    std::vector<FirstStepAnswer> firstStepAnswers;

    // нужно чтобы нам bitset был хоть сколько-то гибким
    if (input.points_count < 128) {
        firstStepAnswers = DoFirstStep<128, true>(input);
    } else if (input.points_count < 256) {
        firstStepAnswers = DoFirstStep<256, true>(input);
    } else if (input.points_count < 512) {
        firstStepAnswers = DoFirstStep<512, true>(input);
    } else {
        firstStepAnswers = DoFirstStep<std::numeric_limits<InputData::points_type>::max(), true>(input);
    }
    
    // сохраняем копию input для использования в потоках
    InputData input_copy = std::move(input);

    if (firstStepAnswers.empty()) {
        return {0};
    }

    for (size_t i = 0; i < firstStepAnswers.size(); ++i) {
        std::cout << "Solution #" << i << "\n" << firstStepAnswers[i] << "\n";
    }

    std::vector<std::optional<Solution>> solutions(firstStepAnswers.size());
    std::vector<std::thread> threads;
    threads.reserve(firstStepAnswers.size());

    for (size_t i = 0; i < firstStepAnswers.size(); ++i) {
        threads.emplace_back([&input_copy, &args, &firstStepAnswers, &solutions, i]() {
            // вынуждены создавать копию тк строим отображение mapped (наверное можно выпилить костыль)
            InputData thread_input = input_copy;
            
            Solution solution = Optimize(firstStepAnswers[i], thread_input, args);
            
            solutions[i] = std::move(solution);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (size_t i = 0; i < solutions.size(); ++i) {
        if (solutions[i].has_value()) {
            std::cout << "Solution from thread #" << i << ":\n";
            std::cout << *solutions[i] << "\n";
        }
    }

    Solution* best_solution = nullptr;
    for (auto& sol : solutions) {
        if (sol.has_value()) {
            if (best_solution == nullptr || sol->score > best_solution->score) {
                best_solution = &(*sol);
            }
        }
    }

    if (best_solution == nullptr) {
        return {0};
    }

    if (args.save_csv) [[unlikely]] {
        std::ofstream csv(args.csv_file, std::ios::app);
        csv << args.problemJsonPath << "," << (*best_solution).get_data_to_csv() << "\n";
    }

    std::cout << "\nBest solution (score: " << best_solution->score << "):\n";
    std::cout << *best_solution << "\n";

    return *best_solution;
}

int main(int argc, char *argv[]) {
    ProgramArguments args;
    if (!ParseProgramArguments(argc, argv, args)) {
        return -1;
    }

    InputData input;
    if (!JsonParser::ParseInputDataFromJson(args.problemJsonPath, input)) {
        return -2;
    }

    if (!JsonParser::WriteSolutionTojsonFile(args.solutionJsonPath, Solve(std::move(input), args))) {
        return -3;
    }

    return 0;
}