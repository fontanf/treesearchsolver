#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct GreedyOptionalParameters
{
    /** Maximum size of the solution pool. */
    NodeId maximum_size_of_the_solution_pool = 1;
    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct GreedyOutput
{
    GreedyOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    /** Solution pool. */
    SolutionPool<BranchingScheme> solution_pool;
    /** Number of nodes. */
    Counter number_of_nodes = 0;
};

template <typename BranchingScheme>
inline GreedyOutput<BranchingScheme> greedy(
        const BranchingScheme& branching_scheme,
        GreedyOptionalParameters parameters = {})
{
    using Node = typename BranchingScheme::Node;

    // Initial display.
    VER(parameters.info,
               "======================================" << std::endl
            << "          Tree Search Solver          " << std::endl
            << "======================================" << std::endl
            << std::endl
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Greedy" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum size of the pool:    " << parameters.maximum_size_of_the_solution_pool << std::endl
            << "Time limit:                  " << parameters.info.time_limit << std::endl;
       );

    GreedyOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);

    auto current_node = branching_scheme.root();
    for (output.number_of_nodes = 1;; ++output.number_of_nodes) {
        std::shared_ptr<Node> best_child = nullptr;
        while (!branching_scheme.infertile(current_node)) {
            if (best_child != nullptr
                    && branching_scheme(best_child, current_node))
                break;
            // Get next child.
            auto child = branching_scheme.next_child(current_node);
            if (child == nullptr)
                continue;
            // Update best solution.
            if (branching_scheme.better(child, output.solution_pool.worst())) {
                std::stringstream ss;
                output.solution_pool.add(child, ss, parameters.info);
            }
            if (branching_scheme.leaf(child))
                continue;
            // Update best child;
            if (best_child == nullptr
                    || branching_scheme(child, best_child))
                best_child = child;
        }
        if (best_child == nullptr)
            break;
        current_node = best_child;
    }

    output.solution_pool.display_end(parameters.info);
    VER(parameters.info, "Number of nodes:            " << output.number_of_nodes << std::endl);
    PUT(parameters.info, "Algorithm", "NumberOfNodes", output.number_of_nodes);
    return output;
}

}

