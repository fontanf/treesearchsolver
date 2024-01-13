/**
 * Permutation flow shop scheduling problem, total completion time
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/permutationflowshopschedulingtct.hpp
 *
 * Tree search:
 * - Forward branching
 * - Guide:
 *   - 0: total completion time
 *   - 1: idle time
 *   - 2: weighted idle time
 *   - 3: total completion time and weighted idle time
 */

#pragma once

#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/permutationflowshopschedulingtct.hpp"

#include <memory>

namespace treesearchsolver
{
namespace permutationflowshopschedulingtct
{

using namespace orproblems::permutationflowshopschedulingtct;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Node
    {
        /** Parent node. */
        std::shared_ptr<Node> parent = nullptr;

        /** Array indicating for each job, if it still available. */
        std::vector<bool> available_jobs;

        /** Last job added to the partial solution. */
        JobId job_id = -1;

        /** Number of jobs in the partial solution. */
        JobId number_of_jobs = 0;

        /** For each machine, the current time. */
        std::vector<Time> times;

        /** Total completion time of the partial solution. */
        Time total_completion_time = 0;

        /** Idle time. */
        Time idle_time = 0;

        /** Weighted idle time. */
        double weighted_idle_time = 0;

        /** Bound. */
        Time bound = 0;

        /** Guide. */
        double guide = 0;

        /** Next child to generate. */
        JobId next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 2;
    };

    BranchingScheme(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_jobs.resize(n, true);
        r->times.resize(m, 0);
        r->bound = 0;
        for (JobId job_id = 0; job_id < n; ++job_id)
            r->bound += instance_.processing_time(job_id, m - 1);
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        MachineId m = instance_.number_of_machines();
        auto parent = node->parent;
        node->available_jobs = parent->available_jobs;
        node->available_jobs[node->job_id] = false;
        node->times = parent->times;
        node->times[0] = parent->times[0]
            + instance_.processing_time(node->job_id, 0);
        for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
            if (node->times[machine_id - 1] > parent->times[machine_id]) {
                node->times[machine_id] = node->times[machine_id - 1]
                    + instance_.processing_time(node->job_id, machine_id);
            } else {
                node->times[machine_id] = parent->times[machine_id]
                    + instance_.processing_time(node->job_id, machine_id);
            }
        }
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& parent) const
    {
        assert(!infertile(parent));
        assert(!leaf(parent));

        // Compute parent's structures.
        if (parent->times.empty())
            compute_structures(parent);

        //if (parent->next_child_pos == 0)
        //    std::cout << "parent"
        //        << " j " << parent->j
        //        << " n " << parent->number_of_jobs
        //        << " ct " << parent->total_completion_time
        //        << " it " << parent->idle_time
        //        << " wit " << parent->weighted_idle_time
        //        << " guide " << parent->guide
        //        << std::endl;

        // Get the next job to process.
        JobId job_id_next = parent->next_child_pos;

        // Update parent
        parent->next_child_pos++;

        // Check job availibility.
        if (!parent->available_jobs[job_id_next])
            return nullptr;

        // Compute new child.
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->parent = parent;
        child->job_id = job_id_next;
        child->number_of_jobs = parent->number_of_jobs + 1;
        child->idle_time = parent->idle_time;
        child->weighted_idle_time = parent->weighted_idle_time;
        Time t_prec = parent->times[0]
            + instance_.processing_time(job_id_next, 0);
        Time t = 0;
        for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
            if (t_prec > parent->times[machine_id]) {
                Time idle_time = t_prec - parent->times[machine_id];
                t = t_prec + instance_.processing_time(job_id_next, machine_id);
                child->idle_time += idle_time;
                child->weighted_idle_time += ((double)parent->number_of_jobs / n + 1) * (m - machine_id) * idle_time;
            } else {
                t = parent->times[machine_id]
                    + instance_.processing_time(job_id_next, machine_id);
            }
            t_prec = t;
        }
        child->total_completion_time = parent->total_completion_time + t;
        // Compute bound.
        child->bound = parent->bound
            + (n - parent->number_of_jobs) * (t - parent->times[m - 1])
            - instance_.processing_time(job_id_next, m - 1);
        // Compute guide.
        double alpha = (double)child->number_of_jobs / instance_.number_of_jobs();
        switch (parameters_.guide_id) {
        case 0: {
            child->guide = child->bound;
            break;
        } case 1: {
            child->guide = child->idle_time;
            break;
        } case 2: {
            child->guide = alpha * child->total_completion_time
                + (1.0 - alpha) * child->idle_time * child->number_of_jobs / m;
            break;
        } case 3: {
            //child->guide = alpha * child->total_completion_time
            //    + (1.0 - alpha) * (child->weighted_idle_time + m * child->idle_time) / 2;
            child->guide = alpha * child->total_completion_time
                + (1.0 - alpha) * (child->weighted_idle_time / m + child->idle_time) / 2 * child->number_of_jobs / m;
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
        return (node->next_child_pos == instance_.number_of_jobs());
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
        if (node_1->bound >= node_2->total_completion_time)
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
            const std::shared_ptr<Node>& node) const
    {
        (void)node;
        return false;
    }

    const Instance& instance() const { return instance_; }

    struct NodeHasher
    {
        const BranchingScheme& branching_scheme_;
        std::hash<std::vector<bool>> hasher;

        NodeHasher(const BranchingScheme& branching_scheme):
            branching_scheme_(branching_scheme) { }

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (node_1->available_jobs != node_2->available_jobs)
                return false;
            return true;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher(node->available_jobs);
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(*this); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->total_completion_time <= node_2->total_completion_time) {
            bool dominates = true;
            for (MachineId machine_id = 0;
                    machine_id < instance_.number_of_machines();
                    ++machine_id) {
                if (node_1->times[machine_id] > node_2->times[machine_id]) {
                    dominates = false;
                    break;
                }
            }
            if (dominates)
                return true;
        }

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
        std::stringstream ss;
        ss << node->total_completion_time;
        return ss.str();
    }

    void solution_format(
            const std::shared_ptr<Node>& node,
            std::ostream& os,
            int verbosity_level) const
    {
        if (node->times.empty())
            compute_structures(node);

        if (verbosity_level >= 1) {
            os
                << "Total completion time:  " << node->total_completion_time << std::endl
                << "Idle time:              " << node->idle_time << std::endl
                ;
        }
    }

    inline void solution_write(
            const std::shared_ptr<Node>& node,
            std::string certificate_path) const
    {
        if (certificate_path.empty())
            return;
        std::ofstream file(certificate_path);
        if (!file.good()) {
            throw std::runtime_error(
                    "Unable to open file \"" + certificate_path + "\".");
        }

        std::vector<JobId> jobs;
        for (auto node_tmp = node;
                node_tmp->parent != nullptr;
                node_tmp = node_tmp->parent) {
            jobs.push_back(node_tmp->job_id);
        }
        std::reverse(jobs.begin(), jobs.end());
        for (JobId job_id: jobs)
            file << job_id << " ";
    }


private:

    /** Instance. */
    const Instance& instance_;

    /** Parameters. */
    Parameters parameters_;

};

}
}
