#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct IterativeMemoryBoundedBestFirstSearchOptionalParameters
{
    NodeId maximum_size_of_the_solution_pool = 1;
    NodeId maximum_number_of_nodes = -1;
    double growth_factor = 1.5;
    NodeId minimum_size_of_the_queue = 0;
    NodeId maximum_size_of_the_queue = 100000000;

    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct IterativeMemoryBoundedBestFirstSearchOutput
{
    IterativeMemoryBoundedBestFirstSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    SolutionPool<BranchingScheme> solution_pool;

    Counter number_of_nodes = 0;
    NodeId maximum_size_of_the_queue = 0;
};

template <typename BranchingScheme>
inline IterativeMemoryBoundedBestFirstSearchOutput<BranchingScheme> iterative_memory_bounded_best_first_search(
        const BranchingScheme& branching_scheme,
        IterativeMemoryBoundedBestFirstSearchOptionalParameters parameters = {})
{
    // Initial display.
    parameters.info.os()
            << "======================================" << std::endl
            << "          Tree Search Solver          " << std::endl
            << "======================================" << std::endl
            << std::endl
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Iterative Memory Bounded Best First Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Minimum size of the queue:   " << parameters.minimum_size_of_the_queue << std::endl
            << "Maximum size of the queue:   " << parameters.maximum_size_of_the_queue << std::endl
            << "Maximum number of nodes:     " << parameters.maximum_number_of_nodes << std::endl
            << "Growth factor:               " << parameters.growth_factor << std::endl
            << "Maximum size of the pool:    " << parameters.maximum_size_of_the_solution_pool << std::endl
            << "Time limit:                  " << parameters.info.time_limit << std::endl
            << std::endl;

    IterativeMemoryBoundedBestFirstSearchOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);
    output.solution_pool.display_init(parameters.info);
    auto node_hasher = branching_scheme.node_hasher();
    NodeSet<BranchingScheme> q(branching_scheme);
    NodeMap<BranchingScheme> history{0, node_hasher, node_hasher};

    for (output.maximum_size_of_the_queue = parameters.minimum_size_of_the_queue;;
            output.maximum_size_of_the_queue = output.maximum_size_of_the_queue * parameters.growth_factor) {
        if (output.maximum_size_of_the_queue == (Counter)(output.maximum_size_of_the_queue * parameters.growth_factor))
            output.maximum_size_of_the_queue++;
        if (output.maximum_size_of_the_queue > parameters.maximum_size_of_the_queue)
            break;

        std::stringstream ss;
        ss << "q " << output.maximum_size_of_the_queue;
        output.solution_pool.display(ss, parameters.info);

        q.clear();
        history.clear();

        bool stop = true;
        auto node_cur = branching_scheme.root();

        while (node_cur != nullptr || !q.empty()) {
            output.number_of_nodes++;

            // Check time.
            if (parameters.info.needs_to_end())
                goto imbastarend;

            // Check node limit.
            if (parameters.maximum_number_of_nodes != -1
                    && output.number_of_nodes > parameters.maximum_number_of_nodes)
                goto imbastarend;

            // Get node from the queue.
            if (node_cur == nullptr) {
                node_cur = *q.begin();
                //remove_from_history_and_queue(branching_scheme, history, q, q.begin());
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
                    ss << "q " << output.maximum_size_of_the_queue;
                    output.solution_pool.add(child, ss, parameters.info);
                }
                // Add child to the queue.
                if (!branching_scheme.leaf(child)
                        && !branching_scheme.bound(child, output.solution_pool.worst())) {
                    if ((Counter)q.size() == output.maximum_size_of_the_queue) {
                        stop = false;
                    }
                    if ((Counter)q.size() < output.maximum_size_of_the_queue
                            || branching_scheme(child, *(std::prev(q.end())))) {
                        add_to_history_and_queue(branching_scheme, history, q, child);
                        if ((Counter)q.size() > output.maximum_size_of_the_queue) {
                            //remove_from_history_and_queue(branching_scheme, history, q, std::prev(q.end()));
                            q.erase(std::prev(q.end()));
                        }
                    }
                }
            }

            // If node_cur still has children, put it back to the queue.
            if (branching_scheme.infertile(node_cur)) {
                node_cur = nullptr;
            } else if ((Counter)q.size() != 0
                    && branching_scheme(*(q.begin()), node_cur)) {
                //add_to_history_and_queue(branching_scheme, history, q, node_cur);
                q.insert(node_cur);
                node_cur = nullptr;
                if ((Counter)q.size() > output.maximum_size_of_the_queue) {
                    stop = false;
                    //remove_from_history_and_queue(branching_scheme, history, q, std::prev(q.end()));
                    q.erase(std::prev(q.end()));
                }
            }

        }
        if (stop)
            break;
    }
imbastarend:

    output.solution_pool.display_end(parameters.info);
    parameters.info.os() << "Number of nodes:            " << output.number_of_nodes << std::endl;
    parameters.info.add_to_json("Algorithm", "NumberOfNodes", output.number_of_nodes);
    return output;
}

}

