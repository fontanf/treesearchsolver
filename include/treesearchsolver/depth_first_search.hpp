#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct DepthFirstSearchParameters: Parameters<BranchingScheme>
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
struct DepthFirstSearchOutput: Output<BranchingScheme>
{
    DepthFirstSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        Output<BranchingScheme>(branching_scheme, maximum_size_of_the_solution_pool) { }


    /** Number of nodes. */
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
inline const DepthFirstSearchOutput<BranchingScheme> depth_first_search(
        const BranchingScheme& branching_scheme,
        const DepthFirstSearchParameters<BranchingScheme>& parameters = {})
{
    using Node = typename BranchingScheme::Node;

    // Initial display.
    DepthFirstSearchOutput<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Depth first search");
    algorithm_formatter.print_header();

    std::vector<std::shared_ptr<Node>> q;
    q.push_back(branching_scheme.root());
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

        std::shared_ptr<Node> current_node = q.back();
        q.pop_back();

        std::vector<std::shared_ptr<Node>> children;
        while (!branching_scheme.infertile(current_node)) {

            // Get next child.
            auto child = branching_scheme.next_child(current_node);
            if (child == nullptr)
                continue;

            // Update best solution.
            if (branching_scheme.better(child, output.solution_pool.worst())) {
                algorithm_formatter.update_solution(child);
                std::stringstream ss;
                ss << "node " << output.number_of_nodes;
                algorithm_formatter.print(ss);
            }

            // Check leaf.
            if (branching_scheme.leaf(child))
                continue;

            // Check bound.
            if (branching_scheme.bound(child, output.solution_pool.worst()))
                continue;

            // Check cutoff.
            if (parameters.cutoff != nullptr
                    && branching_scheme.bound(child, parameters.cutoff))
                continue;

            // Update best child;
            children.push_back(child);
        }

        // Sort children.
        std::sort(
                children.begin(),
                children.end(),
                [&branching_scheme](
                    const std::shared_ptr<Node>& node_1,
                    const std::shared_ptr<Node>& node_2)
                {
                    return branching_scheme(node_1, node_2);
                });

        // Add children to the queue.
        for (const auto& child: children)
            q.push_back(child);

        output.number_of_nodes++;
    }

    algorithm_formatter.end();
    return output;
}

}
