#pragma once

/**
 * Thief orienterring problem.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/thieforienteering.hpp
 *
 * Tree search
 * - forward branching on the next item
 * - guide: time^exponent_time * weight^exponent_weight / profit^exponent_profit
 * - no bound
 */

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"

#include "orproblems/thieforienteering.hpp"

namespace treesearchsolver
{

namespace thieforienteering
{

using namespace orproblems::thieforienteering;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_items;
        ItemId i = -1; // Last added item.
        ItemId number_of_items = 0;
        LocationId number_of_locations = 0;
        Time time = 0;
        Profit profit = 0;
        Weight weight = 0;
        double guide = 0;
        ItemPos next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 0;
        double exponent_time = 1;
        double exponent_weight = 1;
        double exponent_profit = 1;
    };

    BranchingScheme(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_items_(instance.number_of_items() + 1),
        generator_(0)
    {
        // Initialize sorted_items_.
        for (ItemId i = 0; i < instance_.number_of_items() + 1; ++i) {
            sorted_items_[i].reset(instance.number_of_items());
            LocationId j = (i == instance_.number_of_items())?
                0: instance_.item(i).location;
            for (LocationId i2 = 0; i2 < instance_.number_of_items(); ++i2) {
                LocationId j2 = instance_.item(i2).location;
                double c;
                switch (parameters.guide_id) {
                case 0: {
                    c = std::pow(instance_.duration(j, j2, instance_.capacity() / 2), parameters_.exponent_time)
                        * std::pow(instance_.item(i2).weight, parameters_.exponent_weight)
                        / std::pow(instance_.item(i2).profit, parameters_.exponent_profit);
                    break;
                } default: {
                    c = instance_.duration(j, j2, instance_.capacity() / 2)
                        * instance_.item(i2).weight
                        / instance_.item(i2).profit;
                }
                }
                if (i == i2)
                    c = std::numeric_limits<double>::max();
                sorted_items_[i].set_cost(i2, c);
            }
        }
    }

    inline LocationId neighbor(ItemId i, ItemPos pos) const
    {
        assert(i < instance_.number_of_items() + 1);
        assert(pos < instance_.number_of_items());
        return sorted_items_[i].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_items.resize(instance_.number_of_items(), true);
        r->i = instance_.number_of_items();
        r->guide = 0;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        ItemId i_next = neighbor(father->i, father->next_child_pos);
        // Update father
        father->next_child_pos++;
        // Check item availibility.
        if (!father->available_items[i_next])
            return nullptr;
        // Check capacity.
        if (father->weight + instance_.item(i_next).weight > instance_.capacity())
            return nullptr;
        // Check time limit.
        LocationId j = (father->father == nullptr)?
            0: instance_.item(father->i).location;
        LocationId j_next = instance_.item(i_next).location;
        Time t = instance_.duration(j, j_next, father->weight);
        Time t_end = instance_.duration(j_next, instance_.number_of_locations() - 1,
                father->weight + instance_.item(i_next).weight);
        if (father->time + t + t_end > instance_.time_limit())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->available_items = father->available_items;
        child->available_items[i_next] = false;
        child->i = i_next;
        child->number_of_items = father->number_of_items + 1;
        child->number_of_locations = father->number_of_locations;
        if (j_next != j) {
            for (ItemId i_tmp: instance_.location(j).items)
                child->available_items[i_tmp] = false;
            child->number_of_locations++;
        }
        child->time = father->time + t;
        child->profit = father->profit + instance_.item(i_next).profit;
        child->weight = father->weight + instance_.item(i_next).weight;
        switch (parameters_.guide_id) {
        case 0: {
            child->guide = std::pow(child->time, parameters_.exponent_time)
                * std::pow(child->weight, parameters_.exponent_weight)
                / std::pow(child->profit, parameters_.exponent_profit);
            break;
        } default: {
        }
        }
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == instance_.number_of_items() - 1);
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        //if (node_1->number_of_items != node_2->number_of_items)
        //    return node_1->number_of_items < node_2->number_of_items;
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
        (void)node_1;
        (void)node_2;
        return false;
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

    bool equals(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        return node_1->available_items == node_2->available_items;
    }

    /*
     * Dominances.
     */

    inline bool comparable(
            const std::shared_ptr<Node>& node) const
    {
        (void)node;
        return true;
    }

    const Instance& instance() const { return instance_; }

    struct NodeHasher
    {
        const BranchingScheme& branching_scheme_;
        std::hash<LocationId> hasher_1;
        std::hash<std::vector<bool>> hasher_2;

        NodeHasher(const BranchingScheme& branching_scheme):
            branching_scheme_(branching_scheme) { }

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (branching_scheme_.instance().item(node_1->i).location
                    != branching_scheme_.instance().item(node_2->i).location)
                return false;
            if (node_1->available_items != node_2->available_items)
                return false;
            return true;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher_1(branching_scheme_.instance().item(node->i).location);
            optimizationtools::hash_combine(hash, hasher_2(node->available_items));
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(*this); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->time <= node_2->time
                && node_1->profit >= node_2->profit
                && node_1->weight <= node_2->weight)
            return true;
        return false;
    }

    /*
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        LocationId j = instance_.item(node->i).location;
        std::stringstream ss;
        ss << node->profit
            << " (n" << node->number_of_locations
            << " m" << node->number_of_items
            << " w" << std::round(100 * (double)node->weight / instance_.capacity()) / 100
            << " t" << std::round(100 * (node->time + instance_.duration(j, instance_.number_of_locations() - 1, node->weight)) / instance_.time_limit()) / 100
            << ")";
        return ss.str();
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->number_of_locations
                << " m " << node_tmp->number_of_items
                << " t " << node_tmp->time
                << " w " << node_tmp->weight
                << " p " << node_tmp->profit
                << " guide " << node_tmp->guide
                << " i " << node_tmp->i
                << std::endl;
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

        std::vector<thieforienteering::ItemId> items;
        auto node_tmp = node;
        while (node_tmp->father != nullptr) {
            items.push_back(node_tmp->i);
            node_tmp = node_tmp->father;
        }
        for (thieforienteering::ItemId i: items)
            cert << i << " ";
        cert << std::endl;
        cert.close();
    }


private:

    const Instance& instance_;
    Parameters parameters_;

    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_items_;
    mutable std::mt19937_64 generator_;

};

}

}

