#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

#include <queue>

namespace treesearchsolver
{

template <typename BranchingScheme>
struct NestedBestFirstBreadthFirstSearchParameters: Parameters<BranchingScheme>
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
struct NestedBestFirstBreadthFirstSearchOutput: Output<BranchingScheme>
{
    NestedBestFirstBreadthFirstSearchOutput(
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
inline const NestedBestFirstBreadthFirstSearchOutput<BranchingScheme> nested_best_first_breadth_first_search(
        const BranchingScheme& branching_scheme,
        const NestedBestFirstBreadthFirstSearchParameters<BranchingScheme>& parameters = {})
{
    using Node = typename BranchingScheme::Node;

    // Initial display.
    NestedBestFirstBreadthFirstSearchOutput<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Nested best first breadth first search");
    algorithm_formatter.print_header();

    auto node_hasher = branching_scheme.node_hasher();
    NodeMap<BranchingScheme> history{0, node_hasher, node_hasher};
    NodeSet<BranchingScheme> q(branching_scheme);

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
        std::cout << branching_scheme.display(current_node) << std::endl;

        // Bound.
        if (branching_scheme.bound(current_node, output.solution_pool.worst())) {
            current_node = nullptr;
            continue;
        }

        // Start breadth first search.
        std::queue<std::shared_ptr<Node>> brfs_q;
        NodeMap<BranchingScheme> brfs_history{0, node_hasher, node_hasher};
        brfs_q.push(current_node);
        Counter brfs_number_of_nodes = 0;
        while (!brfs_q.empty()) {

            output.number_of_nodes++;
            brfs_number_of_nodes++;
            if (brfs_number_of_nodes > 1e5)
                break;

            // Get node from the queue.
            auto brfs_current_node = brfs_q.front();
            brfs_q.pop();

            // Generate children.
            while (!branching_scheme.infertile(brfs_current_node)) {

                // Get next child.
                auto brfs_child = branching_scheme.next_child(brfs_current_node);

                // Bound.
                if (brfs_child == nullptr)
                    continue;

                // Update best solution.
                if (branching_scheme.better(brfs_child, output.solution_pool.worst())) {
                    algorithm_formatter.update_solution(brfs_child);
                    std::stringstream ss;
                    ss << "node " << output.number_of_nodes;
                    algorithm_formatter.print(ss);
                }

                // Add child to the queue.
                if (branching_scheme.leaf(brfs_child))
                    continue;

                // Check bound.
                if (branching_scheme.bound(brfs_child, output.solution_pool.worst()))
                    continue;

                bool added = add_to_history_and_queue(branching_scheme, history, q, brfs_child);
                if (added)
                    brfs_q.push(brfs_child);
            }

        }

    }

    algorithm_formatter.end();
    return output;
}

}

