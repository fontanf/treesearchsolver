#pragma once

/**
 * U-shaped Assembly Line Balancing Problem of Type 1.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/ushapedassemblylinebalancing1.hpp
 *
 * TODO
 *
 */

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

#include "orproblems/ushapedassemblylinebalancing1.hpp"

namespace treesearchsolver
{

namespace ushapedassemblylinebalancing1
{

using namespace orproblems::ushapedassemblylinebalancing1;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Parameters
    {
        GuideId guide_id = 0;
    };

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> jobs;
        JobId j = -1;
        JobId number_of_jobs = 0;
        StationId number_of_stations = 0;
        Time current_station_time = 0;
        Time processing_time_sum = 0;
        StationId bound = -1;
        double guide = 0;
        JobPos next_child_pos = 0;
        bool added_in_current_station = false;
    };

    BranchingScheme(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_jobs_(instance.number_of_jobs())
    {
        // Initialize sorted_jobs_.
        std::iota(sorted_jobs_.begin(), sorted_jobs_.end(), 0);
        sort(sorted_jobs_.begin(), sorted_jobs_.end(),
                [&instance](JobId j1, JobId j2) -> bool
                {
                    return instance.job(j1).processing_time < instance.job(j2).processing_time;
                });
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->jobs.resize(instance_.number_of_jobs(), false);
        r->current_station_time = instance_.cycle_time();
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        JobId j_next = sorted_jobs_[father->next_child_pos];
        // Update father
        father->next_child_pos++;

        if (father->jobs[j_next])
            return nullptr;
        for (JobId j_pred: instance_.job(j_next).predecessors) {
            if (!father->jobs[j_pred]) {
                for (JobId j_succ: instance_.job(j_next).successors)
                    if (!father->jobs[j_succ])
                        return nullptr;
                break;
            }
        }
        Time p = instance_.job(j_next).processing_time;
        if (father->added_in_current_station
               && father->current_station_time + p > instance_.cycle_time()) {
            father->next_child_pos = instance_.number_of_jobs();
            return nullptr;
        }
        if (father->current_station_time + p <= instance_.cycle_time())
            father->added_in_current_station = true;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->j = j_next;
        child->number_of_jobs = father->number_of_jobs + 1;
        child->jobs = father->jobs;
        child->jobs[j_next] = true;
        child->processing_time_sum = father->processing_time_sum + p;
        if (father->current_station_time + p <= instance_.cycle_time()) {
            child->current_station_time = father->current_station_time + p;
            child->number_of_stations = father->number_of_stations;
        } else {
            child->current_station_time = p;
            child->number_of_stations = father->number_of_stations + 1;
        }
        Time total_time = (child->number_of_stations - 1) * instance_.cycle_time()
                + child->current_station_time;
        Time idle_time = total_time - child->processing_time_sum;
        child->bound = std::ceil(
                (double)(idle_time + instance_.processing_time_sum())
                / instance_.cycle_time());
        double mean_job_processing_time = (double)child->processing_time_sum
            / child->number_of_jobs;
        child->guide = (double)idle_time / total_time
            / std::pow(mean_job_processing_time, 2);
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == instance_.number_of_jobs());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(node_1 != nullptr);
        assert(node_2 != nullptr);
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        //if (node_1->number_of_jobs != node_2->number_of_jobs)
        //    return node_1->number_of_jobs < node_2->number_of_jobs;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->number_of_jobs == instance_.number_of_jobs();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->number_of_jobs != instance_.number_of_jobs())
            return false;
        return node_1->bound >= node_2->number_of_stations;
    }

    /*
     * Solution pool.
     */

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_jobs < instance_.number_of_jobs())
            return false;
        if (node_2->number_of_jobs < instance_.number_of_jobs())
            return true;
        return node_1->number_of_stations < node_2->number_of_stations;
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
        std::hash<std::vector<bool>> hasher;

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            return node_1->jobs == node_2->jobs;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher(node->jobs);
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_stations < node_2->number_of_stations)
            return true;
        if (node_1->number_of_stations == node_2->number_of_stations
                && node_1->current_station_time <= node_2->current_station_time)
            return true;
        return false;
    }

    /*
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        return std::to_string(node->number_of_stations);
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        StationId m = node->number_of_stations;
        std::vector<std::vector<JobId>> stations(m);
        std::vector<Time> times(m, 0);
        for (auto node_tmp = node; node_tmp->father != nullptr; node_tmp = node_tmp->father) {
            stations[node_tmp->number_of_stations - 1].push_back(node_tmp->j);
            times[node_tmp->number_of_stations - 1] += instance_.job(node_tmp->j).processing_time;
        }
        for (ushapedassemblylinebalancing1::StationId i = 0; i < m; ++i) {
            os << "Station " << i << " " << times[i] << "/" << instance_.cycle_time() << ":";
            std::reverse(stations[i].begin(), stations[i].end());
            for (ushapedassemblylinebalancing1::JobId j: stations[i])
                os << " " << j;
            os << std::endl;
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

        StationId m = node->number_of_stations;
        std::vector<std::vector<JobId>> stations(m);
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            stations[node_tmp->number_of_stations - 1].push_back(node_tmp->j);
        for (StationId i = 0; i < m; ++i) {
            std::reverse(stations[i].begin(), stations[i].end());
            cert << stations[i].size();
            for (JobId j: stations[i])
                cert << " " << j;
            cert << std::endl;
        }
    }

private:

    const Instance& instance_;
    const Parameters& parameters_;

    mutable std::vector<JobId> sorted_jobs_;

};

}

}

