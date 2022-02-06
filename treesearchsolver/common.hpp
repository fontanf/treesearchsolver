#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"

#include <cstdint>
#include <set>

namespace treesearchsolver
{

using NodeId = int64_t;
using Counter = int64_t;
using Value = double;

enum class ObjectiveSense { Min, Max };


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// cutoff ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename, typename T>
struct HasCutoffMethod
{
    static_assert(
        std::integral_constant<T, false>::value,
        "Second template parameter needs to be of function type.");
};

template<typename C, typename Ret, typename... Args>
struct HasCutoffMethod<C, Ret(Args...)>
{

private:

    template<typename T>
    static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().cutoff(std::declval<Args>()...)), Ret>::type;

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:

    static constexpr bool value = type::value;

};

template<typename BranchingScheme>
std::shared_ptr<typename BranchingScheme::Node> cutoff(
        const BranchingScheme&,
        double,
        std::false_type)
{
    return nullptr;
}

template<typename BranchingScheme>
std::shared_ptr<typename BranchingScheme::Node> cutoff(
        const BranchingScheme& branching_scheme,
        double value,
        std::true_type)
{
    return branching_scheme.cutoff(value);
}

template<typename BranchingScheme>
std::shared_ptr<typename BranchingScheme::Node> cutoff(
        const BranchingScheme& branching_scheme,
        double value)
{
    return cutoff(
            branching_scheme,
            value,
            std::integral_constant<
                bool,
                HasCutoffMethod<BranchingScheme,
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

template <typename BranchingScheme>
class SolutionPool
{
    using Node = typename BranchingScheme::Node;

public:

    SolutionPool(const BranchingScheme& branching_scheme, Counter size_max):
        branching_scheme_(branching_scheme),
        size_max_(size_max),
        solution_pool_comparator_(branching_scheme),
        solutions_(solution_pool_comparator_)
    {
        solutions_.insert(branching_scheme.root());
    }

    virtual ~SolutionPool() { }

    const std::set<std::shared_ptr<Node>, SolutionPoolComparator<BranchingScheme>>& solutions() const { return solutions_; };
    const std::shared_ptr<Node>& best() { return *solutions_.begin(); }
    const std::shared_ptr<Node>& worst() { return *std::prev(solutions_.end()); }

    bool add(
            const std::shared_ptr<Node>& node,
            const std::stringstream& ss,
            optimizationtools::Info& info)
    {
        // If the solution is worse than the worst solution of the pool, stop.
        if ((Counter)solutions_.size() >= size_max_)
            if (!branching_scheme_.better(node, *std::prev(solutions_.end())))
                return false;
        // If new best solution, display.
        bool d = branching_scheme_.better(node, *solutions_.begin());
        // Add new solution to solution pool.
        auto res = solutions_.insert(node);
        if (d) {
            info.output->number_of_solutions++;
            double t = info.elapsed_time();
            std::string sol_str = "Solution" + std::to_string(info.output->number_of_solutions);
            PUT(info, sol_str, "Value", branching_scheme_.display(node));
            PUT(info, sol_str, "Time", t);
            PUT(info, sol_str, "Comment", ss.str());
            if (!info.output->only_write_at_the_end) {
                info.write_json_output();
                //write(info);
            }
        }
        // If the pool size is now above its maximum allowed size, remove worst
        // solutions from it.
        if ((Counter)solutions_.size() > size_max_)
            solutions_.erase(std::prev(solutions_.end()));
        return res.second;
    }

    void display_init(optimizationtools::Info& info)
    {
        VER(info,
                std::setw(11) << "Time"
                << std::setw(32) << "Value"
                << std::setw(32) << "Comment" << std::endl
                << std::setw(11) << "----"
                << std::setw(32) << "-----"
                << std::setw(32) << "-------" << std::endl);
    }

    void display(const std::stringstream& ss, optimizationtools::Info& info)
    {
        double t = info.elapsed_time();
        std::streamsize precision = std::cout.precision();
        VER(info,
                std::setw(11) << std::fixed << std::setprecision(3) << t << std::defaultfloat << std::setprecision(precision)
                << std::setw(32) << branching_scheme_.display(best())
                << std::setw(32) << ss.str()
                << std::endl);
    }

    void display_end(optimizationtools::Info& info)
    {
        double t = info.elapsed_time();
        VER(info, std::defaultfloat
                << std::endl
                << "Final statistics" << std::endl
                << "----------------" << std::endl
                << "Value:                      " << branching_scheme_.display(*solutions_.begin()) << std::endl
                << "Time:                       " << t << std::endl);

        std::string sol_str = "Solution";
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "Value", branching_scheme_.display(*solutions_.begin()));
        info.write_json_output();
        //write(info);
    }

private:

    const BranchingScheme& branching_scheme_;
    Counter size_max_;
    SolutionPoolComparator<BranchingScheme> solution_pool_comparator_;
    std::set<std::shared_ptr<Node>, SolutionPoolComparator<BranchingScheme>> solutions_;

};

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

}

