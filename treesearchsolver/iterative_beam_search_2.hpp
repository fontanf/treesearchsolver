#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct IterativeBeamSearch2Parameters: Parameters<BranchingScheme>
{
    /** Growth factor of the size of the queue. */
    double growth_factor = 2;

    /** Minimum size of the queue. */
    NodeId minimum_size_of_the_queue = 1;

    /** Maximum size of the queue. */
    NodeId maximum_size_of_the_queue = 100000000;

    /** Maximum number of nodes expanded. */
    NodeId maximum_number_of_nodes_expanded = -1;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Maximum number of nodes expanded: " << maximum_number_of_nodes_expanded << std::endl
            << std::setw(width) << std::left << "Growth factor: " << growth_factor << std::endl
            << std::setw(width) << std::left << "Minimum size of the queue: " << minimum_size_of_the_queue << std::endl
            << std::setw(width) << std::left << "Maximum size of the queue: " << maximum_size_of_the_queue << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters<BranchingScheme>::to_json();
        json.merge_patch({
                {"MaximumNumberOfNodesExpanded", maximum_number_of_nodes_expanded},
                {"GrowthFactor", growth_factor},
                {"MinimumSizeOfTheQueue", minimum_size_of_the_queue},
                {"MaximumSizeOfTheQueue", maximum_size_of_the_queue}});
        return json;
    }
};

template <typename BranchingScheme>
struct IterativeBeamSearch2Output: Output<BranchingScheme>
{
    IterativeBeamSearch2Output(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
       Output<BranchingScheme>(branching_scheme, maximum_size_of_the_solution_pool) { }


    /** Number of nodes generated. */
    NodeId number_of_nodes_generated = 0;

    /** Number of nodes added. */
    NodeId number_of_nodes_added = 0;

    /** Number of nodes processed. */
    NodeId number_of_nodes_processed = 0;

    /** Number of nodes expanded. */
    NodeId number_of_nodes_expanded = 0;

    /** Maximum size of the queue reached. */
    NodeId maximum_size_of_the_queue = 0;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Output<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of nodes generated: " << number_of_nodes_generated << std::endl
            << std::setw(width) << std::left << "Number of nodes added: " << number_of_nodes_added << std::endl
            << std::setw(width) << std::left << "Number of nodes processed: " << number_of_nodes_processed << std::endl
            << std::setw(width) << std::left << "Number of nodes expanded: " << number_of_nodes_expanded << std::endl
            << std::setw(width) << std::left << "Maximum size of the queue: " << maximum_size_of_the_queue << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output<BranchingScheme>::to_json();
        json.merge_patch({
                {"NumberOfNodesGenerated", number_of_nodes_generated},
                {"NumberOfNodesAdded", number_of_nodes_added},
                {"NumberOfNodesProcessed", number_of_nodes_processed},
                {"NumberOfNodesExpanded", number_of_nodes_expanded},
                {"MaximumSizeOfTheQueue", maximum_size_of_the_queue}});
        return json;
    }
};

template <typename BranchingScheme>
inline const IterativeBeamSearch2Output<BranchingScheme> iterative_beam_search_2(
        const BranchingScheme& branching_scheme,
        const IterativeBeamSearch2Parameters<BranchingScheme>& parameters = {})
{
    // Initial display.
    IterativeBeamSearch2Output<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Iterative beam search 2");
    algorithm_formatter.print_header();

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
        algorithm_formatter.print(ss);

        // Initialize queue.
        bool stop = true;
        auto current_node = branching_scheme.root();
        output.number_of_nodes_generated++;
        q[0]->insert(current_node);
        output.number_of_nodes_added++;

        Depth current_depth = 0;
        for (;;) {
            //std::cout << "depth " << current_depth << std::endl;

            while (!q[current_depth]->empty()) {

                // Get node from the queue.
                auto current_node = *q[current_depth]->begin();
                q[current_depth]->erase(q[current_depth]->begin());
                output.number_of_nodes_processed++;

                // Bound.
                if (branching_scheme.bound(current_node, output.solution_pool.worst())) {
                    continue;
                }

                if ((NodeId)q[current_depth + 1]->size() == output.maximum_size_of_the_queue
                        && branching_scheme(*(std::prev(q[current_depth + 1]->end())), current_node)) {
                    stop = false;
                    break;
                }

                // Get next child.
                auto children = branching_scheme.children(current_node);
                output.number_of_nodes_expanded++;

                for (auto child: children) {

                    output.number_of_nodes_generated++;

                    // Check time.
                    if (parameters.timer.needs_to_end())
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
                        algorithm_formatter.update_solution(child);
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
                            bool added = add_to_history_and_queue(
                                    branching_scheme,
                                    *history[child_depth],
                                    *q[child_depth],
                                    child);
                            if (added)
                                output.number_of_nodes_added++;
                            //q_next->insert(child);
                            if ((NodeId)q[child_depth]->size() > output.maximum_size_of_the_queue) {
                                remove_from_history_and_queue(
                                        branching_scheme,
                                        *history[child_depth],
                                        *q[child_depth],
                                        std::prev(q[child_depth]->end()));
                            }
                        }
                    }
                }

                // Check node limit.
                if (parameters.maximum_number_of_nodes_expanded != -1
                        && output.number_of_nodes_expanded > parameters.maximum_number_of_nodes_expanded)
                    goto ibsend;

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

    algorithm_formatter.end();
    return output;
}

}

