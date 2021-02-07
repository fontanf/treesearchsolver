#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"

#include <cstdint>
#include <set>

namespace treesearchsolver
{

typedef int64_t NodeId;
typedef int64_t Counter;
typedef double Value;

enum class ObjectiveSense { Min, Max };

template <typename BranchingScheme>
struct SolutionPoolComparator
{
    typedef typename BranchingScheme::Node Node;

    SolutionPoolComparator(const BranchingScheme& branching_scheme):
        branching_scheme(branching_scheme) {  }

    const BranchingScheme& branching_scheme;

    bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const {
        if (branching_scheme.better(node_1, node_2))
            return true;
        if (branching_scheme.better(node_1, node_2))
            return false;
        if (branching_scheme.equals(node_1, node_2))
            return false;
        return node_1.get() < node_2.get();
    }
};

template <typename BranchingScheme>
class SolutionPool
{
    typedef typename BranchingScheme::Node Node;

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
            info.output->sol_number++;
            double t = info.elapsed_time();
            std::string sol_str = "Solution" + std::to_string(info.output->sol_number);
            PUT(info, sol_str, "Value", branching_scheme_.display(node));
            PUT(info, sol_str, "Time", t);
            PUT(info, sol_str, "Comment", ss.str());
            display(ss, info);
            if (!info.output->onlywriteattheend) {
                info.write_ini();
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
        VER(info, "----------------------------------------------------------------------" << std::endl);
        VER(info, std::left << std::setw(6) << "Sol");
        VER(info, std::left << std::setw(16) << "Time");
        VER(info, std::left << std::setw(16) << "Value");
        VER(info, std::left << std::setw(32) << "Comment");
        VER(info, std::endl);
        VER(info, "----------------------------------------------------------------------" << std::endl);
    }

    void display(const std::stringstream& ss, optimizationtools::Info& info)
    {
        double t = info.elapsed_time();
        VER(info, std::left << std::setw(6) << info.output->sol_number);
        VER(info, std::left << std::setw(16) << t);
        VER(info, std::left << std::setw(16) << branching_scheme_.display(best()));
        VER(info, std::left << std::setw(32) << ss.str());
        VER(info, std::endl);
    }

    void display_end(optimizationtools::Info& info)
    {
        double t = info.elapsed_time();
        std::string sol_str = "Solution";
        VER(info, "---" << std::endl);
        PUT(info, sol_str, "Time", t);
        PUT(info, sol_str, "Value", branching_scheme_.display(*solutions_.begin()));
        VER(info, "Time: " << t << std::endl);
        VER(info, "Value: " << branching_scheme_.display(*solutions_.begin()) << std::endl);
        info.write_ini();
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
    typedef typename BranchingScheme::Node Node;

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
                *it = list.back();
                list.pop_back();
                q.erase(*it);
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

