#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct IterativeBeamSearchParameters: Parameters<BranchingScheme>
{
    /** Growth factor of the size of the queue. */
    double growth_factor = 2;

    /** Minimum size of the queue. */
    NodeId minimum_size_of_the_queue = 1;

    /** Maximum size of the queue. */
    NodeId maximum_size_of_the_queue = 100000000;

    /** Maximum number of nodes. */
    NodeId maximum_number_of_nodes = -1;

    /**
     * If true, use a single history shared across all depth levels.
     * If false (default), each depth level has its own history.
     */
    bool global_history = false;


    virtual int format_width() const override { return 36; }

    virtual void format(std::ostream& os) const override
    {
        Parameters<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Maximum number of nodes: " << maximum_number_of_nodes << std::endl
            << std::setw(width) << std::left << "Growth factor: " << growth_factor << std::endl
            << std::setw(width) << std::left << "Minimum size of the queue: " << minimum_size_of_the_queue << std::endl
            << std::setw(width) << std::left << "Maximum size of the queue: " << maximum_size_of_the_queue << std::endl
            << std::setw(width) << std::left << "Global history: " << global_history << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters<BranchingScheme>::to_json();
        json.merge_patch({
                {"MaximumNumberOfNodes", maximum_number_of_nodes},
                {"GrowthFactor", growth_factor},
                {"MinimumSizeOfTheQueue", minimum_size_of_the_queue},
                {"MaximumSizeOfTheQueue", maximum_size_of_the_queue},
                {"GlobalHistory", global_history}});
        return json;
    }
};

template <typename BranchingScheme>
struct IterativeBeamSearchOutput: Output<BranchingScheme>
{
    IterativeBeamSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
       Output<BranchingScheme>(branching_scheme, maximum_size_of_the_solution_pool) { }


    /** Number of nodes explored. */
    NodeId number_of_nodes = 0;

    /** Maximum size of the queue reached. */
    NodeId maximum_size_of_the_queue = 0;

    /** True if all nodes have been explored. */
    bool optimal = false;


    virtual int format_width() const override { return 28; }

    virtual void format(std::ostream& os) const override
    {
        Output<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of nodes: " << number_of_nodes << std::endl
            << std::setw(width) << std::left << "Maximum size of the queue: " << maximum_size_of_the_queue << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output<BranchingScheme>::to_json();
        json.merge_patch({
                {"NumberOfNodes", number_of_nodes},
                {"MaximumSizeOfTheQueue", maximum_size_of_the_queue}});
        return json;
    }
};

