#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct LargeNeighborhoodSearchParameters
{
    /** Maximum size of the solution pool. */
    NodeId maximum_size_of_the_solution_pool = 1;

    /** Maximum number of iterations. */
    Counter maximum_number_of_iterations = -1;

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct LargeNeighborhoodSearchOutput
{
    LargeNeighborhoodSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    /** Solution pool. */
    SolutionPool<BranchingScheme> solution_pool;

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

template <typename BranchingScheme>
inline LargeNeighborhoodSearchOutput<BranchingScheme> large_neighborhood_search(
        const BranchingScheme& branching_scheme,
        LargeNeighborhoodSearchParameters parameters = {})
{
    using Node = typename BranchingScheme::Node;

    // Initial display.
    parameters.info.os()
            << "======================================" << std::endl
            << "           TreeSearchSolver           " << std::endl
            << "======================================" << std::endl
            << std::endl
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Large Neighborhood Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum size of the pool:    " << parameters.maximum_size_of_the_solution_pool << std::endl
            << "Time limit:                  " << parameters.info.time_limit << std::endl;

    LargeNeighborhoodSearchOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);


    output.solution_pool.display_end(parameters.info);
    parameters.info.os() << "Number of iterations:       " << output.number_of_iterations << std::endl;
    parameters.info.add_to_json("Algorithm", "NumberOfIterations", output.number_of_iterations);
    return output;
}

}

