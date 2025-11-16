#include "utils/json_parser.hpp"

#include "include/first_step.hpp"
#include "include/algorithm.hpp"

#ifdef DEBUG
#include "utils/debug.h"
#endif

#include <iostream>
#include <fstream>

using points_type = FirstStepAnswer::points_type;

Solution Solve(InputData &&input, const ProgramArguments& args) {

    auto firstStepAnswer = DoFirstStep<true>(input);
    
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

    MetaParameters params {
        .population_size = 10,
        .alpha = 25,
        .beta = 0.5,
        .nloop = 10,
        .kMax = 5,
        .p = 0.05,
        .max_iter_without_solution = 15,
        .max_crossover_candidates = 4,
    };

    auto answer = applyTspTDPDP(std::move(solution), input, params);
    
    for (size_t i = 0; i < answer.tour.size(); ++i) {
        answer.tour[i] = input.from_new_to_old[answer.tour[i]];
    }

    if (args.save_csv) {
        std::ofstream csv(args.csv_file, std::ios::app);
        csv << args.problemJsonPath << "," << firstStepAnswer.get_data_to_csv() << "," << answer.get_data_to_csv() << std::endl;
    }

    return answer;
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

    auto answer = Solve(std::move(input), args);

    return 0;
}