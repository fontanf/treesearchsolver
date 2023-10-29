#pragma once

#include "treesearchsolver/common.hpp"

namespace treesearchsolver
{

template <typename BranchingScheme>
struct AnytimeColumnSearchOutput
{
    AnytimeColumnSearchOutput(
            const BranchingScheme& branching_scheme,
            Counter maximum_size_of_the_solution_pool):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }

    /** Solution pool. */
    SolutionPool<BranchingScheme> solution_pool;

    /** Number of nodes explored. */
    Counter number_of_nodes = 0;

    /** Number of iterations. */
    Counter number_of_iterations = 0;
};

template <typename BranchingScheme>
using AnytimeColumnSearchCallback = std::function<void(const AnytimeColumnSearchOutput<BranchingScheme>&)>;

template <typename BranchingScheme>
struct AnytimeColumnSearchOptionalParameters
{
    using Node = typename BranchingScheme::Node;

    /** Maximum size of the solution pool. */
    NodeId maximum_size_of_the_solution_pool = 1;

    /** Initial size of the column. */
    Counter initial_column_size = 1;

    /** Growth factor of the column size. */
    double column_size_growth_factor = 1.5;

    /** Maximum number of nodes. */
    NodeId maximum_number_of_nodes = -1;

    /** Maximum number of iterations. */
    NodeId maximum_number_of_iterations = -1;

    /**
     * Goal.
     *
     * If not 'nullptr', The alglorithm stops as soon as a better node is
     * found.
     */
    std::shared_ptr<Node> goal = nullptr;

    /** Callback function called when a new best solution is found. */
    AnytimeColumnSearchCallback<BranchingScheme> new_solution_callback
        = [](const AnytimeColumnSearchOutput<BranchingScheme>&) { };

    /** Info structure. */
    optimizationtools::Info info = optimizationtools::Info();
};

template <typename BranchingScheme>
inline AnytimeColumnSearchOutput<BranchingScheme> anytime_column_search(
        const BranchingScheme& branching_scheme,
        AnytimeColumnSearchOptionalParameters<BranchingScheme> parameters = {})
{
    using Node = typename BranchingScheme::Node;

    // Initial display.
    parameters.info.os()
            << "======================================" << std::endl
            << "           TreeSearchSolver           " << std::endl
            << "======================================" << std::endl
            << std::endl
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Anytime Column Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Initial column size:           " << parameters.initial_column_size << std::endl
            << "Column size growth factor:     " << parameters.column_size_growth_factor << std::endl
            << "Maximum number of nodes:       " << parameters.maximum_number_of_nodes << std::endl
            << "Maximum number of iterations:  " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum size of the pool:      " << parameters.maximum_size_of_the_solution_pool << std::endl
            << "Time limit:                    " << parameters.info.time_limit << std::endl
            << std::endl;

    AnytimeColumnSearchOutput<BranchingScheme> output(
            branching_scheme, parameters.maximum_size_of_the_solution_pool);
    output.solution_pool.display_init(parameters.info);

    // Initialize q and history.
    auto node_hasher = branching_scheme.node_hasher();
    std::vector<NodeSet<BranchingScheme>> q
        = {NodeSet<BranchingScheme>(branching_scheme)};
    std::vector<NodeMap<BranchingScheme>> history
        = {NodeMap<BranchingScheme>(0, node_hasher, node_hasher)};
    q[0].insert(branching_scheme.root());

    double maximum_number_of_children = parameters.initial_column_size;

    for (output.number_of_iterations = 0;; ++output.number_of_iterations) {
        // Check maximum number of iterations.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations > parameters.maximum_number_of_iterations)
            goto acsend;

        // Display.
        std::stringstream ss;
        ss << "iteration " << output.number_of_iterations;
        output.solution_pool.display(ss, parameters.info);

        NodeId number_of_nodes_back = output.number_of_nodes;
        for (Counter current_depth = 0; current_depth < (Counter)q.size(); ++current_depth) {

            // Number of children generated at the current depth.
            Counter number_of_children = 0;
            std::shared_ptr<Node> current_node = nullptr;
            while ((current_node != nullptr || !q[current_depth].empty())
                    && number_of_children < maximum_number_of_children) {

                // Get node from the queue.
                if (current_node == nullptr) {
                    current_node = *(q[current_depth].begin());
                    q[current_depth].erase(q[current_depth].begin());

                    // Bound.
                    if (branching_scheme.bound(current_node, output.solution_pool.worst())) {
                        current_node = nullptr;
                        continue;
                    }
                }

                // Get next child.
                auto child = branching_scheme.next_child(current_node);

                if (child != nullptr) {

                    output.number_of_nodes++;

                    // Check time.
                    if (parameters.info.needs_to_end())
                        goto acsend;

                    // Check node limit.
                    if (parameters.maximum_number_of_nodes != -1
                            && output.number_of_nodes > parameters.maximum_number_of_nodes)
                        goto acsend;

                    // Check best known bound.
                    if (parameters.goal != nullptr
                            && !branching_scheme.better(
                                parameters.goal,
                                output.solution_pool.best()))
                        goto acsend;

                    // Get child depth.
                    Depth child_depth = depth(branching_scheme, child);
                    if (child_depth == -1)
                        child_depth = current_depth + 1;

                    // Update best solution.
                    if (branching_scheme.better(child, output.solution_pool.worst())) {
                        std::stringstream ss;
                        ss << "iteration " << output.number_of_iterations;
                        output.solution_pool.add(child, ss, parameters.info);
                        parameters.new_solution_callback(output);
                        //output.solution_pool.display(ss, parameters.info);
                    }

                    // Add child to the queue.
                    if (!branching_scheme.leaf(child)
                            && !branching_scheme.bound(child, output.solution_pool.worst())) {
                        number_of_children++;
                        while ((Counter)q.size() <= child_depth) {
                            q.push_back(NodeSet<BranchingScheme>(
                                        branching_scheme));
                            history.push_back(NodeMap<BranchingScheme>(
                                        0, node_hasher, node_hasher));
                        }
                        add_to_history_and_queue(
                                branching_scheme,
                                history[child_depth],
                                q[child_depth],
                                child);
                    }
                }

                // If current_node still has children, put it back to the queue.
                if (branching_scheme.infertile(current_node)) {
                    current_node = nullptr;
                } else if (!q[current_depth].empty()
                        && branching_scheme(*(q[current_depth].begin()), current_node)) {
                    q[current_depth].insert(current_node);
                    current_node = nullptr;
                }

            }

            // Now that the exploration of the current depth has finished for
            // this iteration, add back the current node to the queue.
            if (current_node != nullptr) {
                q[current_depth].insert(current_node);
                current_node = nullptr;
            }

        }

        if (output.number_of_nodes == number_of_nodes_back) {
            break;
        }

        // Update column size.
        maximum_number_of_children *= parameters.column_size_growth_factor;
    }
acsend:

    output.solution_pool.display_end(parameters.info);
    parameters.info.os() << "Number of nodes:            " << output.number_of_nodes << std::endl;
    parameters.info.os() << "Number of iterations:       " << output.number_of_iterations << std::endl;
    parameters.info.add_to_json("Algorithm", "NumberOfNodes", output.number_of_nodes);
    parameters.info.add_to_json("Algorithm", "NumberOfIterations", output.number_of_iterations);
    return output;
}

}

