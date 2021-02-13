#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"

namespace treesearchsolver
{

namespace thieforienteering
{

typedef int64_t VertexId;
typedef int64_t Distance;
typedef double Time;
typedef int64_t ItemId;
typedef int64_t ItemPos;
typedef int64_t Profit;
typedef int64_t Weight;
typedef int64_t GuideId;

struct Item
{
    Profit profit;
    Weight weight;
    VertexId location;
};

struct Location
{
    double x;
    double y;
    double z;
    std::vector<ItemId> items;
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
    void set_time_limit(Time time_limit)
    {
        time_limit_ = time_limit;
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
    void add_item(VertexId j, Profit profit, Weight weight)
    {
        ItemId i = items_.size();
        locations_[j].items.push_back(i);
        Item item;
        item.location = j;
        item.profit = profit;
        item.weight = weight;
        items_.push_back(item);
    }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "santos2018") {
            read_santos2018(file);
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
    inline Time time_limit() const { return time_limit_; }
    inline Time duration(VertexId j1, VertexId j2, Weight weight) const
    {
        double speed = speed_max_ - (double)(weight * (speed_max_ - speed_min_)) / capacity();
        return (double)distance(j1, j2) / speed;
    }

    inline ItemId item_number() const { return items_.size(); }
    inline const Item& item(ItemId i) const { return items_[i]; }
    inline const std::vector<ItemId>& items(VertexId j) const { return locations_[j].items; }
    inline Weight capacity() const { return capacity_; }

private:

    void read_santos2018(std::ifstream& file)
    {
        std::string tmp;
        std::vector<std::string> line;
        VertexId n = -1;
        ItemId n_items = -1;
        std::string edge_weight_type;
        std::string edge_weight_format;
        std::string node_coord_type = "TWOD_COORDS";
        while (getline(file, tmp)) {
            line = optimizationtools::split(tmp, ' ');
            if (line.size() == 0) {
            } else if (tmp.rfind("NAME", 0) == 0) {
            } else if (tmp.rfind("PROBLEM NAME", 0) == 0) {
            } else if (tmp.rfind("COMMENT", 0) == 0) {
            } else if (tmp.rfind("TYPE", 0) == 0) {
            } else if (tmp.rfind("KNAPSACK DATA TYPE", 0) == 0) {
            } else if (tmp.rfind("DISPLAY_DATA_TYPE", 0) == 0) {
            } else if (tmp.rfind("DIMENSION", 0) == 0) {
                n = std::stol(line.back());
                locations_ = std::vector<Location>(n);
                distances_ = std::vector<std::vector<Distance>>(n, std::vector<Distance>(n, -1));
            } else if (tmp.rfind("NUMBER OF ITEMS", 0) == 0) {
                n_items = std::stol(line.back());
            } else if (tmp.rfind("CAPACITY OF KNAPSACK", 0) == 0) {
                capacity_ = std::stol(line.back());
            } else if (tmp.rfind("MAX TIME", 0) == 0) {
                time_limit_ = std::stol(line.back());
            } else if (tmp.rfind("MIN SPEED", 0) == 0) {
                speed_min_ = std::stod(line.back());
            } else if (tmp.rfind("MAX SPEED", 0) == 0) {
                speed_max_ = std::stod(line.back());
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
            } else if (tmp.rfind("ITEMS SECTION", 0) == 0) {
                ItemId tmp = -1;
                Profit profit = -1;
                Weight weight = -1;
                VertexId location = -1;
                for (ItemId i = 0; i < n_items; ++i) {
                    file >> tmp >> profit >> weight >> location;
                    add_item(location - 1, profit, weight);
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
            distances_[j][j] = 0;
    }

    std::vector<Location> locations_;
    std::vector<std::vector<Distance>> distances_;
    Distance distance_max_ = 0;
    double speed_min_ = -1;
    double speed_max_ = -1;
    Time time_limit_ = -1;

    std::vector<Item> items_;
    Weight capacity_ = -1;

};

class BranchingScheme
{
    /**
     * - Branching on the next item
     * - Guide:
     *   - time^exponent_time * weight^exponent_weight / profit^exponent_profit
     * - No bound
     */

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_items;
        ItemId i = -1; // Last added item.
        ItemId item_number = 0;
        VertexId vertex_number = 0;
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
        sorted_items_(instance.item_number() + 1),
        generator_(0)
    {
        // Initialize sorted_items_.
        for (ItemId i = 0; i < instance_.item_number() + 1; ++i) {
            sorted_items_[i].reset(instance.item_number());
            VertexId j = (i == instance_.item_number())?
                0: instance_.item(i).location;
            for (VertexId i2 = 0; i2 < instance_.item_number(); ++i2) {
                VertexId j2 = instance_.item(i2).location;
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

    inline VertexId neighbor(ItemId i, ItemPos pos) const
    {
        assert(i < instance_.item_number() + 1);
        assert(pos < instance_.item_number());
        return sorted_items_[i].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_items.resize(instance_.item_number(), true);
        r->i = instance_.item_number();
        r->guide = 0;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node> father) const
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
        VertexId j = (father->father == nullptr)?
            0: instance_.item(father->i).location;
        VertexId j_next = instance_.item(i_next).location;
        Time t = instance_.duration(j, j_next, father->weight);
        Time t_end = instance_.duration(j_next, instance_.vertex_number() - 1,
                father->weight + instance_.item(i_next).weight);
        if (father->time + t + t_end > instance_.time_limit())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->available_items = father->available_items;
        child->available_items[i_next] = false;
        child->i = i_next;
        child->item_number = father->item_number + 1;
        child->vertex_number = father->vertex_number;
        if (j_next != j) {
            for (ItemId i_tmp: instance_.items(j))
                child->available_items[i_tmp] = false;
            child->vertex_number++;
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
        return (node->next_child_pos == instance_.item_number() - 1);
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        //if (node_1->item_number != node_2->item_number)
        //    return node_1->item_number < node_2->item_number;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->item_number == instance_.item_number();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        (void)node_1;
        (void)node_2;
        return false;
    }

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

    std::string display(const std::shared_ptr<Node>& node) const
    {
        VertexId j = instance_.item(node->i).location;
        std::stringstream ss;
        ss << node->profit
            << " (n" << node->vertex_number
            << " m" << node->item_number
            << " w" << std::round(100 * (double)node->weight / instance_.capacity()) / 100
            << " t" << std::round(100 * (node->time + instance_.duration(j, instance_.vertex_number() - 1, node->weight)) / instance_.time_limit()) / 100
            << ")";
        return ss.str();
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

    const Instance& instance() const { return instance_; }

    struct NodeHasher
    {
        const BranchingScheme& branching_scheme_;
        std::hash<VertexId> hasher_1;
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
            return node_1->available_items == node_2->available_items;
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
        if (node_1->time > node_2->time
                || node_1->profit < node_2->profit
                || node_1->weight > node_2->weight)
            return false;
        return true;
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

