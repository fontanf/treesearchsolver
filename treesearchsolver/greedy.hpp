#pragma once

#include "treesearchsolver/algorithm_formatter.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct GreedyOutput: Output<BranchingScheme>
{
    GreedyOutput(
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
inline const GreedyOutput<BranchingScheme> greedy(
        const BranchingScheme& branching_scheme,
        const Parameters<BranchingScheme>& parameters = {})
{
    using Node = typename BranchingScheme::Node;

    // Initial display.
    GreedyOutput<BranchingScheme> output(
            branching_scheme,
            parameters.maximum_size_of_the_solution_pool);
    AlgorithmFormatter<BranchingScheme> algorithm_formatter(
            branching_scheme,
            parameters,
            output);
    algorithm_formatter.start("Greedy");
    algorithm_formatter.print_header();

    auto current_node = branching_scheme.root();
    for (output.number_of_nodes = 1;; ++output.number_of_nodes) {
        std::shared_ptr<Node> best_child = nullptr;
        while (!branching_scheme.infertile(current_node)) {

            if (best_child != nullptr
                    && branching_scheme(best_child, current_node))
                break;

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

            if (branching_scheme.leaf(child))
                continue;

            // Update best child;
            if (best_child == nullptr
                    || branching_scheme(child, best_child))
                best_child = child;
        }
        if (best_child == nullptr)
            break;

        current_node = best_child;
    }

    algorithm_formatter.end();
    return output;
}

}
