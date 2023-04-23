/**
 * No-wait job shop scheduling problem, Makespan.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/nowaitjobshopschedulingmakespan.hpp
 *
 * Tree search:
 * - Forward branching
 */

#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/nowaitjobshopschedulingmakespan.hpp"

namespace treesearchsolver
{

namespace nowaitjobshopschedulingmakespan
{

using namespace orproblems::nowaitjobshopschedulingmakespan;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<Time> starts;
        JobId job_id = -1; // New job.
        Time start = -1; // Start date of new job.

        JobId number_of_jobs = 0;
        Time processing_time = 0;
        Time makespan_sum = 0;
        Time makespan = 0;
        Time starts_sum = 0;

        std::vector<lib_interval_tree::interval_tree_t<Time>> intervals;
        std::vector<Time> possible_starts;
        std::vector<Time> makespans;

        double guide = 0;
        JobPos next_job = -1;
        OperationPos next_start_pos = 1;
        Time number_of_children = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 0;
    };

    BranchingScheme(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters) { }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->starts.resize(n, -1);
        r->intervals.resize(m);
        r->makespans.resize(m, 0);
        r->possible_starts = {0};
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        auto father = node->father;
        node->intervals = father->intervals;
        node->makespans = father->makespans;
        Time current_time = node->start;
        for (OperationPos k = 0; k < instance_.number_of_operations(node->job_id); ++k) {
            MachineId machine_id = instance_.operation(node->job_id, k).machine_id;
            Time completion_time = current_time
                + instance_.operation(node->job_id, k).processing_time;
            node->intervals[machine_id].insert_overlap({current_time, completion_time});
            if (node->makespans[machine_id] < completion_time)
                node->makespans[machine_id] = completion_time;
            current_time = completion_time;
        }
        //MachineId m = instance_.number_of_machines();
        //for (MachineId i = 0; i < m; ++i) {
        //    std::cout << "i " << i << ": ";
        //    for (const auto& interval: node->intervals[i])
        //        std::cout << " " << interval.low() << " " << interval.high() << ";";
        //    std::cout << std::endl;
        //}
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        //if (father->intervals.empty())
        //    std::cout << "father"
        //        << " j " << father->j
        //        << " start " << father->start
        //        << " n " << father->number_of_jobs
        //        << " makespan_sum " << father->makespan_sum
        //        << " makespan " << father->makespan
        //        << " guide " << father->guide
        //        << std::endl;

        // Compute father's structures.
        if (father->intervals.empty())
            compute_structures(father);

        if (father->next_start_pos >= (Time)father->possible_starts.size()) {
            father->next_job++;
            father->possible_starts.clear();
            father->next_start_pos = 0;
        }

        JobId job_id_next = father->next_job;

        // Check job availibility.
        if (father->starts[job_id_next] >= 0)
            return nullptr;

        // Compute possible starts.
        if (father->possible_starts.empty()) {
            father->possible_starts.push_back({0});
            Time p = 0;
            for (OperationPos k = 0; k < instance_.number_of_operations(job_id_next); ++k) {
                MachineId machine_id = instance_.operation(job_id_next, k).machine_id;
                for (const auto& interval: father->intervals[machine_id])
                    if (interval.high() - p > 0)
                        father->possible_starts.push_back(interval.high() - p);
                p += instance_.operation(job_id_next, k).processing_time;
            }
            std::sort(father->possible_starts.begin(), father->possible_starts.end());
            std::unique(father->possible_starts.begin(), father->possible_starts.end());
            //std::cout << "number of possible starts: " << father->possible_starts.size() << std::endl;
        }
        if (father->possible_starts.empty())
            return nullptr;

        Time start = father->possible_starts[father->next_start_pos];
        father->next_start_pos++;

        // Check if start is feasible.
        Time current_time = start;
        Time processing_time = father->processing_time;
        Time makespan_sum = 0;
        Time makespan = 0;
        for (OperationPos k = 0; k < instance_.number_of_operations(job_id_next); ++k) {
            Time pij = instance_.operation(job_id_next, k).processing_time;
            Time completion_time = current_time + pij;
            MachineId i = instance_.operation(job_id_next, k).machine_id;
            if (father->intervals[i].overlap_find({current_time, completion_time}, true)
                    != father->intervals[i].end())
                return nullptr;
            processing_time += pij;
            Time makespan_cur = std::max(
                    completion_time,
                    father->makespans[i]);
            makespan_sum += makespan_cur;
            makespan = std::max(makespan, makespan_cur);
            current_time = completion_time;
        }

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->job_id = job_id_next;
        child->start = start;
        child->number_of_jobs = father->number_of_jobs + 1;
        child->processing_time = processing_time;
        child->makespan_sum = makespan_sum;
        child->makespan = makespan;
        child->starts = father->starts;
        child->starts[job_id_next] = start;
        child->starts_sum += start;
        // Compute guide.
        //JobPos n = instance_.number_of_jobs();
        //MachineId m = instance_.number_of_machines();
        //double alpha = (double)(child->number_of_jobs - 1) / (n - 1);
        child->guide = (double)(child->starts_sum) / child->processing_time;
        //child->guide
        //    = (1 - alpha) * (double)(child->starts_sum) / child->processing_time
        //    + alpha * child->makespan;
        //child->guide
        //    = (1 - alpha) * (double)(child->makespan_sum - child->processing_time) / m
        //    + alpha * child->makespan;
        father->number_of_children++;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_job == instance_.number_of_jobs() - 1
                && node->next_start_pos == (Time)node->possible_starts.size());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->number_of_jobs != node_2->number_of_jobs)
            return node_1->number_of_jobs < node_2->number_of_jobs;
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
        if (node_1->makespan >= node_2->makespan)
            return true;
        return false;
    }

    /*
     * Solution pool.
     */

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_jobs != instance_.number_of_jobs())
            return false;
        if (node_2->number_of_jobs != instance_.number_of_jobs())
            return true;
        return node_1->makespan < node_2->makespan;
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

    const Instance& instance() const { return instance_; }

    struct NodeHasher
    {
        const BranchingScheme& branching_scheme_;
        std::hash<Time> hasher;

        NodeHasher(const BranchingScheme& branching_scheme):
            branching_scheme_(branching_scheme) { }

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (node_1->starts != node_2->starts)
                return false;
            return true;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = 0;
            for (Time s: node->starts)
                optimizationtools::hash_combine(hash, hasher(s));
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(*this); }

    inline bool dominates(
            const std::shared_ptr<Node>&,
            const std::shared_ptr<Node>&) const
    {
        return true;
    }

    /*
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->makespan;
        return ss.str();
    }

    std::ostream& print_solution(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->number_of_jobs
                << " p " << node_tmp->processing_time
                << " makespan_sum " << node_tmp->makespan_sum
                << " makesean " << node_tmp->makespan
                << " j " << node_tmp->job_id
                << " s " << node_tmp->start
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

        JobId n = instance_.number_of_jobs();
        std::vector<Time> starts(n, -1);
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            starts[node_tmp->job_id] = node_tmp->start;
        }
        for (JobId job_id = 0; job_id < n; ++job_id)
            cert << starts[job_id] << " ";
    }


private:

    const Instance& instance_;
    Parameters parameters_;

};

}

}

