#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct AStarOptionalParameters
{
    NodeId solution_pool_size_max = 1;
    NodeId node_number_max = -1;

    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct AStarOutput
{
    AStarOutput(
            const BranchingScheme& branching_scheme,
            Counter solution_pool_size_max):
        solution_pool(branching_scheme, solution_pool_size_max) { }

    SolutionPool<BranchingScheme> solution_pool;

    Counter node_number = 0;
};

template <typename BranchingScheme>
inline AStarOutput<BranchingScheme> astar(
        const BranchingScheme& branching_scheme,
        AStarOptionalParameters parameters = {})
{
    AStarOutput<BranchingScheme> output(
            branching_scheme, parameters.solution_pool_size_max);
    output.solution_pool.display_init(parameters.info);
    auto node_hasher = branching_scheme.node_hasher();
    NodeMap<BranchingScheme> history{0, node_hasher, node_hasher};
    NodeSet<BranchingScheme> q(branching_scheme);

    auto node_cur = branching_scheme.root();

    while (node_cur != nullptr || !q.empty()) {
        output.node_number++;

        // Check time.
        if (!parameters.info.check_time())
            break;

        // Check node limit.
        if (parameters.node_number_max != -1
                && output.node_number > parameters.node_number_max)
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
                ss << "node " << output.node_number;
                output.solution_pool.add(child, ss, parameters.info);
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
    VER(parameters.info, "Node number: " << output.node_number << std::endl);
    PUT(parameters.info, "Algorithm", "NodeNumber", output.node_number);
    return output;
}

}

