#pragma once

/**
 * Traveling Repairman Problem.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/travelingrepairman.hpp
 *
 * Tree search:
 * - forward branching
 *
 */

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"
#include "optimizationtools/indexed_set.hpp"

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
        LocationId j = 0;
        LocationId number_of_locations = 1;
        Time current_time = 0;
        double total_completion_time = 0;
        Time bound_orig = 0;
        Time bound = 0;
        double guide = 0;
        LocationPos next_child_pos = 0;
    };

    BranchingSchemeForward(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_locations_(instance.number_of_locations()),
        generator_(0)
    {
        // Initialize sorted_locations_.
        for (LocationId j = 0; j < instance_.number_of_locations(); ++j) {
            sorted_locations_[j].reset(instance.number_of_locations());
            for (LocationId j2 = 0; j2 < instance_.number_of_locations(); ++j2)
                sorted_locations_[j].set_cost(j2, instance_.travel_time(j, j2));
        }
    }

    inline LocationId neighbor(LocationId j, LocationPos pos) const
    {
        return sorted_locations_[j].get(pos, generator_);
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

        LocationId j_next = neighbor(father->j, father->next_child_pos);
        Time t = instance_.travel_time(father->j, j_next);
        // Update father
        father->next_child_pos++;
        if (father->next_child_pos != n) {
            Time t_next = instance_.travel_time(
                    father->j,
                    neighbor(father->j, father->next_child_pos));
            father->bound = father->bound
                + (n - father->number_of_locations) * (t_next - t);
            father->guide = father->bound;
        }
        if (father->visited[j_next])
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        child->father = father;
        child->visited = father->visited;
        child->visited[j_next] = true;
        child->j = j_next;
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
            if (node_1->j != node_2->j)
                return false;
            return node_1->visited == node_2->visited;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher_1(node->j);
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

    std::ostream& print(
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
                << " j " << node_tmp->j
                << " j_pred " << node_tmp->father->j
                << " tj " << instance_.travel_time(node_tmp->father->j, node_tmp->j)
                << std::endl;
        }
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

        std::vector<LocationId> locations;
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            locations.push_back(node_tmp->j);
        std::reverse(locations.begin(), locations.end());
        for (LocationId j: locations)
            cert << j << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_locations_;
    mutable std::mt19937_64 generator_;

};

}

}

