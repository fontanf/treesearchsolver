#pragma once

#include "optimizationtools/utils/output.hpp"

#include <cstdint>
#include <set>
#include <iomanip>

namespace treesearchsolver
{

using NodeId = int64_t;
using Counter = int64_t;
using Depth = int64_t;
using Value = double;

enum class ObjectiveSense { Min, Max };

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// depth /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename, typename T>
struct HasDepthMethod
{
    static_assert(
        std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type.");
};

template<typename C, typename Ret, typename... Args>
struct HasDepthMethod<C, Ret(Args...)>
{

private:

    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().depth(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:

    static constexpr bool value = type::value;

};

template<typename BranchingScheme>
Depth depth(
        const BranchingScheme&,
        const std::shared_ptr<typename BranchingScheme::Node>&,
        std::false_type)
{
    return -1;
}

template<typename BranchingScheme>
Depth depth(
        const BranchingScheme& branching_scheme,
        const std::shared_ptr<typename BranchingScheme::Node>& node,
        std::true_type)
{
    return branching_scheme.depth(node);
}

template<typename BranchingScheme>
Depth depth(
        const BranchingScheme& branching_scheme,
        const std::shared_ptr<typename BranchingScheme::Node>& node)
{
    return depth(
            branching_scheme,
            node,
            std::integral_constant<
                bool,
                HasDepthMethod<BranchingScheme,
                Depth(const std::shared_ptr<typename BranchingScheme::Node>&)>::value>());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// goal_node ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename, typename T>
struct HasGoalNodeMethod
{
    static_assert(
        std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type.");
};

template<typename C, typename Ret, typename... Args>
struct HasGoalNodeMethod<C, Ret(Args...)>
{

private:

    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().goal_node(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:

    static constexpr bool value = type::value;

};

template<typename BranchingScheme>
std::shared_ptr<typename BranchingScheme::Node> goal_node(
        const BranchingScheme&,
        double,
        std::false_type)
{
    return nullptr;
}

template<typename BranchingScheme>
std::shared_ptr<typename BranchingScheme::Node> goal_node(
        const BranchingScheme& branching_scheme,
        double value,
        std::true_type)
{
    return branching_scheme.goal_node(value);
}

template<typename BranchingScheme>
std::shared_ptr<typename BranchingScheme::Node> goal_node(
        const BranchingScheme& branching_scheme,
        double value)
{
    return goal_node(
            branching_scheme,
            value,
            std::integral_constant<
                bool,
                HasGoalNodeMethod<BranchingScheme,
                std::shared_ptr<typename BranchingScheme::Node>(double)>::value>());
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Solution Pool /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename BranchingScheme>
struct SolutionPoolComparator
{
    using Node = typename BranchingScheme::Node;

    SolutionPoolComparator(const BranchingScheme& branching_scheme):
        branching_scheme(branching_scheme) {  }

    const BranchingScheme& branching_scheme;

    bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const {
        if (branching_scheme.better(node_1, node_2))
            return true;
        if (branching_scheme.better(node_2, node_1))
            return false;
        if (branching_scheme.equals(node_1, node_2))
            return false;
        return node_1.get() < node_2.get();
    }
};

/**
 * Solution pool class.
 */
template <typename BranchingScheme>
class SolutionPool
{
    using Node = typename BranchingScheme::Node;

public:

    /** Constructor. */
    SolutionPool(
            const BranchingScheme& branching_scheme,
            Counter size_max):
        branching_scheme_(branching_scheme),
        size_max_(size_max),
        solution_pool_comparator_(branching_scheme),
        solutions_(solution_pool_comparator_)
    {
        solutions_.insert(branching_scheme.root());
    }

    /** Get the branching scheme. */
    const BranchingScheme& branching_scheme() const { return branching_scheme_; }

    /** Get solutions. */
    const std::set<std::shared_ptr<Node>, SolutionPoolComparator<BranchingScheme>>& solutions() const { return solutions_; };

    /** Get the best solution of the pool. */
    const std::shared_ptr<Node>& best() const { return *solutions_.begin(); }

    /** Get the worst solution of the pool. */
    const std::shared_ptr<Node>& worst() const { return *std::prev(solutions_.end()); }

    /** Add a solution to the pool. */
    int add(
            const std::shared_ptr<Node>& node)
    {
        // If the solution is worse than the worst solution of the pool, stop.
        if ((Counter)solutions_.size() >= size_max_)
            if (!branching_scheme_.better(node, *std::prev(solutions_.end())))
                return 0;
        // If new best solution, display.
        bool d = branching_scheme_.better(node, *solutions_.begin());
        // Add new solution to solution pool.
        auto res = solutions_.insert(node);
        // If the pool size is now above its maximum allowed size, remove worst
        // solutions from it.
        if ((Counter)solutions_.size() > size_max_)
            solutions_.erase(std::prev(solutions_.end()));
        if (d)
            return 2;
        return res.second;
    }

private:

    /** Branching scheme. */
    const BranchingScheme& branching_scheme_;

    /** Maximum size of the pool. */
    Counter size_max_;

    /** Comparator. */
    SolutionPoolComparator<BranchingScheme> solution_pool_comparator_;

    /** Solutions. */
    std::set<std::shared_ptr<Node>, SolutionPoolComparator<BranchingScheme>> solutions_;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename BranchingScheme>
struct Output: optimizationtools::Output
{
    /** Constructor. */
    Output(
            const BranchingScheme& branching_scheme,
            int maximum_size_of_the_solution_pool = 10):
        solution_pool(branching_scheme, maximum_size_of_the_solution_pool) { }


    /** Solution. */
    SolutionPool<BranchingScheme> solution_pool;

    /** Elapsed time. */
    double time = 0.0;


    virtual nlohmann::json to_json() const
    {
        return {
            {"Value", solution_pool.branching_scheme().display(solution_pool.best())},
            {"Time", time}};
    }

    virtual int format_width() const { return 30; }

    virtual void format(std::ostream& os) const
    {
        int width = format_width();
        os
            << std::setw(width) << std::left << "Value: " << solution_pool.branching_scheme().display(solution_pool.best()) << std::endl
            << std::setw(width) << std::left << "Time: " << time << std::endl
            ;
    }
};

template <typename BranchingScheme>
using NewSolutionCallback = std::function<void(const Output<BranchingScheme>&)>;

template <typename BranchingScheme>
struct Parameters: optimizationtools::Parameters
{
    using Node = typename BranchingScheme::Node;

    /** Maximum size of the solution pool. */
    NodeId maximum_size_of_the_solution_pool = 1;

    /** Callback function called when a new best solution is found. */
    NewSolutionCallback<BranchingScheme> new_solution_callback = [](const Output<BranchingScheme>&) { };

    /**
     * Goal.
     *
     * If not 'nullptr', The alglorithm stops as soon as a better node is
     * found.
     */
    std::shared_ptr<Node> goal = nullptr;

    /**
     * Cutoff.
     *
     * If not 'nullptr', the algorithm won't consider worse solutions.
     */
    std::shared_ptr<Node> cutoff = nullptr;


    virtual nlohmann::json to_json() const override
    {
        nlohmann::json json = optimizationtools::Parameters::to_json();
        json.merge_patch({
                {"MaximumSizeOfTheSolutionPool", maximum_size_of_the_solution_pool},
                {"HasGoal", (goal != nullptr)},
                {"HasCutoff", (cutoff != nullptr)}});
        return json;
    }

    virtual int format_width() const override { return 23; }

    virtual void format(std::ostream& os) const override
    {
        optimizationtools::Parameters::format(os);
        int width = format_width();
        os
            << std::setw(width) << std::left << "Maximum size of the solution pool: " << maximum_size_of_the_solution_pool << std::endl
            << std::setw(width) << std::left << "Has goal: " << (goal != nullptr) << std::endl
            << std::setw(width) << std::left << "Has cutoff: " << (cutoff != nullptr) << std::endl
            ;
    }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename BranchingScheme>
using NodeMap = std::unordered_map<
        std::shared_ptr<typename BranchingScheme::Node>,
        std::vector<std::shared_ptr<typename BranchingScheme::Node>>,
        const typename BranchingScheme::NodeHasher&,
        const typename BranchingScheme::NodeHasher&>;

template <typename BranchingScheme>
using NodeSet = std::set<
        std::shared_ptr<typename BranchingScheme::Node>,
        const BranchingScheme&>;

template <typename BranchingScheme>
inline bool add_to_history_and_queue(
        const BranchingScheme& branching_scheme,
        NodeMap<BranchingScheme>& history,
        NodeSet<BranchingScheme>& q,
        const std::shared_ptr<typename BranchingScheme::Node>& node)
{
    using Node = typename BranchingScheme::Node;
    assert(node != nullptr);

    // If node is not comparable, stop.
    if (branching_scheme.comparable(node)) {
        auto& list = history[node];

        // Check if node is dominated.
        for (const std::shared_ptr<Node>& n: list)
            if (branching_scheme.dominates(n, node))
                return false;

        // Remove dominated nodes from history.
        for (auto it = list.begin(); it != list.end();) {
            if (branching_scheme.dominates(node, *it)) {
                q.erase(*it);
                *it = list.back();
                list.pop_back();
            } else {
                ++it;
            }
        }

        // Add node to history.
        list.push_back(node);
    }

    // Add to queue.
    q.insert(node);
    return true;
}

template <typename BranchingScheme>
inline void remove_from_history(
        const BranchingScheme& branching_scheme,
        NodeMap<BranchingScheme>& history,
        const std::shared_ptr<typename BranchingScheme::Node>& node)
{
    // Remove from history.
    if (branching_scheme.comparable(node)) {
        auto& list = history[node];
        for (auto it = list.begin(); it != list.end();) {
            if (*it == node) {
                *it = list.back();
                list.pop_back();
                if (list.empty())
                    history.erase(node);
                break;
            } else {
                ++it;
            }
        }
    }
}

template <typename BranchingScheme>
inline void remove_from_history_and_queue(
        const BranchingScheme& branching_scheme,
        NodeMap<BranchingScheme>& history,
        std::set<std::shared_ptr<typename BranchingScheme::Node>, const BranchingScheme&>& q,
        typename NodeSet<BranchingScheme>::const_iterator node)
{
    // Remove from history.
    remove_from_history(branching_scheme, history, *node);
    // Remove from queue.
    q.erase(node);
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////// branching_scheme_solution_write ////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename, typename T>
struct HasSolutionWriteMethod
{
    static_assert(
        std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type.");
};

template<typename C, typename Ret, typename... Args>
struct HasSolutionWriteMethod<C, Ret(Args...)>
{

private:

    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().solution_write(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:

    static constexpr bool value = type::value;

};

template<typename BranchingScheme>
void solution_write(
        const BranchingScheme&,
        const std::shared_ptr<typename BranchingScheme::Node>&,
        const std::string&,
        std::false_type)
{
}

template<typename BranchingScheme>
void solution_write(
        const BranchingScheme& branching_scheme,
        const std::shared_ptr<typename BranchingScheme::Node>& solution,
        const std::string& certificate_path,
        std::true_type)
{
    return branching_scheme.solution_write(
            solution,
            certificate_path);
}

template<typename BranchingScheme>
void solution_write(
        const BranchingScheme& branching_scheme,
        const std::shared_ptr<typename BranchingScheme::Node>& solution,
        const std::string& certificate_path)
{
    return solution_write(
            branching_scheme,
            solution,
            certificate_path,
            std::integral_constant<
                bool,
                HasSolutionWriteMethod<BranchingScheme,
                void(const BranchingScheme&, const std::shared_ptr<typename BranchingScheme::Node>&, const std::string&)>::value>());
}

}

