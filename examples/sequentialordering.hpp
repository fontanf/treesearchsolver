#pragma once

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

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"
#include "optimizationtools/indexed_set.hpp"

#include "orproblems/sequentialordering.hpp"

namespace treesearchsolver
{

namespace sequentialordering
{

using namespace orproblems::sequentialordering;

typedef int64_t GuideId;

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
        std::vector<bool> visited; // All visited vertices but the last.
        VertexId j = 0; // Last visited vertex.
        VertexId number_of_vertices = 1;
        Distance length = 0;
        Distance bound_outgoing = 0;
        Distance bound = 0;
        Distance guide = 0;
        VertexPos next_child_pos = 0;
    };

    inline void compute_bound(
            const std::shared_ptr<Node>& node) const
    {
        switch (parameters_.bound_id) {
        case 0: { // prefix
            node->bound = node->length
                + instance_.distance(node->j, neighbor(node->j, 0));
            break;
        } case 1: { // outgoing O(1)
            if (node->j == 0) { // root
                node->bound_outgoing = 0;
                for (VertexId j = 0; j < instance_.number_of_vertices(); ++j) {
                    Distance d = instance_.distance(j, neighbor(j, 0));
                    if (d != std::numeric_limits<Distance>::max())
                        node->bound_outgoing += instance_.distance(j, neighbor(j, 0));
                }
            } else {
                node->bound_outgoing = node->father->bound_outgoing
                    - instance_.distance(node->father->j, neighbor(node->father->j, 0));
            }
            node->bound = node->length + node->bound_outgoing;
            break;
        } default: {
            node->bound = node->length
                + instance_.distance(node->j, neighbor(node->j, 0));
        }
        }
    }

    BranchingScheme(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_vertices_(instance.number_of_vertices()),
        generator_(0)
    {
        // Initialize sorted_vertices_.
        for (VertexId j = 0; j < instance_.number_of_vertices(); ++j) {
            sorted_vertices_[j].reset(instance.number_of_vertices());
            for (VertexId j2 = 0; j2 < instance_.number_of_vertices(); ++j2)
                sorted_vertices_[j].set_cost(j2, instance_.distance(j, j2));
        }
    }

    inline VertexId neighbor(VertexId j, VertexPos pos) const
    {
        return sorted_vertices_[j].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->visited.resize(instance_.number_of_vertices(), false);
        compute_bound(r);
        r->guide = r->bound;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        VertexId j_next = neighbor(father->j, father->next_child_pos);
        Distance d = instance_.distance(father->j, j_next);
        // Update father
        father->next_child_pos++;
        Distance d_next = instance_.distance(
                father->j,
                neighbor(father->j, father->next_child_pos));
        if (d_next == std::numeric_limits<Distance>::max()) {
            father->guide = -1;
        } else {
            father->bound = father->bound - d + d_next;
            father->guide = father->bound;
        }
        if (father->visited[j_next]
                || d == std::numeric_limits<Distance>::max())
            return nullptr;
        for (VertexId j_pred: instance_.predecessors(j_next))
            if (j_pred != father->j && !father->visited[j_pred])
                return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->visited = father->visited;
        child->visited[father->j] = true;
        child->j = j_next;
        child->number_of_vertices = father->number_of_vertices + 1;
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
        return node->number_of_vertices == instance_.number_of_vertices();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->number_of_vertices != instance_.number_of_vertices())
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
        if (node_1->number_of_vertices < instance_.number_of_vertices())
            return false;
        if (node_2->number_of_vertices < instance_.number_of_vertices())
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
        std::hash<VertexId> hasher_1;
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
            assert(node != nullptr);
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
        if (node_1->length <= node_2->length)
            return true;
        return false;
    }

    /*
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_vertices != instance_.number_of_vertices())
            return "";
        std::stringstream ss;
        ss << node->length;
        return ss.str();
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->number_of_vertices
                << " l " << node_tmp->length
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->j
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

        std::vector<VertexId> vertices;
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            vertices.push_back(node_tmp->j);
        std::reverse(vertices.begin(), vertices.end());
        for (VertexId j: vertices)
            cert << j << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_vertices_;
    mutable std::mt19937_64 generator_;

};

}

}

