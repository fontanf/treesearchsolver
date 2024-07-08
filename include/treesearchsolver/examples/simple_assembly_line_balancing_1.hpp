/**
 * Simple assembly line balancing problem of type 1
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/include/orproblems/scheduling/simple_assembly_line_balancing_1.hpp
 *
 * TODO
 *
 */

#pragma once

#include "orproblems/scheduling/simple_assembly_line_balancing_1.hpp"

#include <memory>

namespace treesearchsolver
{
namespace simple_assembly_line_balancing_1
{

using namespace orproblems::simple_assembly_line_balancing_1;

using NodeId = int64_t;
using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Node
    {
        /** Parent node. */
        std::shared_ptr<Node> parent = nullptr;

        /** Array indicating for each job, if it has been processed. */
        std::vector<bool> jobs;

        /** Last processed job. */
        JobId job_id = -1;

        /** Number of jobs processed. */
        JobId number_of_jobs = 0;

        /** Number of stations in the partial solution. */
        StationId number_of_stations = 0;

        /** Current time at the last station of the partial solution. */
        Time current_station_time = 0;

        /** Sum of the processing time of all processed jobs. */
        Time processing_time_sum = 0;

        /** Bound. */
        StationId bound = -1;

        /** Guide. */
        double guide = 0;

        /** Unique id of the node. */
        NodeId node_id = -1;
    };

    BranchingScheme(
            const Instance& instance):
        instance_(instance)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->node_id = node_id_;
        node_id_++;
        r->jobs.resize(instance_.number_of_jobs(), false);
        r->current_station_time = instance_.cycle_time();
        return r;
    }

