/**
 * Sequential Ordering Problem.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/sequentialordering.hpp
 *
 * Tree search:
 * - forward branching
 * - guide: current length + distance to the closest next child
 *
 */

#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/sequentialordering.hpp"

namespace treesearchsolver
{

namespace sequentialordering
{

using namespace orproblems::sequentialordering;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Parameters
    {
        GuideId bound_id = 0;
    };

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> visited; // All visited locations but the last.
        LocationId location_id = 0; // Last visited vertex.
        LocationId number_of_locations = 1;
        Distance length = 0;
        Distance bound_outgoing = 0;
        Distance bound = 0;
        Distance guide = 0;
        LocationPos next_child_pos = 0;
    };

    inline void compute_bound(
            const std::shared_ptr<Node>& node) const
    {
        switch (parameters_.bound_id) {
        case 0: { // prefix
            node->bound = node->length
                + instance_.distance(node->location_id, neighbor(node->location_id, 0));
            break;
        } case 1: { // outgoing O(1)
            if (node->location_id == 0) { // root
                node->bound_outgoing = 0;
                for (LocationId location_id = 0;
                        location_id < instance_.number_of_locations();
                        ++location_id) {
                    Distance d = instance_.distance(location_id, neighbor(location_id, 0));
                    if (d != std::numeric_limits<Distance>::max())
                        node->bound_outgoing += instance_.distance(location_id, neighbor(location_id, 0));
                }
            } else {
                node->bound_outgoing = node->father->bound_outgoing
                    - instance_.distance(node->father->location_id, neighbor(node->father->location_id, 0));
            }
            node->bound = node->length + node->bound_outgoing;
            break;
        } default: {
            node->bound = node->length
                + instance_.distance(node->location_id, neighbor(node->location_id, 0));
        }
        }
    }

    BranchingScheme(
            const Instance& instance,
            const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
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
        compute_bound(r);
        r->guide = r->bound;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        LocationId location_id_next = neighbor(father->location_id, father->next_child_pos);
        Distance d = instance_.distance(father->location_id, location_id_next);
        // Update father
        father->next_child_pos++;
        Distance d_next = instance_.distance(
                father->location_id,
                neighbor(father->location_id, father->next_child_pos));
        if (d_next == std::numeric_limits<Distance>::max()) {
            father->guide = -1;
        } else {
            father->bound = father->bound - d + d_next;
            father->guide = father->bound;
        }
        if (father->visited[location_id_next]
                || d == std::numeric_limits<Distance>::max())
            return nullptr;
        for (LocationId location_id_pred: instance_.predecessors(location_id_next)) {
            if (location_id_pred != father->location_id
                    && !father->visited[location_id_pred]) {
                return nullptr;
            }
        }

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->visited = father->visited;
        child->visited[father->location_id] = true;
        child->location_id = location_id_next;
        child->number_of_locations = father->number_of_locations + 1;
        child->length = father->length + d;
        compute_bound(child);
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
            const std::shared_ptr<Node>& node) const
    {
        (void)node;
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
            if (node_1->location_id != node_2->location_id)
                return false;
            return node_1->visited == node_2->visited;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            assert(node != nullptr);
            size_t hash = hasher_1(node->location_id);
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
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_locations != instance_.number_of_locations())
            return "";
        std::stringstream ss;
        ss << node->length;
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
                << " l " << node_tmp->length
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->location_id
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

        std::vector<LocationId> locations;
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            locations.push_back(node_tmp->location_id);
        }
        std::reverse(locations.begin(), locations.end());
        for (LocationId location_id: locations)
            cert << location_id << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_locations_;
    mutable std::mt19937_64 generator_;

};

}

}

