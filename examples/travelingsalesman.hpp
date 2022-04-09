#pragma once

/**
 * Traveling Salesman Problem.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/travelingsalesman.hpp
 *
 * Tree search 1:
 * - forward branching
 * - guide: current length + distance to the closest next child
 *
 * Tree search 2:
 * - insertion branching
 * - guide: current length
 *
 */

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/travelingsalesman.hpp"

namespace treesearchsolver
{

namespace travelingsalesman
{

using namespace orproblems::travelingsalesman;

using GuideId = int64_t;

class BranchingSchemeForward
{

public:

    struct Parameters
    {
        GuideId bound_id = 1;
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

    BranchingSchemeForward(const Instance& instance, const Parameters& parameters):
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

    inline void compute_bound(
            const std::shared_ptr<Node>& node) const
    {
        switch (parameters_.bound_id) {
        case 0: { // prefix
            node->bound = node->length;
            break;
        } case 1: { // outgoing
            if (node->j == 0) { // root
                node->bound_outgoing = 0;
                for (VertexId j = 0; j < instance_.number_of_vertices(); ++j)
                    node->bound_outgoing += instance_.distance(j, neighbor(j, 0));
            } else {
                node->bound_outgoing = node->father->bound_outgoing
                    - instance_.distance(node->father->j, neighbor(node->father->j, 0));
            }
            node->bound = node->length + node->bound_outgoing;
            break;
        } default: {
            node->bound = node->length;
        }
        }
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
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
        if (father->visited[j_next])
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
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
        Distance d2 = node_2->length + instance_.distance(node_2->j, 0);
        return node_1->bound >= d2;
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
        return node_1->length + instance_.distance(node_1->j, 0)
            < node_2->length + instance_.distance(node_2->j, 0);
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
        ss << node->length + instance_.distance(node->j, 0);
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
            std::string certificate_path) const
    {
        if (certificate_path.empty())
            return;
        std::ofstream cert(certificate_path);
        if (!cert.good()) {
            throw std::runtime_error(
                    "Unable to open file \"" + certificate_path + "\".");
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

class BranchingSchemeInsertion
{

public:

    struct Parameters
    {
        GuideId guide_id = 0;
        GuideId sort_criterion_id = 0;
    };

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<VertexId> vertices;
        VertexPos pos = 0;
        VertexId number_of_vertices = 1;
        Distance length = 0;
        Distance guide = 0;
        VertexPos next_child_pos = 0;
    };

    BranchingSchemeInsertion(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_vertices_(instance.number_of_vertices() - 1)
    {
        // Initialize sorted_vertices_.
        std::iota(sorted_vertices_.begin(), sorted_vertices_.end(), 1);
        switch (parameters_.sort_criterion_id) {
        case 0: {
            std::sort(sorted_vertices_.begin(), sorted_vertices_.end(),
                    [&instance](VertexId j1, VertexId j2) -> bool
                    {
                        return instance.distance(0, j1) < instance.distance(0, j2);
                    });
            break;
        } case 1: {
            std::sort(sorted_vertices_.begin(), sorted_vertices_.end(),
                    [&instance](VertexId j1, VertexId j2) -> bool
                    {
                        return instance.distance(0, j1) > instance.distance(0, j2);
                    });
            break;
        } case 2: {
            std::random_shuffle(sorted_vertices_.begin(), sorted_vertices_.end());
            break;
        } default: {
            assert(false);
            break;
        }
        }
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        auto father = node->father;
        node->vertices.insert(
                node->vertices.end(),
                father->vertices.begin(),
                father->vertices.begin() + node->pos + 1);
        node->vertices.push_back(sorted_vertices_[instance_.number_of_vertices() - node->number_of_vertices]);
        node->vertices.insert(
                node->vertices.end(),
                father->vertices.begin() + node->pos + 1,
                father->vertices.end());
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        r->vertices = {0, 0};
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        // Compute father's structures.
        if (father->vertices.empty())
            compute_structures(father);

        VertexId j_next = sorted_vertices_[instance_.number_of_vertices() - father->number_of_vertices - 1];
        VertexPos pos = father->next_child_pos;
        VertexId j_bef = father->vertices[pos];
        VertexId j_aft = father->vertices[pos + 1];
        // Update father
        father->next_child_pos++;
        if (father->number_of_vertices == 2) // Remove for asymmetric
            father->next_child_pos++;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        child->pos = pos;
        child->father = father;
        if (j_bef != j_aft) {
            child->length = father->length
                - instance_.distance(j_bef, j_aft)
                + instance_.distance(j_bef, j_next)
                + instance_.distance(j_next, j_aft);
        } else {
            child->length = father->length
                + instance_.distance(j_bef, j_next)
                + instance_.distance(j_next, j_aft);
        }
        child->number_of_vertices = father->number_of_vertices + 1;
        child->guide = child->length;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == node->number_of_vertices);
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->number_of_vertices != node_2->number_of_vertices)
            return node_1->number_of_vertices < node_2->number_of_vertices;
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
        return node_1->length >= node_2->length;
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
            const std::shared_ptr<Node>&,
            const std::shared_ptr<Node>&) const
    {
        return false;
    }

    /*
     * Dominances.
     */

    inline bool comparable(
            const std::shared_ptr<Node>&) const
    {
        return false;
    }

    struct NodeHasher
    {
        inline bool operator()(
                const std::shared_ptr<Node>&,
                const std::shared_ptr<Node>&) const
        {
            return false;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>&) const
        {
            return 0;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(); }

    inline bool dominates(
            const std::shared_ptr<Node>&,
            const std::shared_ptr<Node>&) const
    {
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
                << " pos " << node_tmp->pos
                << " n " << node_tmp->number_of_vertices
                << " l " << node_tmp->length
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

        if (node->vertices.empty())
            compute_structures(node);
        for (VertexId j: node->vertices)
            if (j != 0)
                cert << j << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

    mutable std::vector<VertexId> sorted_vertices_;

};

}

}

