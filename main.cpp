#include "utils/json_parser.hpp"

#include "include/first_step.hpp"
#include "include/algorithm.hpp"

#ifdef DEBUG
#include "utils/debug.h"
#endif

#include <iostream>

using points_type = FirstStepAnswer::points_type;

Solution Solve(InputData &&input, uint64_t time_limit) {

    auto firstStepAnswer = DoFirstStep<true>(input);

    std::cout << "First stpen answer:\n" << firstStepAnswer << std::endl;

    // Пересчитываем путь до маппинга, используя исходные индексы
    auto [distance, time, score] = input.get_path_time_distance_score(firstStepAnswer.vertexes);
    
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
    
    Solution solution(std::move(tour), distance, time, score);

    MetaParameters params {
        .population_size = 8,
        .alpha = 5,
        .beta = 0.5,
        .nloop = 7,
        .kMax = 4,
        .p = 0.05,
        .max_iter_without_solution = 15,
        .max_crossover_candidates = 4,
    };

    auto answer = applyTspTDPDP(std::move(solution), input, params);
    auto [distance1, time1, score1] = input.get_path_time_distance_score(answer.tour);
    answer.distance = distance1, answer.time = time1, answer.score = score1;
    
    for (size_t i = 0; i < answer.tour.size(); ++i) {
        answer.tour[i] = input.from_new_to_old[answer.tour[i]];
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

    auto answer = Solve(std::move(input), args.time);

    std::cout << answer << std::endl;

    return 0;
}