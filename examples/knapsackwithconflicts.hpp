#pragma once

/**
 * Knapsack Problem with Conflicts.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/knapsackwithconflicts.hpp
 *
 * Branching scheme:
 * - Root node: empty solution, no item
 * - Children: add a new item in the knapsack, i.e. create one child for each
 *   valid item.
 * - Dominance: if two nodes node_1 and node_2 have the same available items
 *   left and:
 *   - profit(node_1) >= profit(node_2)
 *   - weight(node_1) <= weight(node_2)
 *   then node_1 dominates node_2
 *
 */

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

#include "orproblems/knapsackwithconflicts.hpp"

namespace treesearchsolver
{

namespace knapsackwithconflicts
{

using namespace orproblems::knapsackwithconflicts;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Parameters
    {
        GuideId guide_id = 0;
    };

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_items;
        ItemId j = -1;
        ItemPos j_pos = -1;
        ItemId number_of_items = 0;
        ItemId number_of_remaining_items = -1;
        Profit remaining_weight = 0;
        Profit remaining_profit = 0;
        Weight weight = 0;
        Profit profit = 0;
        double guide = 0;
        ItemPos next_child_pos = 0;
    };

    BranchingScheme(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters) { }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_items.resize(instance_.number_of_items(), true);
        r->number_of_remaining_items = instance_.number_of_items();
        for (ItemId j = 0; j < instance_.number_of_items(); ++j) {
            r->remaining_weight += instance_.item(j).weight;
            r->remaining_profit += instance_.item(j).profit;
        }
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        ItemId j_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;

        if (!father->available_items[j_next])
            return nullptr;
        if (father->weight + instance_.item(j_next).weight > instance_.capacity())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->j = j_next;
        child->j_pos = father->next_child_pos - 1;
        child->number_of_items = father->number_of_items + 1;
        child->available_items = father->available_items;
        child->available_items[j_next] = false;
        child->number_of_remaining_items = father->number_of_remaining_items - 1;
        child->remaining_weight = father->remaining_weight - instance_.item(j_next).weight;
        child->remaining_profit = father->remaining_profit - instance_.item(j_next).profit;
        for (ItemId j: instance_.item(j_next).neighbors) {
            if (child->available_items[j]) {
                child->available_items[j] = false;
                child->number_of_remaining_items--;
                child->remaining_weight -= instance_.item(j).weight;
                child->remaining_profit -= instance_.item(j).profit;
            }
        }
        child->weight = father->weight + instance_.item(j_next).weight;
        child->profit = father->profit + instance_.item(j_next).profit;
        child->guide =
            (parameters_.guide_id == 0)? (double)child->weight / child->profit:
            (parameters_.guide_id == 1)? (double)child->weight / child->profit / child->remaining_profit:
                                         (double)1.0 / (child->profit + child->remaining_profit);
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        return (node->next_child_pos == instance_.number_of_items());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_items != node_2->number_of_items)
            return node_1->number_of_items < node_2->number_of_items;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->number_of_items == instance_.number_of_items();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        return node_1->profit + node_1->remaining_profit <= node_2->profit;
    }

    /*
     * Solution pool.
     */

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        return node_1->profit > node_2->profit;
    }

    std::shared_ptr<Node> cutoff(double value) const
    {
        auto node = std::shared_ptr<Node>(new BranchingScheme::Node());
        node->profit = value;
        return node;
    }

    bool equals(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_items != node_2->number_of_items)
            return false;
        std::vector<bool> v(instance_.number_of_items(), false);
        for (auto node_tmp = node_1; node_tmp->father != nullptr; node_tmp = node_tmp->father)
            v[node_tmp->j] = true;
        for (auto node_tmp = node_1; node_tmp->father != nullptr; node_tmp = node_tmp->father)
            if (!v[node_tmp->j])
                return false;
        return true;
    }

    std::string display(const std::shared_ptr<Node>& node) const
    {
        std::stringstream ss;
        ss << node->profit
            << " (n" << node->number_of_items << "/" << instance_.number_of_items()
            << " w" << node->weight << "/" << instance_.capacity()
            << ")";
        return ss.str();
    }

    /**
     * Dominances.
     */

    inline bool comparable(
            const std::shared_ptr<Node>&) const
    {
        return true;
    }

    struct NodeHasher
    {
        std::hash<std::vector<bool>> hasher;

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            return node_1->available_items == node_2->available_items;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher(node->available_items);
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->profit >= node_2->profit
                && node_1->weight <= node_2->weight)
            return true;
        return false;
    }

    /*
     * Outputs.
     */

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr; node_tmp = node_tmp->father)
            os << "j " << node_tmp->j
                << " wj " << instance_.item(node_tmp->j).weight
                << " pj " << instance_.item(node_tmp->j).profit
                << std::endl;
        return os;
    }

    inline void write(
            const std::shared_ptr<Node>& node,
            std::string certificate_path) const
    {
        if (certificate_path.empty())
            return;
        std::ofstream cert(certificate_path);
        if (!cert.good()) {
            throw std::runtime_error(
                    "Unable to open file \"" + certificate_path + "\".");
        }

        std::vector<ItemId> items;
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            items.push_back(node_tmp->j);
        std::reverse(items.begin(), items.end());
        for (ItemId j: items)
            cert << j << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

};

}

}

