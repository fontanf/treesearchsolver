/**
 * Knapsack problem with conflicts
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

#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

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
        ItemId item_id = -1;
        ItemPos item_pos = -1;
        ItemId number_of_items = 0;
        ItemId number_of_remaining_items = -1;
        Profit remaining_weight = 0;
        Profit remaining_profit = 0;
        Weight weight = 0;
        Profit profit = 0;
        double guide = 0;
        ItemPos next_child_pos = 0;
    };

    BranchingScheme(
            const Instance& instance,
            const Parameters& parameters):
        instance_(instance),
        parameters_(parameters) { }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_items.resize(instance_.number_of_items(), true);
        r->number_of_remaining_items = instance_.number_of_items();
        for (ItemId item_id = 0;
                item_id < instance_.number_of_items();
                ++item_id) {
            r->remaining_weight += instance_.item(item_id).weight;
            r->remaining_profit += instance_.item(item_id).profit;
        }
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        ItemId item_id_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;

        if (!father->available_items[item_id_next])
            return nullptr;
        if (father->weight + instance_.item(item_id_next).weight > instance_.capacity())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->item_id = item_id_next;
        child->item_pos = father->next_child_pos - 1;
        child->number_of_items = father->number_of_items + 1;
        child->available_items = father->available_items;
        child->available_items[item_id_next] = false;
        child->number_of_remaining_items = father->number_of_remaining_items - 1;
        child->remaining_weight = father->remaining_weight - instance_.item(item_id_next).weight;
        child->remaining_profit = father->remaining_profit - instance_.item(item_id_next).profit;
        for (ItemId item_id: instance_.item(item_id_next).neighbors) {
            if (child->available_items[item_id]) {
                child->available_items[item_id] = false;
                child->number_of_remaining_items--;
                child->remaining_weight -= instance_.item(item_id).weight;
                child->remaining_profit -= instance_.item(item_id).profit;
            }
        }
        child->weight = father->weight + instance_.item(item_id_next).weight;
        child->profit = father->profit + instance_.item(item_id_next).profit;
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

    std::shared_ptr<Node> goal_node(double value) const
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
            v[node_tmp->item_id] = true;
        for (auto node_tmp = node_1; node_tmp->father != nullptr; node_tmp = node_tmp->father)
            if (!v[node_tmp->item_id])
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

    /*
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

    std::ostream& print_solution(
            const std::shared_ptr<Node>& node,
            std::ostream &os,
            int verbosity_level)
    {
        if (verbosity_level >= 1) {
            os << "Profit:            " << node->profit << std::endl;
            os << "Weight:            " << node->weight << " / " << instance_.capacity() << std::endl;
            os << "Number of items:   " << node->number_of_items << " / " << instance_.number_of_items() << std::endl;
        }
        if (verbosity_level >= 2) {
            os << std::endl
                << std::setw(12) << "Item"
                << std::setw(12) << "Profit"
                << std::setw(12) << "Weight"
                << std::setw(12) << "Efficiency"
                << std::setw(12) << "# conflicts"
                << std::endl
                << std::setw(12) << "----"
                << std::setw(12) << "------"
                << std::setw(12) << "------"
                << std::setw(12) << "----------"
                << std::setw(12) << "-----------"
                << std::endl;
            for (auto node_tmp = node;
                    node_tmp->father != nullptr;
                    node_tmp = node_tmp->father) {
                ItemId item_id = node_tmp->item_id;
                const Item& item = instance_.item(item_id);
                os
                    << std::setw(12) << item_id
                    << std::setw(12) << item.profit
                    << std::setw(12) << item.weight
                    << std::setw(12) << (double)item.profit / item.weight
                    << std::setw(12) << item.neighbors.size()
                    << std::endl;
            }
        }
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
            items.push_back(node_tmp->item_id);
        std::reverse(items.begin(), items.end());
        for (ItemId item_id: items)
            cert << item_id << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

};

}

}

