#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"

/**
 * Sequential Ordering Problem.
 * (Asymmetric Travelling Salesman Problem with Precedence Constraints)
 *
 * Input:
 * - n cities and an n×n matrix containing the distances between each pair of
 *   cities (not necessarily symmetric)
 * - a directed acyclic graph such that each vertex represents a city and an
 *   arc from vertex j1 to vertex j2 indicates that city j1 must be visited
 *   before city j2
 * Problem:
 * - find a route from city 1 such that:
 *   - each city is visited exactly once
 *   - precedence constraints are satisfied
 * Objective:
 * - minimize the total length of the tour
 *
 * Tree search:
 * - forward branching
 * - guide: current length + distance to the closest next child
 *
 */

namespace treesearchsolver
{

namespace sequentialordering
{

typedef int64_t VertexId;
typedef int64_t VertexPos;
typedef int64_t Distance;

struct Location
{
    std::vector<VertexId> predecessors;
};

class Instance
{

public:

    Instance(VertexId n):
        locations_(n),
        distances_(n, std::vector<Distance>(n, -1))
    {
        for (VertexId j = 0; j < n; ++j)
            distances_[j][j] = 0;
    }
    void set_distance(VertexId j1, VertexId j2, Distance d)
    {
        distances_[j1][j2] = d;
        distance_max_ = std::max(distance_max_, d);
    }
    void add_predecessor(VertexId j1, VertexId j2)
    {
        locations_[j1].predecessors.push_back(j2);
    }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "tsplib") {
            read_tsplib(file);
        } else if (format == "soplib") {
            read_soplib(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline VertexId vertex_number() const { return locations_.size(); }
    inline Distance distance(VertexId j1, VertexId j2) const { return distances_[j1][j2]; }
    inline const std::vector<VertexId>& predecessors(VertexId j) const { return locations_[j].predecessors; }
    inline Distance maximum_distance() const { return distance_max_; }

private:

    void read_tsplib(std::ifstream& file)
    {
        std::string tmp;
        std::vector<std::string> line;
        VertexId n = -1;
        std::string edge_weight_type;
        std::string edge_weight_format;
        for (;;) {
            getline(file, tmp);
            line = optimizationtools::split(tmp, ' ');
            if (tmp.rfind("DIMENSION", 0) == 0) {
                n = std::stol(line.back());
                locations_ = std::vector<Location>(n);
                distances_ = std::vector<std::vector<Distance>>(n, std::vector<Distance>(n, -1));
                for (VertexId j = 0; j < n; ++j)
                    distances_[j][j] = std::numeric_limits<Distance>::max();
            } else if (tmp.rfind("EDGE_WEIGHT_TYPE", 0) == 0) {
                edge_weight_type = line.back();
            } else if (tmp.rfind("EDGE_WEIGHT_FORMAT", 0) == 0) {
                edge_weight_format = line.back();
            } else if (tmp.rfind("EDGE_WEIGHT_SECTION", 0) == 0) {
                if (edge_weight_format == "FULL_MATRIX") {
                    for (VertexId j1 = 0; j1 < n - 1; ++j1) {
                        getline(file, tmp);
                        line = optimizationtools::split(tmp, ' ');
                        for (VertexId j2 = 0; j2 < n; ++j2) {
                            Distance d = std::stol(line[j2]);
                            if (d == -1)
                                add_predecessor(j1, j2);
                            if (j2 == j1 || d == -1)
                                d = std::numeric_limits<Distance>::max();
                            set_distance(j1, j2, d);
                        }
                    }
                } else {
                    std::cerr << "\033[31m" << "ERROR, EDGE_WEIGHT_FORMAT \"" << edge_weight_format << "\" not implemented." << "\033[0m" << std::endl;
                }
            } else if (tmp.rfind("EOF", 0) == 0) {
                break;
            } else {
                std::cerr << "\033[31m" << "ERROR, ENTRY \"" << line[0] << "\" not implemented." << "\033[0m" << std::endl;
            }
        }

        // Compute distances.
        if (edge_weight_type == "EXPLICIT") {
        } else {
            std::cerr << "\033[31m" << "ERROR, EDGE_WEIGHT_TYPE \"" << edge_weight_type << "\" not implemented." << "\033[0m" << std::endl;
        }
    }

    void read_soplib(std::ifstream& file)
    {
        std::string tmp;
        std::vector<std::string> line;
        for (VertexId j1 = 0; getline(file, tmp); ++j1) {
            line = optimizationtools::split(tmp, '\t');
            if (j1 == 0) {
                VertexId n = line.size();
                locations_ = std::vector<Location>(n);
                distances_ = std::vector<std::vector<Distance>>(n, std::vector<Distance>(n, -1));
            }
            for (VertexId j2 = 0; j2 < vertex_number(); ++j2) {
                Distance d = std::stol(line[j2]);
                if (d == -1)
                    add_predecessor(j1, j2);
                if (j2 == j1 || d == -1)
                    d = std::numeric_limits<Distance>::max();
                set_distance(j1, j2, d);
            }
        }
    }

    std::vector<Location> locations_;
    std::vector<std::vector<Distance>> distances_;
    Distance distance_max_ = 0;

};

class BranchingScheme
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> visited; // All visited vertices but the last.
        VertexId j = 0; // Last visited vertex.
        VertexId vertex_number = 1;
        Distance length = 0;
        Distance guide = 0;
        VertexPos next_child_pos = 0;
    };

    BranchingScheme(const Instance& instance):
        instance_(instance),
        sorted_vertices_(instance.vertex_number()),
        generator_(0)
    {
        // Initialize sorted_vertices_.
        for (VertexId j = 0; j < instance_.vertex_number(); ++j) {
            sorted_vertices_[j].reset(instance.vertex_number());
            for (VertexId j2 = 0; j2 < instance_.vertex_number(); ++j2)
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
        r->visited.resize(instance_.vertex_number(), false);
        r->guide = instance_.distance(0, neighbor(0, 0));
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node> father) const
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
            father->guide = father->guide - d + d_next;
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
        child->vertex_number = father->vertex_number + 1;
        child->length = father->length + d;
        child->guide = child->length
            + instance_.distance(j_next, neighbor(j_next, 0));
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
        return node->vertex_number == instance_.vertex_number();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->vertex_number != instance_.vertex_number())
            return false;
        return node_1->length >= node_2->length;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->vertex_number < instance_.vertex_number())
            return false;
        if (node_2->vertex_number < instance_.vertex_number())
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

    Distance display(const std::shared_ptr<Node>& node) const
    {
        return node->length;
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
        if (node_1->length > node_2->length)
            return false;
        return true;
    }

private:

    const Instance& instance_;

    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_vertices_;
    mutable std::mt19937_64 generator_;

};

}

}

