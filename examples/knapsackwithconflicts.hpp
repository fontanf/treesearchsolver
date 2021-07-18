#pragma once

/**
 * Knapsack Problem with Conflicts.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/knapsackwithconflicts.hpp
 *
 * TODO
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

typedef int64_t GuideId;

class BranchingScheme
{

public:

    struct Parameters
    {
        GuideId guide_id = 0;
        bool force_order = false;
    };

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_items;
        ItemId j = -1;
        ItemPos j_pos = -1;
        ItemId item_number = 0;
        Weight weight = 0;
        Profit profit = 0;
        Profit bound = -1;
        double guide = 0;
        ItemPos next_child_pos = 0;
    };

    BranchingScheme(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_items_(instance.item_number())
    {
        // Initialize sorted_items_.
        std::iota(sorted_items_.begin(), sorted_items_.end(), 0);
        sort(sorted_items_.begin(), sorted_items_.end(),
                [&instance](ItemId j1, ItemId j2) -> bool
                {
                    return instance.item(j1).profit / instance.item(j1).weight
                        > instance.item(j2).profit / instance.item(j2).weight;
                });
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_items.resize(instance_.item_number(), true);
        ItemId j = sorted_items_[0];
        r->bound = std::floor(instance_.item(j).profit / instance_.item(j).weight
                * instance_.capacity());
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        ItemId j_next = sorted_items_[father->next_child_pos];
        // Update father
        father->next_child_pos++;
        //std::cout << "father"
        //    << " j " << father->j
        //    << " j_pos " << father->j_pos
        //    << " w " << father->weight
        //    << " p " << father->profit
        //    << " nc " << father->next_child_pos << "/" << instance_.item_number()
        //    << std::endl;

        if (!father->available_items[j_next])
            return nullptr;
        if (father->weight + instance_.item(j_next).weight > instance_.capacity())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->j = j_next;
        child->j_pos = father->next_child_pos - 1;
        child->item_number = father->item_number + 1;
        child->available_items = father->available_items;
        child->available_items[j_next] = false;
        for (ItemId j: instance_.item(j_next).neighbors)
            child->available_items[j] = false;
        child->weight = father->weight + instance_.item(j_next).weight;
        child->profit = father->profit + instance_.item(j_next).profit;
        child->guide = (double)child->weight / child->profit;
        if (!parameters_.force_order) {
            child->bound = child->profit
                + std::floor(instance_.item(sorted_items_[0]).profit
                        / instance_.item(sorted_items_[0]).weight
                        * (instance_.capacity() - child->weight));
        } else {
            for (ItemId j_pos = father->j_pos + 1; j_pos < child->j_pos; ++j_pos)
                child->available_items[sorted_items_[j_pos]] = false;
            child->next_child_pos = father->next_child_pos;
            child->bound = child->profit
                + std::floor(instance_.item(j_next).profit
                        / instance_.item(j_next).weight
                        * (instance_.capacity() - child->weight));
        }
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == instance_.item_number());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(node_1 != nullptr);
        assert(node_2 != nullptr);
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->item_number != node_2->item_number)
            return node_1->item_number < node_2->item_number;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->next_child_pos == instance_.item_number()
           || node->item_number == instance_.item_number();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        return node_1->bound <= node_2->profit;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        return node_1->profit > node_2->profit;
    }

    bool equals(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->item_number != node_2->item_number)
            return false;
        std::vector<bool> v(instance_.item_number(), false);
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
            << " (n" << node->item_number << "/" << instance_.item_number()
            << " w" << node->weight << "/" << instance_.capacity()
            << ")";
        return ss.str();
    }

    /**
     * Dominances.
     */

    inline bool comparable(
            const std::shared_ptr<Node>& node) const
    {
        (void)node;
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
            std::string filepath) const
    {
        if (filepath.empty())
            return;
        std::ofstream cert(filepath);
        if (!cert.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
            return;
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
    const Parameters& parameters_;

    mutable std::vector<ItemId> sorted_items_;

};

}

}

