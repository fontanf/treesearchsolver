#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"
#include "optimizationtools/indexed_set.hpp"

/**
 * Travelling Salesman Problem.
 *
 * Input:
 * - n cities and an n×n symmetric matrix containing the distances between each
 *   pair of cities
 * Problem:
 * - find a tour from city 1 to city 1 such that
 *   - each other city is visited exactly once
 * Objective:
 * - minimize the total length of the tour
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

namespace treesearchsolver
{

namespace travellingsalesman
{

typedef int64_t VertexId;
typedef int64_t VertexPos;
typedef int64_t Distance;

struct Location
{
    double x;
    double y;
    double z;
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
    void set_xy(VertexId j, double x, double y, double z = -1)
    {
        locations_[j].x = x;
        locations_[j].y = y;
        locations_[j].z = z;
    }
    void set_distance(VertexId j1, VertexId j2, Distance d)
    {
        distances_[j1][j2] = d;
        distances_[j2][j1] = d;
        distance_max_ = std::max(distance_max_, d);
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
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline VertexId vertex_number() const { return locations_.size(); }
    inline double x(VertexId j) const { return locations_[j].x; }
    inline double y(VertexId j) const { return locations_[j].y; }
    inline Distance distance(VertexId j1, VertexId j2) const { return distances_[j1][j2]; }
    inline Distance maximum_distance() const { return distance_max_; }

    std::pair<bool, Distance> check(std::string certificate_path)
    {
        std::ifstream file(certificate_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << certificate_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return {false, 0};
        }

        VertexId n = vertex_number();
        VertexId j_prec = 0;
        VertexId j = -1;
        optimizationtools::IndexedSet vertices(n);
        vertices.add(0);
        VertexPos duplicates = 0;
        Distance total_distance = 0;
        while (file >> j) {
            if (vertices.contains(j)) {
                duplicates++;
                std::cout << "Vertex " << j << " has already been visited." << std::endl;
            }
            vertices.add(j);
            total_distance += distance(j_prec, j);
            std::cout << "Job: " << j
                << "; Distance: " << distance(j_prec, j)
                << "; Total distance: " << total_distance
                << std::endl;
            j_prec = j;
        }
        total_distance += distance(j_prec, 0);
        bool feasible
            = (vertices.size() == n)
            && (duplicates == 0);

        std::cout << "---" << std::endl;
        std::cout << "Vertices number:        " << vertices.size() << " / " << n  << std::endl;
        std::cout << "Duplicates:             " << duplicates << std::endl;
        std::cout << "Feasible:               " << feasible << std::endl;
        std::cout << "Total distance:         " << total_distance << std::endl;
        return {feasible, total_distance};
    }

private:

    void read_tsplib(std::ifstream& file)
    {
        std::string tmp;
        std::vector<std::string> line;
        VertexId n = -1;
        std::string edge_weight_type;
        std::string edge_weight_format;
        std::string node_coord_type = "TWOD_COORDS";
        while (getline(file, tmp)) {
            line = optimizationtools::split(tmp, ' ');
            if (line.size() == 0) {
            } else if (tmp.rfind("NAME", 0) == 0) {
            } else if (tmp.rfind("COMMENT", 0) == 0) {
            } else if (tmp.rfind("TYPE", 0) == 0) {
            } else if (tmp.rfind("DISPLAY_DATA_TYPE", 0) == 0) {
            } else if (tmp.rfind("DIMENSION", 0) == 0) {
                n = std::stol(line.back());
                locations_ = std::vector<Location>(n);
                distances_ = std::vector<std::vector<Distance>>(n, std::vector<Distance>(n, -1));
            } else if (tmp.rfind("EDGE_WEIGHT_TYPE", 0) == 0) {
                edge_weight_type = line.back();
            } else if (tmp.rfind("EDGE_WEIGHT_FORMAT", 0) == 0) {
                edge_weight_format = line.back();
            } else if (tmp.rfind("NODE_COORD_TYPE", 0) == 0) {
                node_coord_type = line.back();
            } else if (tmp.rfind("EDGE_WEIGHT_SECTION", 0) == 0) {
                if (edge_weight_format == "UPPER_ROW") {
                    Distance d;
                    for (VertexId j1 = 0; j1 < n - 1; ++j1) {
                        for (VertexId j2 = j1 + 1; j2 < n; ++j2) {
                            file >> d;
                            set_distance(j1, j2, d);
                        }
                    }
                } else if (edge_weight_format == "LOWER_ROW") {
                    Distance d;
                    for (VertexId j1 = 1; j1 < n; ++j1) {
                        for (VertexId j2 = 0; j2 < j1; ++j2) {
                            file >> d;
                            set_distance(j1, j2, d);
                        }
                    }
                } else if (edge_weight_format == "UPPER_DIAG_ROW") {
                    Distance d;
                    for (VertexId j1 = 0; j1 < n; ++j1) {
                        for (VertexId j2 = j1; j2 < n; ++j2) {
                            file >> d;
                            set_distance(j1, j2, d);
                        }
                    }
                } else if (edge_weight_format == "LOWER_DIAG_ROW") {
                    Distance d;
                    for (VertexId j1 = 0; j1 < n; ++j1) {
                        for (VertexId j2 = 0; j2 <= j1; ++j2) {
                            file >> d;
                            set_distance(j1, j2, d);
                        }
                    }
                } else if (edge_weight_format == "FULL_MATRIX") {
                    Distance d;
                    for (VertexId j1 = 0; j1 < n; ++j1) {
                        for (VertexId j2 = 0; j2 < n; ++j2) {
                            file >> d;
                            set_distance(j1, j2, d);
                        }
                    }
                } else {
                    std::cerr << "\033[31m" << "ERROR, EDGE_WEIGHT_FORMAT \"" << edge_weight_format << "\" not implemented." << "\033[0m" << std::endl;
                }
            } else if (tmp.rfind("NODE_COORD_SECTION", 0) == 0) {
                if (node_coord_type == "TWOD_COORDS") {
                    VertexId tmp;
                    double x, y;
                    for (VertexId j = 0; j < n; ++j) {
                        file >> tmp >> x >> y;
                        set_xy(j, x, y);
                    }
                } else if (node_coord_type == "THREED_COORDS") {
                    VertexId tmp;
                    double x, y, z;
                    for (VertexId j = 0; j < n; ++j) {
                        file >> tmp >> x >> y >> z;
                        set_xy(j, x, y, z);
                    }
                }
            } else if (tmp.rfind("DISPLAY_DATA_SECTION", 0) == 0) {
                VertexId tmp;
                double x, y;
                for (VertexId j = 0; j < n; ++j) {
                    file >> tmp >> x >> y;
                    set_xy(j, x, y);
                }
            } else if (tmp.rfind("EOF", 0) == 0) {
                break;
            } else {
                std::cerr << "\033[31m" << "ERROR, ENTRY \"" << line[0] << "\" not implemented." << "\033[0m" << std::endl;
            }
        }

        // Compute distances.
        if (edge_weight_type == "EUC_2D") {
            for (VertexId j1 = 0; j1 < n; ++j1) {
                for (VertexId j2 = j1 + 1; j2 < n; ++j2) {
                    Distance xd = x(j2) - x(j1);
                    Distance yd = y(j2) - y(j1);
                    Distance d = std::round(std::sqrt(xd * xd + yd * yd));
                    set_distance(j1, j2, d);
                }
            }
        } else if (edge_weight_type == "CEIL_2D") {
            for (VertexId j1 = 0; j1 < n; ++j1) {
                for (VertexId j2 = j1 + 1; j2 < n; ++j2) {
                    Distance xd = x(j2) - x(j1);
                    Distance yd = y(j2) - y(j1);
                    Distance d = std::ceil(std::sqrt(xd * xd + yd * yd));
                    set_distance(j1, j2, d);
                }
            }
        } else if (edge_weight_type == "GEO") {
            std::vector<double> latitudes(n, 0);
            std::vector<double> longitudes(n, 0);
            for (VertexId j = 0; j < n; ++j) {
                double pi = 3.141592;
                int deg_x = std::round(x(j));
                double min_x = x(j) - deg_x;
                latitudes[j] = pi * (deg_x + 5.0 * min_x / 3.0) / 180.0;
                int deg_y = std::round(y(j));
                double min_y = y(j) - deg_y;
                longitudes[j] = pi * (deg_y + 5.0 * min_y / 3.0) / 180.0;
            }
            double rrr = 6378.388;
            for (VertexId j1 = 0; j1 < n; ++j1) {
                for (VertexId j2 = j1 + 1; j2 < n; ++j2) {
                    double q1 = cos(longitudes[j1] - longitudes[j2]);
                    double q2 = cos(latitudes[j1] - latitudes[j2]);
                    double q3 = cos(latitudes[j1] + latitudes[j2]);
                    Distance d = (Distance)(rrr * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
                    set_distance(j1, j2, d);
                }
            }
        } else if (edge_weight_type == "ATT") {
            for (VertexId j1 = 0; j1 < n; ++j1) {
                for (VertexId j2 = j1 + 1; j2 < n; ++j2) {
                    double xd = x(j1) - x(j2);
                    double yd = y(j1) - y(j2);
                    double rij = sqrt((xd * xd + yd * yd) / 10.0);
                    int tij = std::round(rij);
                    Distance d = (tij < rij)? tij + 1: tij;
                    set_distance(j1, j2, d);
                }
            }
        } else if (edge_weight_type == "EXPLICIT") {
        } else {
            std::cerr << "\033[31m" << "ERROR, EDGE_WEIGHT_TYPE \"" << edge_weight_type << "\" not implemented." << "\033[0m" << std::endl;
        }
        for (VertexId j = 0; j < n; ++j)
            distances_[j][j] = std::numeric_limits<Distance>::max();
    }

    std::vector<Location> locations_;
    std::vector<std::vector<Distance>> distances_;
    Distance distance_max_ = 0;

};

std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "vertex number " << instance.vertex_number() << std::endl;
    for (VertexId j1 = 0; j1 < instance.vertex_number(); ++j1) {
        os << "vertex " << j1 << ":";
        for (VertexId j2 = 0; j2 < instance.vertex_number(); ++j2)
            os << " " << instance.distance(j1, j2);
        os << std::endl;
    }
    return os;
}

typedef int64_t GuideId;

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
        VertexId vertex_number = 1;
        Distance length = 0;
        Distance bound_outgoing = 0;
        Distance bound = 0;
        Distance guide = 0;
        VertexPos next_child_pos = 0;
    };

    BranchingSchemeForward(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
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
                for (VertexId j = 0; j < instance_.vertex_number(); ++j)
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
        r->visited.resize(instance_.vertex_number(), false);
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
        child->vertex_number = father->vertex_number + 1;
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
        return node->vertex_number == instance_.vertex_number();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->vertex_number != instance_.vertex_number())
            return false;
        Distance d2 = node_2->length + instance_.distance(node_2->j, 0);
        return node_1->bound >= d2;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->vertex_number < instance_.vertex_number())
            return false;
        if (node_2->vertex_number < instance_.vertex_number())
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
        if (node->vertex_number != instance_.vertex_number())
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
                << " n " << node_tmp->vertex_number
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
        VertexId vertex_number = 1;
        Distance length = 0;
        Distance guide = 0;
        VertexPos next_child_pos = 0;
    };

    BranchingSchemeInsertion(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_vertices_(instance.vertex_number() - 1)
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
        node->vertices.push_back(sorted_vertices_[instance_.vertex_number() - node->vertex_number]);
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

        VertexId j_next = sorted_vertices_[instance_.vertex_number() - father->vertex_number - 1];
        VertexPos pos = father->next_child_pos;
        VertexId j_bef = father->vertices[pos];
        VertexId j_aft = father->vertices[pos + 1];
        // Update father
        father->next_child_pos++;
        if (father->vertex_number == 2) // Remove for asymmetric
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
        child->vertex_number = father->vertex_number + 1;
        child->guide = child->length;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == node->vertex_number);
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->vertex_number != node_2->vertex_number)
            return node_1->vertex_number < node_2->vertex_number;
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
        if (node->vertex_number != instance_.vertex_number())
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
                << " n " << node_tmp->vertex_number
                << " l " << node_tmp->length
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