    inline std::vector<std::shared_ptr<Node>> children(
            const std::shared_ptr<Node>& parent) const
    {
        std::vector<std::shared_ptr<Node>> c;

        // Try to add a job in the current workstation.
        if (parent->number_of_stations > 0) {
            for (JobId job_id = 0;
                    job_id < instance_.number_of_jobs();
                    ++job_id) {

                // Check if the job has already been processed.
                if (parent->jobs[job_id])
                    continue;

                // Check if the job fits in the current station.
                Time p = instance_.job(job_id).processing_time;
                if (parent->current_station_time + p > instance_.cycle_time())
                    continue;

                // Check if the predecessors of the job have already been processed.
                bool ok = true;
                for (JobId predecessor_id: instance_.job(job_id).predecessors) {
                    if (!parent->jobs[predecessor_id]) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;

                // Compute new child.
                auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
                child->node_id = node_id_;
                node_id_++;
                child->parent = parent;
                child->job_id = job_id;
                child->number_of_jobs = parent->number_of_jobs + 1;
                child->jobs = parent->jobs;
                child->jobs[job_id] = true;
                child->processing_time_sum = parent->processing_time_sum + p;
                child->current_station_time = parent->current_station_time + p;
                child->number_of_stations = parent->number_of_stations;
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
                c.push_back(child);
            }
        }
        if (!c.empty())
            return c;

        // No job can be inserted in the current station.

        // First we look for a solitary job, that is a task which cannot share a
        // workstation with any other task.
        Time smallest_remaining_processing_time = instance_.cycle_time() + 1;
        // Longest valid remaining job. Valid in the sense that all its
        // predecessors have been scheduled.
        JobId longest_valid_remaining_job = -1;
        // Detect if there is a job with successors (for successor rule).
        bool has_job_with_successors = false;
        for (JobId job_id = 0;
                job_id < instance_.number_of_jobs();
                ++job_id) {

            // Check if the job has already been processed.
            if (parent->jobs[job_id])
                continue;

            Time p = instance_.job(job_id).processing_time;
            if (smallest_remaining_processing_time > p)
                smallest_remaining_processing_time = p;

            bool ok = true;
            for (JobId predecessor_id: instance_.job(job_id).predecessors) {
                if (!parent->jobs[predecessor_id]) {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            if (!instance_.job(job_id).successors.empty())
                has_job_with_successors = true;

            if (longest_valid_remaining_job == -1
                    || instance_.job(longest_valid_remaining_job).processing_time
                    < instance_.job(job_id).processing_time) {
                longest_valid_remaining_job = job_id;
            }
        }

        // If a solitary task has been found, only generate the child node
        // corresponding to its insertion in a new station.
        Time p = instance_.job(longest_valid_remaining_job).processing_time;
        if (instance_.job(longest_valid_remaining_job).processing_time
                + smallest_remaining_processing_time
                >= instance_.cycle_time()) {

            // Compute new child.
            auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
            child->node_id = node_id_;
            node_id_++;
            child->parent = parent;
            child->job_id = longest_valid_remaining_job;
            child->number_of_jobs = parent->number_of_jobs + 1;
            child->jobs = parent->jobs;
            child->jobs[longest_valid_remaining_job] = true;
            child->processing_time_sum = parent->processing_time_sum + p;
            child->current_station_time = p;
            child->number_of_stations = parent->number_of_stations + 1;
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
            c.push_back(child);
            return c;
        }

        for (JobId job_id = 0;
                job_id < instance_.number_of_jobs();
                ++job_id) {

            // Check if the job has already been processed.
            if (parent->jobs[job_id])
                continue;

            // Apply successor rule.
            if (has_job_with_successors
                    && instance_.job(job_id).successors.empty()) {
                continue;
            }

            // Check if the predecessors of the job have already been processed.
            bool ok = true;
            for (JobId predecessor_id: instance_.job(job_id).predecessors) {
                if (!parent->jobs[predecessor_id]) {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            Time p = instance_.job(job_id).processing_time;

            // Compute new child.
            auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
            child->node_id = node_id_;
            node_id_++;
            child->parent = parent;
            child->job_id = job_id;
            child->number_of_jobs = parent->number_of_jobs + 1;
            child->jobs = parent->jobs;
            child->jobs[job_id] = true;
            child->processing_time_sum = parent->processing_time_sum + p;
            child->current_station_time = p;
            child->number_of_stations = parent->number_of_stations + 1;
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
            c.push_back(child);
        }

        return c;
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        //if (node_1->number_of_jobs != node_2->number_of_jobs)
        //    return node_1->number_of_jobs < node_2->number_of_jobs;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1->node_id < node_2->node_id;
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

    std::shared_ptr<Node> goal_node(double value) const
    {
        auto node = std::shared_ptr<Node>(new BranchingScheme::Node());
        node->number_of_jobs = instance_.number_of_jobs();
        node->number_of_stations = value;
        return node;
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
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        return std::to_string(node->number_of_stations);
    }

    void solution_format(
            std::ostream &os,
            const std::shared_ptr<Node>& node,
            int verbosity_level) const
    {
        if (verbosity_level >= 1) {
            os
                << "Number of stations:  " << node->number_of_stations << std::endl
                ;
        }

        if (verbosity_level >= 2) {
            std::vector<std::vector<JobId>> stations(node->number_of_stations);
            std::vector<Time> times(node->number_of_stations, 0);
            for (auto node_tmp = node;
                    node_tmp->parent != nullptr;
                    node_tmp = node_tmp->parent) {
                stations[node_tmp->number_of_stations - 1].push_back(node_tmp->job_id);
                times[node_tmp->number_of_stations - 1] += instance_.job(node_tmp->job_id).processing_time;
            }
            os << std::endl
                << std::setw(12) << "Station"
                << std::setw(12) << "Time"
                << std::setw(12) << "# jobs"
                << std::endl
                << std::setw(12) << "-------"
                << std::setw(12) << "----"
                << std::setw(12) << "------"
                << std::endl;
            for (StationId station_id = 0;
                    station_id < node->number_of_stations;
                    ++station_id) {
                os
                    << std::setw(12) << station_id
                    << std::setw(12) << times[station_id]
                    << std::setw(12) << stations[station_id].size()
                    << std::endl;
            }
        }
    }

    inline void solution_write(
            const std::shared_ptr<Node>& node,
            const std::string& certificate_path) const
    {
        if (certificate_path.empty())
            return;
        std::ofstream file(certificate_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "Unable to open file \"" + certificate_path + "\".");
        }

        std::vector<std::vector<JobId>> stations(node->number_of_stations);
        for (auto node_tmp = node;
                node_tmp->parent != nullptr;
                node_tmp = node_tmp->parent) {
            stations[node_tmp->number_of_stations - 1].push_back(node_tmp->job_id);
        }

        for (StationId station_id = 0;
                station_id < node->number_of_stations;
                ++station_id) {
            std::reverse(stations[station_id].begin(), stations[station_id].end());
            file << stations[station_id].size();
            for (JobId job_id: stations[station_id])
                file << " " << job_id;
            file << std::endl;
        }
    }

private:

    /** Instance. */
    const Instance& instance_;

    mutable NodeId node_id_ = 0;

};

}
}
