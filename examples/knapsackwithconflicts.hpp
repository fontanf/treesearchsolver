#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"

/**
 * Knapsack Problem with Conflicts.
 *
 * Input:
 * - a knapsack with capacity C
 * - n items with weight wⱼ and profit pⱼ (j = 1..n)
 * - a graph G such that each node corresponds to an item
 * Problem:
 * - Select a subset of items such that:
 *   - the total weight of the selected items does not exceed the knapsack
 *     capacity
 *   - if there exists an edge between j₁ to j₂ in G, then j₁ and j₂ must not
 *     be both selected
 * Objective:
 * - Maximize the total profit of the selected items
 *
 */

namespace treesearchsolver
{

namespace knapsackwithconflicts
{

typedef int64_t ItemId;
typedef int64_t ItemPos;
typedef int64_t Weight;
typedef double Profit;
typedef int64_t GuideId;

struct Item
{
    ItemId id;
    Weight weight;
    Profit profit;
    std::vector<ItemId> neighbors;
};

class Instance
{

public:

    Instance() { }
    void add_item(Weight w, Profit p)
    {
        Item item;
        item.id = items_.size();
        item.weight = w;
        item.profit = p;
        items_.push_back(item);
    }
    void add_conflict(ItemId j1, ItemId j2)
    {
        assert(j1 >= 0);
        assert(j2 >= 0);
        assert(j1 < item_number());
        assert(j2 < item_number());
        items_[j1].neighbors.push_back(j2);
        items_[j2].neighbors.push_back(j1);
    }
    void set_capacity(Weight capacity) { capacity_ = capacity; }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "default") {
            read_default(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline ItemId item_number() const { return items_.size(); }
    inline const Item& item(ItemId j) const { return items_[j]; }
    inline Weight capacity() const { return capacity_; }

private:

    void read_default(std::ifstream& file)
    {
        ItemId n = -1;
        Weight c = -1;
        std::string tmp;
        file >> tmp >> tmp >> tmp >> n >> tmp;
        file >> tmp >> tmp >> tmp >> c >> tmp;
        set_capacity(c);
        if (tmp == ";")
            file >> tmp;
        file >> tmp >> tmp >> tmp >> tmp >> tmp >> tmp;

        Weight w = -1;
        Profit p = -1;
        for (ItemId j = 0; j < n; ++j) {
            file >> tmp >> p >> w;
            add_item(w, p);
        }

        file >> tmp >> tmp >> tmp >> tmp;
        ItemId j1 = -1;
        ItemId j2 = -1;
        for (;;) {
            file >> j1 >> j2;
            if (!file)
                break;
            add_conflict(j1, j2);
        }
    }

    std::vector<Item> items_;
    Weight capacity_ = 0;

};

static std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "capacity " << instance.capacity() << std::endl;
    os << "item number " << instance.item_number() << std::endl;
    for (ItemId j = 0; j < instance.item_number(); ++j) {
        os << "item " << j
            << " w " << instance.item(j).weight
            << " p " << instance.item(j).profit
            << " neighbors";
        for (ItemId j2: instance.item(j).neighbors)
            os << " " << j2;
        os << std::endl;
    }
    return os;
}

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

private:

    const Instance& instance_;
    const Parameters& parameters_;

    mutable std::vector<ItemId> sorted_items_;

};

}

}

