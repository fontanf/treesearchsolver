#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct IterativeBeamSearchOutput
{
    IterativeBeamSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    /** Solution pool. */
    SolutionPool<BranchingScheme> solution_pool;
    /** Number of nodes explored. */
    NodeId number_of_nodes = 0;
    /** Maximum size of the queue reached. */
    NodeId maximum_size_of_the_queue = 0;
};

template <typename BranchingScheme>
using IterativeBeamSearchCallback = std::function<void(const IterativeBeamSearchOutput<BranchingScheme>&)>;

template <typename BranchingScheme>
struct IterativeBeamSearchOptionalParameters
{
    using Node = typename BranchingScheme::Node;

    /** Maximum size of the solution pool. */
    NodeId maximum_size_of_the_solution_pool = 1;
    /** Growth factor of the size of the queue. */
    double growth_factor = 2;
    /** Minimum size of the queue. */
    NodeId minimum_size_of_the_queue = 1;
    /** Maximum size of the queue. */
    NodeId maximum_size_of_the_queue = 100000000;
    /** Maximum number of nodes. */
    NodeId maximum_number_of_nodes = -1;
    /**
     * Goal.
     *
     * If not 'nullptr', The alglorithm stops as soon as a better node is
     * found.
     */
    std::shared_ptr<Node> goal = nullptr;
    /** Callback function called when a new best solution is found. */
    IterativeBeamSearchCallback<BranchingScheme> new_solution_callback
        = [](const IterativeBeamSearchOutput<BranchingScheme>&) { };
    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
inline IterativeBeamSearchOutput<BranchingScheme> iterative_beam_search(
        const BranchingScheme& branching_scheme,
        IterativeBeamSearchOptionalParameters<BranchingScheme> parameters = {})
{
    // Initial display.
    parameters.info.os()
            << "======================================" << std::endl
            << "          Tree Search Solver          " << std::endl
            << "======================================" << std::endl
            << std::endl
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Iterative Beam Search" << std::endl
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

    IterativeBeamSearchOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);
    output.solution_pool.display_init(parameters.info);

    // Initialize q and history.
    auto node_hasher = branching_scheme.node_hasher();
    std::vector<std::shared_ptr<NodeSet<BranchingScheme>>> q(2, nullptr);
    std::vector<std::shared_ptr<NodeMap<BranchingScheme>>> history(2, nullptr);
    q[0] = std::shared_ptr<NodeSet<BranchingScheme>>(
            new NodeSet<BranchingScheme>(branching_scheme));
    history[0] = std::shared_ptr<NodeMap<BranchingScheme>>(
            new NodeMap<BranchingScheme>(0, node_hasher, node_hasher));
    q[1] = std::shared_ptr<NodeSet<BranchingScheme>>(
            new NodeSet<BranchingScheme>(branching_scheme));
    history[1] = std::shared_ptr<NodeMap<BranchingScheme>>(
            new NodeMap<BranchingScheme>(0, node_hasher, node_hasher));
    Depth number_of_queues = 2;

