#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct BestFirstSearchOptionalParameters
{
    NodeId maximum_size_of_the_solution_pool = 1;

    NodeId maximum_number_of_nodes = -1;

    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct BestFirstSearchOutput
{
    BestFirstSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    SolutionPool<BranchingScheme> solution_pool;

    Counter number_of_nodes = 0;
};

template <typename BranchingScheme>
inline BestFirstSearchOutput<BranchingScheme> best_first_search(
        const BranchingScheme& branching_scheme,
        BestFirstSearchOptionalParameters parameters = {})
{
    // Initial display.
    parameters.info.os()
            << "======================================" << std::endl
            << "          Tree Search Solver          " << std::endl
            << "======================================" << std::endl
            << std::endl
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Best First Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of nodes:     " << parameters.maximum_number_of_nodes << std::endl
            << "Maximum size of the pool:    " << parameters.maximum_size_of_the_solution_pool << std::endl
            << "Time limit:                  " << parameters.info.time_limit << std::endl
            << std::endl;

    BestFirstSearchOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);
    output.solution_pool.display_init(parameters.info);
    auto node_hasher = branching_scheme.node_hasher();
    NodeMap<BranchingScheme> history{0, node_hasher, node_hasher};
    NodeSet<BranchingScheme> q(branching_scheme);

    auto node_cur = branching_scheme.root();

    while (node_cur != nullptr || !q.empty()) {
        output.number_of_nodes++;

        // Check time.
        if (parameters.info.needs_to_end())
            break;

        // Check node limit.
        if (parameters.maximum_number_of_nodes != -1
                && output.number_of_nodes > parameters.maximum_number_of_nodes)
            break;

        // Get node from the queue.
        if (node_cur == nullptr) {
            node_cur = *q.begin();
            q.erase(q.begin());
        }

        // Bound.
        if (branching_scheme.bound(node_cur, output.solution_pool.worst())) {
            node_cur = nullptr;
            continue;
        }

        // Get next child.
        auto child = branching_scheme.next_child(node_cur);
        // Bound.
        if (child != nullptr) {
            // Update best solution.
            if (branching_scheme.better(child, output.solution_pool.worst())) {
                std::stringstream ss;
                ss << "node " << output.number_of_nodes;
                output.solution_pool.add(child, ss, parameters.info);
                output.solution_pool.display(ss, parameters.info);
            }
            // Add child to the queue.
            if (!branching_scheme.leaf(child)
                    && !branching_scheme.bound(child, output.solution_pool.worst()))
                add_to_history_and_queue(branching_scheme, history, q, child);
        }

        // If node_cur still has children, put it back to the queue.
        if (branching_scheme.infertile(node_cur)) {
            node_cur = nullptr;
        } else if ((Counter)q.size() != 0
                && branching_scheme(*(q.begin()), node_cur)) {
            q.insert(node_cur);
            node_cur = nullptr;
        }

    }

    output.solution_pool.display_end(parameters.info);
    parameters.info.os() << "Number of nodes:            " << output.number_of_nodes << std::endl;
    parameters.info.add_to_json("Algorithm", "NumberOfNodes", output.number_of_nodes);
    return output;
}

}

