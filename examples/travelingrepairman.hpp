/**
 * Traveling repairman problem
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/travelingrepairman.hpp
 *
 * Tree search:
 * - forward branching
 *
 */

#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/travelingrepairman.hpp"

namespace treesearchsolver
{

namespace travelingrepairman
{

using namespace orproblems::travelingrepairman;

using GuideId = int64_t;

class BranchingSchemeForward
{

public:

    struct Parameters
    {
        GuideId guide_id = 0;
    };

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> visited;
        LocationId location_id = 0;
        LocationId number_of_locations = 1;
        Time current_time = 0;
        double total_completion_time = 0;
        Time bound_orig = 0;
        Time bound = 0;
        double guide = 0;
        LocationPos next_child_pos = 0;
    };

    BranchingSchemeForward(
            const Instance& instance,
            Parameters parameters):
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
                        instance_.travel_time(location_id, location_id_2));
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
        auto r = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        r->visited.resize(instance_.number_of_locations(), false);
        r->visited[0] = true;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));
        LocationId n = instance_.number_of_locations();

        LocationId location_id_next = neighbor(father->location_id, father->next_child_pos);
        Time t = instance_.travel_time(father->location_id, location_id_next);
        // Update father
        father->next_child_pos++;
        if (father->next_child_pos != n) {
            Time t_next = instance_.travel_time(
                    father->location_id,
                    neighbor(father->location_id, father->next_child_pos));
            father->bound = father->bound
                + (n - father->number_of_locations) * (t_next - t);
            father->guide = father->bound;
        }
        if (father->visited[location_id_next])
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        child->father = father;
        child->visited = father->visited;
        child->visited[location_id_next] = true;
        child->location_id = location_id_next;
        child->number_of_locations = father->number_of_locations + 1;
        child->current_time = father->current_time + t;
        child->total_completion_time = father->total_completion_time
            + child->current_time;
        child->bound_orig = father->bound_orig
            + (n - father->number_of_locations)
            * (child->current_time - father->current_time);
        child->bound = child->bound_orig;
        child->guide = child->bound;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == instance_.number_of_locations());
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
        return node_1->bound >= node_2->total_completion_time;
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
        return node_1->total_completion_time < node_2->total_completion_time;
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
            if (node_1->location_id != node_2->location_id)
                return false;
            return node_1->visited == node_2->visited;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
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
        if (node_1->current_time <= node_2->current_time
                && node_1->total_completion_time <= node_2->total_completion_time)
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
        ss << node->total_completion_time;
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
                << " t " << node_tmp->current_time
                << " tct " << node_tmp->total_completion_time
                << " bndo " << node_tmp->bound_orig
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->location_id
                << " j_pred " << node_tmp->father->location_id
                << " tj " << instance_.travel_time(node_tmp->father->location_id, node_tmp->location_id)
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