    for (output.maximum_size_of_the_queue = parameters.minimum_size_of_the_queue;;) {

        std::stringstream ss;
        ss << "q " << output.maximum_size_of_the_queue;
        output.solution_pool.display(ss, parameters.info);

        // Initialize queue.
        bool stop = true;
        auto current_node = branching_scheme.root();
        q[0]->insert(current_node);

        Depth current_depth = 0;
        for (;;) {

            current_node = nullptr;
            while (current_node != nullptr || !q[current_depth]->empty()) {

                // Get node from the queue.
                if (current_node == nullptr) {
                    current_node = *q[current_depth]->begin();
                    q[current_depth]->erase(q[current_depth]->begin());

                    // Bound.
                    if (branching_scheme.bound(current_node, output.solution_pool.worst())) {
                        current_node = nullptr;
                        continue;
                    }
                }

                if ((NodeId)q[current_depth + 1]->size() == output.maximum_size_of_the_queue
                        && branching_scheme(*(std::prev(q[current_depth + 1]->end())), current_node)) {
                    stop = false;
                    break;
                }

                // Get next child.
                auto child = branching_scheme.next_child(current_node);

                if (child != nullptr) {

                    output.number_of_nodes++;

                    // Check time.
                    if (parameters.info.needs_to_end())
                        goto ibsend;

                    // Check node limit.
                    if (parameters.maximum_number_of_nodes != -1
                            && output.number_of_nodes > parameters.maximum_number_of_nodes)
                        goto ibsend;

                    // Check best known bound.
                    if (parameters.goal != nullptr
                            && !branching_scheme.better(
                                parameters.goal,
                                output.solution_pool.best()))
                        goto ibsend;

                    // Get child depth.
                    Depth child_depth = depth(branching_scheme, child);
                    if (child_depth == -1)
                        child_depth = current_depth + 1;

                    // Update best solution.
                    if (branching_scheme.better(child, output.solution_pool.worst())) {
                        std::stringstream ss;
                        ss << "q " << output.maximum_size_of_the_queue;
                        output.solution_pool.add(child, ss, parameters.info);
                        parameters.new_solution_callback(output);
                    }

                    // Add child to the queue.
                    if (!branching_scheme.leaf(child)
                            && !branching_scheme.bound(child, output.solution_pool.worst())) {

                        // Create new q and history if needed.
                        while (child_depth >= current_depth + number_of_queues) {
                            if ((Depth)q.size() <= current_depth + number_of_queues) {
                                q.push_back(nullptr);
                                history.push_back(nullptr);
                            }
                            q[current_depth + number_of_queues]
                                = std::shared_ptr<NodeSet<BranchingScheme>>(
                                        new NodeSet<BranchingScheme>(branching_scheme));
                            history[current_depth + number_of_queues]
                                = std::shared_ptr<NodeMap<BranchingScheme>>(
                                        new NodeMap<BranchingScheme>(0, node_hasher, node_hasher));
                            number_of_queues++;
                        }

                        // Update stop.
                        if ((NodeId)q[child_depth]->size() >= output.maximum_size_of_the_queue)
                            stop = false;

                        // Check queue size.
                        if ((NodeId)q[child_depth]->size() < output.maximum_size_of_the_queue
                                || branching_scheme(child, *(std::prev(q[child_depth]->end())))) {
                            add_to_history_and_queue(
                                    branching_scheme,
                                    *history[child_depth],
                                    *q[child_depth],
                                    child);
                            //q_next->insert(child);
                            if ((NodeId)q[child_depth]->size() > output.maximum_size_of_the_queue)
                                remove_from_history_and_queue(
                                        branching_scheme,
                                        *history[child_depth],
                                        *q[child_depth],
                                        std::prev(q[child_depth]->end()));
                        }
                    }
                }

                // If current_node still has children, put it back to the queue.
                if (branching_scheme.infertile(current_node)) {
                    current_node = nullptr;
                } else if (!q[current_depth]->empty()
                        && branching_scheme(*(q[current_depth]->begin()), current_node)) {
                    q[current_depth]->insert(current_node);
                    current_node = nullptr;
                }

            }

            // Update q and history.
            if ((Depth)q.size() <= current_depth + number_of_queues) {
                q.push_back(nullptr);
                history.push_back(nullptr);
            }
            q[current_depth]->clear();
            history[current_depth]->clear();
            q[current_depth + number_of_queues] = q[current_depth];
            history[current_depth + number_of_queues] = history[current_depth];
            q[current_depth] = nullptr;
            history[current_depth] = nullptr;

            // Stop criteria.
            current_depth++;
            bool terminate = true;
            for (Depth d = 0; d < number_of_queues; ++d) {
                if (!q[current_depth + d]->empty()) {
                    terminate = false;
                    break;
                }
            }
            if (terminate)
                break;

        }

        // Update q and history.
        for (Depth d = 0; d < number_of_queues; ++d) {
            q[d] = q[current_depth + d];
            history[d] = history[current_depth + d];
            q[current_depth + d] = nullptr;
            history[current_depth + d] = nullptr;
            q[d]->clear();
            history[d]->clear();
        }

        // Increase the size of the queue.
        output.maximum_size_of_the_queue = std::max(
                output.maximum_size_of_the_queue + 1,
                (NodeId)(output.maximum_size_of_the_queue * parameters.growth_factor));
        if (output.maximum_size_of_the_queue > parameters.maximum_size_of_the_queue)
            break;

        // Stop if no nodes has been pruned.
        if (stop)
            break;
    }
ibsend:

    output.solution_pool.display_end(parameters.info);
    parameters.info.os() << "Number of nodes:            " << output.number_of_nodes << std::endl;
    parameters.info.os() << "Maximum size of the queue:  " << output.maximum_size_of_the_queue << std::endl;
    parameters.info.add_to_json("Algorithm", "NumberOfNodes", output.number_of_nodes);
    parameters.info.add_to_json("Algorithm", "MaximumSizeOfTheQueue", output.maximum_size_of_the_queue);
    return output;
}

}

