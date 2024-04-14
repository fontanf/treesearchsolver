#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct IterativeMemoryBoundedBestFirstSearchParameters: Parameters<BranchingScheme>
{
    /** Maximum number of nodes expanded. */
    NodeId maximum_number_of_nodes = -1;

    /** Growth factor of the size of the queue. */
    double growth_factor = 1.5;

    /** Minimum size of the queue. */
    NodeId minimum_size_of_the_queue = 0;

    /** Maximum size of the queue. */
    NodeId maximum_size_of_the_queue = 100000000;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Maximum number of nodes: " << maximum_number_of_nodes << std::endl
            << std::setw(width) << std::left << "Growth factor: " << growth_factor << std::endl
            << std::setw(width) << std::left << "Minimum size of the queue: " << minimum_size_of_the_queue << std::endl
            << std::setw(width) << std::left << "Maximum size of the queue: " << maximum_size_of_the_queue << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters<BranchingScheme>::to_json();
        json.merge_patch({
                {"MaximumNumberOfNodes", maximum_number_of_nodes},
                {"GrowthFactor", growth_factor},
                {"MinimumSizeOfTheQueue", minimum_size_of_the_queue},
                {"MaximumSizeOfTheQueue", maximum_size_of_the_queue}});
        return json;
    }
};

template <typename BranchingScheme>
struct IterativeMemoryBoundedBestFirstSearchOutput: Output<BranchingScheme>
{
    IterativeMemoryBoundedBestFirstSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
       Output<BranchingScheme>(branching_scheme, maximum_size_of_the_solution_pool) { }


    /** Number of nodes explored. */
    Counter number_of_nodes = 0;

    /** Maximum size of the queue reached. */
    NodeId maximum_size_of_the_queue = 0;


    virtual int format_width() const override { return 37; }

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
inline const IterativeMemoryBoundedBestFirstSearchOutput<BranchingScheme> iterative_memory_bounded_best_first_search(
        const BranchingScheme& branching_scheme,
        const IterativeMemoryBoundedBestFirstSearchParameters<BranchingScheme>& parameters = {})
{
    // Initial display.
    IterativeMemoryBoundedBestFirstSearchOutput<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Iterative memory bounded best first search");
    algorithm_formatter.print_header();

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
        algorithm_formatter.print(ss);

        q.clear();
        history.clear();

        bool stop = true;
        auto node_cur = branching_scheme.root();

        while (node_cur != nullptr || !q.empty()) {
            output.number_of_nodes++;

            // Check time.
            if (parameters.timer.needs_to_end())
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
                    algorithm_formatter.update_solution(child);
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

    algorithm_formatter.end();
    return output;
}

}