template <typename BranchingScheme>
inline const IterativeBeamSearchOutput<BranchingScheme> iterative_beam_search(
        const BranchingScheme& branching_scheme,
        const IterativeBeamSearchParameters<BranchingScheme>& parameters = {})
{
    // Initial display.
    IterativeBeamSearchOutput<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Iterative beam search");
    algorithm_formatter.print_header();

    // Initialize q and history.
    auto node_hasher = branching_scheme.node_hasher();
    std::vector<std::shared_ptr<NodeSet<BranchingScheme>>> q(2, nullptr);
    std::vector<std::shared_ptr<NodeMap<BranchingScheme>>> history(2, nullptr);
    q[0] = std::shared_ptr<NodeSet<BranchingScheme>>(
            new NodeSet<BranchingScheme>(branching_scheme));
    q[1] = std::shared_ptr<NodeSet<BranchingScheme>>(
            new NodeSet<BranchingScheme>(branching_scheme));
    std::shared_ptr<NodeMap<BranchingScheme>> global_history_map;
    if (parameters.global_history) {
        global_history_map = std::shared_ptr<NodeMap<BranchingScheme>>(
                new NodeMap<BranchingScheme>(0, node_hasher, node_hasher));
    } else {
        history[0] = std::shared_ptr<NodeMap<BranchingScheme>>(
                new NodeMap<BranchingScheme>(0, node_hasher, node_hasher));
        history[1] = std::shared_ptr<NodeMap<BranchingScheme>>(
                new NodeMap<BranchingScheme>(0, node_hasher, node_hasher));
    }
    auto get_history = [&](Depth d) -> NodeMap<BranchingScheme>& {
        return parameters.global_history? *global_history_map: *history[d];
    };
    Depth number_of_queues = 2;

    bool end = false;
    for (output.maximum_size_of_the_queue = parameters.minimum_size_of_the_queue;;) {
        //std::cout << "beam_size " << output.maximum_size_of_the_queue << std::endl;

        // Initialize queue.
        bool stop = true;
        q[0]->insert(branching_scheme.root());

        Depth current_depth = 0;
        for (;;) {

            while (!q[current_depth]->empty()) {

                // Get node from the queue.
                auto current_node = *q[current_depth]->begin();
                q[current_depth]->erase(q[current_depth]->begin());

                // Bound.
                if (branching_scheme.bound(current_node, output.solution_pool.worst())) {
                    continue;
                }

                // Check cutoff.
                if (parameters.cutoff != nullptr
                        && branching_scheme.bound(current_node, parameters.cutoff)) {
                    continue;
                }

                // Check time.
                if (parameters.timer.needs_to_end()) {
                    end = true;
                    break;
                }

                // Check best known bound.
                if (parameters.goal != nullptr
                        && !branching_scheme.better(
                            parameters.goal,
                            output.solution_pool.best())) {
                    end = true;
                    break;
                }

                // Get next child.
                bool all_children_generated = false;
                for (NodeId child_id = 0;
                        child_id < output.maximum_size_of_the_queue;
                        ++child_id) {
                    if (branching_scheme.infertile(current_node)) {
                        all_children_generated = true;
                        break;
                    }

                    auto child = branching_scheme.next_child(current_node);
                    if (child == nullptr)
                        continue;

                    output.number_of_nodes++;

                    // Check time.
                    if (parameters.timer.needs_to_end()) {
                        end = true;
                        break;
                    }

                    // Check node limit.
                    if (parameters.maximum_number_of_nodes != -1
                            && output.number_of_nodes > parameters.maximum_number_of_nodes) {
                        end = true;
                        break;
                    }

                    // Check goal.
                    if (parameters.goal != nullptr
                            && !branching_scheme.better(
                                parameters.goal,
                                output.solution_pool.best())) {
                        end = true;
                        break;
                    }

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
                            && !branching_scheme.bound(child, output.solution_pool.worst())
                            && (parameters.cutoff == nullptr || !branching_scheme.bound(child, parameters.cutoff))) {

                        // Create new q and history if needed.
                        while (child_depth >= current_depth + number_of_queues) {
                            if ((Depth)q.size() <= current_depth + number_of_queues) {
                                q.push_back(nullptr);
                                if (!parameters.global_history)
                                    history.push_back(nullptr);
                            }
                            q[current_depth + number_of_queues]
                                = std::shared_ptr<NodeSet<BranchingScheme>>(
                                        new NodeSet<BranchingScheme>(branching_scheme));
                            if (!parameters.global_history)
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
                                    get_history(child_depth),
                                    *q[child_depth],
                                    child);
                            if ((NodeId)q[child_depth]->size() > output.maximum_size_of_the_queue)
                                remove_from_history_and_queue(
                                        branching_scheme,
                                        get_history(child_depth),
                                        *q[child_depth],
                                        std::prev(q[child_depth]->end()));
                        }
                    }
                }
                if (!all_children_generated)
                    stop = false;
            }
            if (end)
                break;

            // Update q and history.
            if ((Depth)q.size() <= current_depth + number_of_queues) {
                q.push_back(nullptr);
                if (!parameters.global_history)
                    history.push_back(nullptr);
            }
            q[current_depth]->clear();
            if (!parameters.global_history) {
                history[current_depth]->clear();
                history[current_depth + number_of_queues] = history[current_depth];
                history[current_depth] = nullptr;
            }
            q[current_depth + number_of_queues] = q[current_depth];
            q[current_depth] = nullptr;

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
        if (end)
            break;

        // Update q and history.
        for (Depth d = 0; d < number_of_queues; ++d) {
            q[d] = q[current_depth + d];
            q[current_depth + d] = nullptr;
            q[d]->clear();
            if (!parameters.global_history) {
                history[d] = history[current_depth + d];
                history[current_depth + d] = nullptr;
                history[d]->clear();
            }
        }
        if (parameters.global_history)
            global_history_map->clear();

        if (stop) {
            output.optimal = true;
            parameters.new_solution_callback(output);
        }

        std::stringstream ss;
        ss << "q " << output.maximum_size_of_the_queue;
        algorithm_formatter.print(ss);

        // Increase the size of the queue.
        NodeId maximum_size_of_the_queue_next = std::max(
                output.maximum_size_of_the_queue + 1,
                (NodeId)(output.maximum_size_of_the_queue * parameters.growth_factor));
        if (maximum_size_of_the_queue_next > parameters.maximum_size_of_the_queue)
            break;
        output.maximum_size_of_the_queue = maximum_size_of_the_queue_next;

        // Stop if no nodes has been pruned.
        if (stop)
            break;
    }
ibsend:

    algorithm_formatter.end();
    return output;
}

}

