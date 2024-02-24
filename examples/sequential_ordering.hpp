/**
 * Sequential ordering problem
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/sequential_ordering.hpp
 *
 * Tree search:
 * - forward branching
 * - guide: current length + distance to the closest next child
 *
 */

#pragma once

#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"

#include "orproblems/scheduling/sequential_ordering.hpp"

#include <memory>

namespace treesearchsolver
{
namespace sequential_ordering
{

using namespace orproblems::sequential_ordering;

class BranchingScheme
{

public:

    struct Node
    {
        /** Parent node. */
        std::shared_ptr<Node> parent = nullptr;

        /** Array indicating for each vertex, if it has been visited. */
        std::vector<bool> visited;

        /** Last visited vertex. */
        LocationId last_location_id = 0;

        /** Number of visited locations. */
        LocationId number_of_locations = 1;

        /** Length of the partial solution. */
        Distance length = 0;

        /**
         * Sum of, for each unvisited vertex, the distance to its clostest
         * neighbor.
         *
         * This is used to compute the outgoing bound efficiently.
         */
        Distance bound_outgoing = 0;

        /** Bound. */
        Distance bound = 0;

        /** Guide. */
        Distance guide = 0;

        /** Next child to generate. */
        LocationPos next_child_pos = 0;
    };

    BranchingScheme(
            const Instance& instance):
        instance_(instance),
        sorted_locations_(instance.number_of_locations()),
        generator_(0)
    {
        // Initialize sorted_locations_.
        for (LocationId location_id = 0;
                location_id < instance_.number_of_locations();
                ++location_id) {
            sorted_locations_[location_id].reset(instance.number_of_locations());
            for (LocationId location_id_2 = 0;
                    location_id_2 < instance_.number_of_locations();
                    ++location_id_2) {
                sorted_locations_[location_id].set_cost(
                        location_id_2,
                        instance_.distance(location_id, location_id_2));
            }
        }
    }

    inline LocationId neighbor(
            LocationId location_id,
            LocationPos pos) const
    {
        return sorted_locations_[location_id].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->visited.resize(instance_.number_of_locations(), false);

        // Bound.
        r->bound_outgoing = 0;
        for (LocationId location_id = 0;
                location_id < instance_.number_of_locations();
                ++location_id) {
            Distance d = instance_.distance(location_id, neighbor(location_id, 0));
            if (d != std::numeric_limits<Distance>::max()) {
                r->bound_outgoing += instance_.distance(
                        location_id,
                        neighbor(location_id, 0));
            }
        }
        r->bound = r->bound_outgoing;

        r->guide = r->bound;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& parent) const
    {
        assert(!infertile(parent));
        assert(!leaf(parent));

        // Get the next vertex to visit.
        LocationId location_id_next = neighbor(parent->last_location_id, parent->next_child_pos);
        Distance d = instance_.distance(parent->last_location_id, location_id_next);

        // Update parent
        parent->next_child_pos++;
        Distance d_next = instance_.distance(
                parent->last_location_id,
                neighbor(parent->last_location_id, parent->next_child_pos));
        if (d_next == std::numeric_limits<Distance>::max()) {
            parent->guide = -1;
        } else {
            parent->bound = parent->bound - d + d_next;
            parent->guide = parent->bound;
        }

        // Check if the vertex has already been visited.
        if (parent->visited[location_id_next]
                || d == std::numeric_limits<Distance>::max())
            return nullptr;

        // Check if the predecessors of the location have already been visited.
        for (LocationId location_id_pred: instance_.predecessors(location_id_next)) {
            if (location_id_pred != parent->last_location_id
                    && !parent->visited[location_id_pred]) {
                return nullptr;
            }
        }

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->parent = parent;
        child->visited = parent->visited;
        child->visited[parent->last_location_id] = true;
        child->last_location_id = location_id_next;
        child->number_of_locations = parent->number_of_locations + 1;
        child->length = parent->length + d;
        child->bound_outgoing = child->parent->bound_outgoing
            - instance_.distance(
                    parent->last_location_id,
                    neighbor(parent->last_location_id, 0));
        child->bound = child->length + child->bound_outgoing;
        child->guide = child->bound;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->guide == -1);
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->number_of_locations == instance_.number_of_locations();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->number_of_locations != instance_.number_of_locations())
            return false;
        return node_1->bound >= node_2->length;
    }

    /*
     * Solution pool.
     */

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_locations < instance_.number_of_locations())
            return false;
        if (node_2->number_of_locations < instance_.number_of_locations())
            return true;
        return node_1->length < node_2->length;
    }

    bool equals(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        (void)node_1;
        (void)node_2;
        return false;
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
        std::hash<LocationId> hasher_1;
        std::hash<std::vector<bool>> hasher_2;

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (node_1->last_location_id != node_2->last_location_id)
                return false;
            return node_1->visited == node_2->visited;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            assert(node != nullptr);
            size_t hash = hasher_1(node->last_location_id);
            optimizationtools::hash_combine(hash, hasher_2(node->visited));
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->length <= node_2->length)
            return true;
        return false;
    }

    /*
     * Outputs
     */

    void instance_format(
            std::ostream& os,
            int verbosity_level) const
    {
        instance_.format(os, verbosity_level);
    }

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_locations != instance_.number_of_locations())
            return "";
        std::stringstream ss;
        ss << node->length;
        return ss.str();
    }

    void solution_format(
            const std::shared_ptr<Node>& node,
            std::ostream& os,
            int verbosity_level) const
    {
        if (verbosity_level >= 1) {
            os
                << "Length:  " << node->length << std::endl
                ;
        }
        if (verbosity_level >= 2) {
        }
    }

    inline void solution_write(
            const std::shared_ptr<Node>& node,
            std::string certificate_path) const
    {
        if (certificate_path.empty())
            return;
        std::ofstream file(certificate_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "Unable to open file \"" + certificate_path + "\".");
        }

        std::vector<LocationId> locations;
        for (auto node_tmp = node;
                node_tmp->parent != nullptr;
                node_tmp = node_tmp->parent) {
            locations.push_back(node_tmp->last_location_id);
        }
        std::reverse(locations.begin(), locations.end());
        for (LocationId location_id: locations)
            file << location_id << " ";
    }

private:

    /** Instance. */
    const Instance& instance_;

    /** Sorted locations. */
    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_locations_;

    /** Generator. */
    mutable std::mt19937_64 generator_;

};

}
}
