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

#pragma once

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
        ItemId item_id = -1; // Last added item.
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

    BranchingScheme(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_items_(instance.number_of_items() + 1),
        generator_(0)
    {
        // Initialize sorted_items_.
        for (ItemId item_id = 0; item_id < instance_.number_of_items() + 1; ++item_id) {
            sorted_items_[item_id].reset(instance.number_of_items());
            LocationId location_id = (item_id == instance_.number_of_items())?
                0: instance_.item(item_id).location_id;
            for (LocationId item_id_2 = 0; item_id_2 < instance_.number_of_items(); ++item_id_2) {
                LocationId location_id_2 = instance_.item(item_id_2).location_id;
                double c;
                switch (parameters.guide_id) {
                case 0: {
                    c = std::pow(instance_.duration(location_id, location_id_2, instance_.capacity() / 2), parameters_.exponent_time)
                        * std::pow(instance_.item(item_id_2).weight, parameters_.exponent_weight)
                        / std::pow(instance_.item(item_id_2).profit, parameters_.exponent_profit);
                    break;
                } default: {
                    c = instance_.duration(location_id, location_id_2, instance_.capacity() / 2)
                        * instance_.item(item_id_2).weight
                        / instance_.item(item_id_2).profit;
                }
                }
                if (item_id == item_id_2)
                    c = std::numeric_limits<double>::max();
                sorted_items_[item_id].set_cost(item_id_2, c);
            }
        }
    }

    inline LocationId neighbor(
            ItemId item_id,
            ItemPos pos) const
    {
        assert(item_id < instance_.number_of_items() + 1);
        assert(pos < instance_.number_of_items());
        return sorted_items_[item_id].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_items.resize(instance_.number_of_items(), true);
        r->item_id = instance_.number_of_items();
        r->guide = 0;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        ItemId item_id_next = neighbor(father->item_id, father->next_child_pos);
        // Update father
        father->next_child_pos++;
        // Check item availibility.
        if (!father->available_items[item_id_next])
            return nullptr;
        // Check capacity.
        if (father->weight + instance_.item(item_id_next).weight > instance_.capacity())
            return nullptr;
        // Check time limit.
        LocationId location_id = (father->father == nullptr)?
            0: instance_.item(father->item_id).location_id;
        LocationId location_id_next = instance_.item(item_id_next).location_id;
        Time t = instance_.duration(location_id, location_id_next, father->weight);
        Time t_end = instance_.duration(location_id_next, instance_.number_of_locations() - 1,
                father->weight + instance_.item(item_id_next).weight);
        if (father->time + t + t_end > instance_.time_limit())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->available_items = father->available_items;
        child->available_items[item_id_next] = false;
        child->item_id = item_id_next;
        child->number_of_items = father->number_of_items + 1;
        child->number_of_locations = father->number_of_locations;
        if (location_id_next != location_id) {
            for (ItemId item_id_tmp: instance_.location(location_id).item_ids)
                child->available_items[item_id_tmp] = false;
            child->number_of_locations++;
        }
        child->time = father->time + t;
        child->profit = father->profit + instance_.item(item_id_next).profit;
        child->weight = father->weight + instance_.item(item_id_next).weight;
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
            if (branching_scheme_.instance().item(node_1->item_id).location_id
                    != branching_scheme_.instance().item(node_2->item_id).location_id)
                return false;
            if (node_1->available_items != node_2->available_items)
                return false;
            return true;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher_1(branching_scheme_.instance().item(node->item_id).location_id);
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
        LocationId location_id = instance_.item(node->item_id).location_id;
        std::stringstream ss;
        ss << node->profit
            << " (n" << node->number_of_locations
            << " m" << node->number_of_items
            << " w" << std::round(100 * (double)node->weight / instance_.capacity()) / 100
            << " t" << std::round(100 * (node->time + instance_.duration(location_id, instance_.number_of_locations() - 1, node->weight)) / instance_.time_limit()) / 100
            << ")";
        return ss.str();
    }

    std::ostream& print_solution(
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
                << " i " << node_tmp->item_id
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
            items.push_back(node_tmp->item_id);
            node_tmp = node_tmp->father;
        }
        for (thieforienteering::ItemId item_id: items)
            cert << item_id << " ";
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

