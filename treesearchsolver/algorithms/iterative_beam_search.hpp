#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

struct IterativeBeamSearchOptionalParameters
{
    NodeId maximum_size_of_the_solution_pool = 1;
    double growth_factor = 2;
    NodeId minimum_size_of_the_queue = 0;
    NodeId maximum_size_of_the_queue = 100000000;
    NodeId maximum_number_of_nodes = -1;

    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
struct IterativeBeamSearchOutput
{
    IterativeBeamSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    SolutionPool<BranchingScheme> solution_pool;

    Counter number_of_nodes = 0;
    NodeId maximum_size_of_the_queue = 0;
};

template <typename BranchingScheme>
inline IterativeBeamSearchOutput<BranchingScheme> iterativebeamsearch(
        const BranchingScheme& branching_scheme,
        IterativeBeamSearchOptionalParameters parameters = {})
{
    IterativeBeamSearchOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);
    output.solution_pool.display_init(parameters.info);
    auto node_hasher = branching_scheme.node_hasher();
    NodeSet<BranchingScheme> q1(branching_scheme);
    NodeSet<BranchingScheme> q2(branching_scheme);
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

        // Initialize queue.
        bool stop = true;
        q1.clear();
        q2.clear();
        NodeSet<BranchingScheme>* q = &q1;
        NodeSet<BranchingScheme>* q_next = &q2;
        NodeSet<BranchingScheme>* q_tmp = nullptr;
        auto node_cur = branching_scheme.root();
        q->insert(node_cur);

        for (Counter depth = 0; !q->empty(); ++depth) {
            history.clear();
            q_next->clear();

            node_cur = nullptr;
            while (node_cur != nullptr || !q->empty()) {
                output.number_of_nodes++;

                // Check time.
                if (!parameters.info.check_time())
                    goto ibsend;

                // Check node limit.
                if (parameters.maximum_number_of_nodes != -1
                        && output.number_of_nodes > parameters.maximum_number_of_nodes)
                    goto ibsend;

                // Get node from the queue.
                if (node_cur == nullptr) {
                    node_cur = *q->begin();
                    q->erase(q->begin());

                    // Bound.
                    if (branching_scheme.bound(node_cur, output.solution_pool.worst())) {
                        node_cur = nullptr;
                        continue;
                    }
                }

                if ((Counter)q_next->size() == output.maximum_size_of_the_queue
                        && branching_scheme(*(std::prev(q_next->end())), node_cur)) {
                    stop = false;
                    break;
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
                        if ((Counter)q_next->size() >= output.maximum_size_of_the_queue)
                            stop = false;
                        if ((Counter)q_next->size() < output.maximum_size_of_the_queue
                                || branching_scheme(child, *(std::prev(q_next->end())))) {
                            add_to_history_and_queue(branching_scheme, history, *q_next, child);
                            //q_next->insert(child);
                            if ((Counter)q_next->size() > output.maximum_size_of_the_queue)
                                remove_from_history_and_queue(branching_scheme, history, *q_next, std::prev(q_next->end()));
                        }
                    }
                }

                // If node_cur still has children, put it back to the queue.
                if (branching_scheme.infertile(node_cur)) {
                    node_cur = nullptr;
                } else if ((Counter)q->size() != 0
                        && branching_scheme(*(q->begin()), node_cur)) {
                    q->insert(node_cur);
                    node_cur = nullptr;
                }

            }
            q_tmp = q;
            q = q_next;
            q_next = q_tmp;
        }

        if (stop)
            break;
    }
ibsend:

    output.solution_pool.display_end(parameters.info);
    VER(parameters.info, "Node number: " << output.number_of_nodes << std::endl);
    PUT(parameters.info, "Algorithm", "NodeNumber", output.number_of_nodes);
    return output;
}

}

