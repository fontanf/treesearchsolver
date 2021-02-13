#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct IterativeMemoryBoundedAStarOptionalParameters
{
    NodeId solution_pool_size_max = 1;
    NodeId node_number_max = -1;
    double growth_factor = 1.5;
    NodeId queue_size_min = 0;
    NodeId queue_size_max = 100000000;

    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct IterativeMemoryBoundedAStarOutput
{
    IterativeMemoryBoundedAStarOutput(
            const BranchingScheme& branching_scheme,
            Counter solution_pool_size_max):
        solution_pool(branching_scheme, solution_pool_size_max) { }

    SolutionPool<BranchingScheme> solution_pool;

    Counter node_number = 0;
    NodeId queue_size_max = 0;
};

template <typename BranchingScheme>
inline IterativeMemoryBoundedAStarOutput<BranchingScheme> iterativememoryboundedastar(
        const BranchingScheme& branching_scheme,
        IterativeMemoryBoundedAStarOptionalParameters parameters = {})
{
    IterativeMemoryBoundedAStarOutput<BranchingScheme> output(
            branching_scheme, parameters.solution_pool_size_max);
    output.solution_pool.display_init(parameters.info);
    auto node_hasher = branching_scheme.node_hasher();
    NodeSet<BranchingScheme> q(branching_scheme);
    NodeMap<BranchingScheme> history{0, node_hasher, node_hasher};

    for (output.queue_size_max = parameters.queue_size_min;;
            output.queue_size_max = output.queue_size_max * parameters.growth_factor) {
        if (output.queue_size_max == (Counter)(output.queue_size_max * parameters.growth_factor))
            output.queue_size_max++;
        if (output.queue_size_max > parameters.queue_size_max)
            break;

        std::stringstream ss;
        ss << "q " << output.queue_size_max;
        output.solution_pool.display(ss, parameters.info);

        q.clear();
        history.clear();

        bool stop = true;
        auto node_cur = branching_scheme.root();

        while (node_cur != nullptr || !q.empty()) {
            output.node_number++;

            // Check time.
            if (!parameters.info.check_time())
                goto imbastarend;

            // Check node limit.
            if (parameters.node_number_max != -1
                    && output.node_number > parameters.node_number_max)
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
                    ss << "q " << output.queue_size_max;
                    output.solution_pool.add(child, ss, parameters.info);
                }
                // Add child to the queue.
                if (!branching_scheme.leaf(child)
                        && !branching_scheme.bound(child, output.solution_pool.worst())) {
                    if ((Counter)q.size() == output.queue_size_max) {
                        stop = false;
                    }
                    if ((Counter)q.size() < output.queue_size_max
                            || branching_scheme(child, *(std::prev(q.end())))) {
                        add_to_history_and_queue(branching_scheme, history, q, child);
                        if ((Counter)q.size() > output.queue_size_max) {
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
                if ((Counter)q.size() > output.queue_size_max) {
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
    VER(parameters.info, "Node number: " << output.node_number << std::endl);
    PUT(parameters.info, "Algorithm", "NodeNumber", output.node_number);
    return output;
}

}

