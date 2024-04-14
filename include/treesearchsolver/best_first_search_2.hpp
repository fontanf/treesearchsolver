#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct BestFirstSearch2Parameters: Parameters<BranchingScheme>
{
    /** Maximum number of nodes. */
    NodeId maximum_number_of_nodes = -1;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Parameters<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Maximum number of nodes: " << maximum_number_of_nodes << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Parameters<BranchingScheme>::to_json();
        json.merge_patch({
                {"MaximumNumberOfNodes", maximum_number_of_nodes}});
        return json;
    }
};

template <typename BranchingScheme>
struct BestFirstSearch2Output: Output<BranchingScheme>
{
    BestFirstSearch2Output(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
       Output<BranchingScheme>(branching_scheme, maximum_size_of_the_solution_pool) { }


    /** Number of nodes processed. */
    Counter number_of_nodes = 0;


    virtual int format_width() const override { return 37; }

    virtual void format(std::ostream& os) const override
    {
        Output<BranchingScheme>::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Number of nodes: " << number_of_nodes << std::endl
            ;
    }

    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = Output<BranchingScheme>::to_json();
        json.merge_patch({
                {"NumberOfNodes", number_of_nodes}});
        return json;
    }
};

template <typename BranchingScheme>
inline const BestFirstSearch2Output<BranchingScheme> best_first_search_2(
        const BranchingScheme& branching_scheme,
        const BestFirstSearch2Parameters<BranchingScheme>& parameters = {})
{
    // Initial display.
    BestFirstSearch2Output<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Best first search");
    algorithm_formatter.print_header();

    auto node_hasher = branching_scheme.node_hasher();
    NodeMap<BranchingScheme> history{0, node_hasher, node_hasher};
    NodeSet<BranchingScheme> q(branching_scheme);

    // Add root node to the queue.
    q.insert(branching_scheme.root());

    while (!q.empty()) {

        // Check time.
        if (parameters.timer.needs_to_end())
            break;

        // Check node limit.
        if (parameters.maximum_number_of_nodes != -1
                && output.number_of_nodes > parameters.maximum_number_of_nodes) {
            break;
        }

        // Check goal.
        if (parameters.goal != nullptr
                && !branching_scheme.better(
                    parameters.goal,
                    output.solution_pool.best())) {
            break;
        }

        // Get node from the queue.
        auto current_node = *q.begin();
        q.erase(q.begin());

        output.number_of_nodes++;

        if (output.number_of_nodes % 1000000 == 0)
            std::cout << branching_scheme.display(current_node) << std::endl;

        // Bound.
        if (branching_scheme.bound(current_node, output.solution_pool.worst())) {
            current_node = nullptr;
            continue;
        }

        // Get next child.
        auto children = branching_scheme.children(current_node);

        for (auto child: children) {
            // Update best solution.
            if (branching_scheme.better(child, output.solution_pool.worst())) {
                algorithm_formatter.update_solution(child);
                std::stringstream ss;
                ss << "node " << output.number_of_nodes;
                algorithm_formatter.print(ss);
            }

            // Add child to the queue.
            if (!branching_scheme.leaf(child)
                    && !branching_scheme.bound(child, output.solution_pool.worst())) {
                add_to_history_and_queue(branching_scheme, history, q, child);
            }
        }
    }

    algorithm_formatter.end();
    return output;
}

}
