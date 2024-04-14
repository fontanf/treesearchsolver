/**
 * Permutation flow shop scheduling problem, makespan
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/permutation_flowshop_scheduling_makespan.hpp
 *
 * Tree search:
 * - Bidirectional branching
 * - Guides:
 *   - 0: bound
 *   - 1: idle time
 *   - 2: weighted idle time
 *   - 3: bound and weighted idle time
 *   - 4: gap, bound and weighted idle time
 */

#pragma once

#include "orproblems/scheduling/permutation_flowshop_scheduling_makespan.hpp"

#include <memory>
#include <sstream>

namespace treesearchsolver
{
namespace permutation_flowshop_scheduling_makespan
{

using namespace orproblems::permutation_flowshop_scheduling_makespan;

using NodeId = int64_t;
using GuideId = int64_t;

class BranchingSchemeBidirectional
{

public:

    struct NodeMachine
    {
        Time time_forward = 0;
        Time time_backward = 0;
        Time remaining_processing_time;
        Time idle_time_forward = 0;
        Time idle_time_backward = 0;
    };

    struct Node
    {
        /** Parent node. */
        std::shared_ptr<Node> parent = nullptr;

        /** Array indicating for each job, if it still available. */
        std::vector<bool> available_jobs;

        /** Position of the last job added in the solution. */
        bool forward = true;

        /** Last job added to the partial solution. */
        JobId job_id = -1;

        /** Number of jobs in the partial solution. */
        JobId number_of_jobs = 0;

        /** Machines. */
        std::vector<NodeMachine> machines;

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

        /** Unique id of the node. */
        NodeId node_id = -1;
    };

    struct Parameters
    {
        /** Enable bidirectional branching (otherwise forward branching). */
        bool bidirectional = true;

        /** Guide. */
        GuideId guide_id = 3;
    };

    BranchingSchemeBidirectional(
            const Instance& instance,
            const Parameters& parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto r = std::shared_ptr<Node>(new BranchingSchemeBidirectional::Node());
        r->node_id = node_id_;
        node_id_++;
        r->available_jobs.resize(n, true);
        r->machines.resize(m);
        for (JobId job_id = 0; job_id < n; ++job_id) {
            for (MachineId machine_id = 0; machine_id < m; ++machine_id) {
                r->machines[machine_id].remaining_processing_time
                    += instance_.processing_time(job_id, machine_id);
            }
        }
        r->bound = 0;
        for (JobId job_id = 0; job_id < n; ++job_id)
            r->bound += instance_.processing_time(job_id, m - 1);
        if (best_node_ == nullptr)
            best_node_ = r;
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        MachineId m = instance_.number_of_machines();
        auto parent = node->parent;
        node->available_jobs = parent->available_jobs;
        node->available_jobs[node->job_id] = false;
        node->machines = parent->machines;
        if (parent->forward) {
            node->machines[0].time_forward
                += instance_.processing_time(node->job_id, 0);
            node->machines[0].remaining_processing_time
                -= instance_.processing_time(node->job_id, 0);
            for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                if (node->machines[machine_id - 1].time_forward
                        > parent->machines[machine_id].time_forward) {
                    Time idle_time = node->machines[machine_id - 1].time_forward
                        - parent->machines[machine_id].time_forward;
                    node->machines[machine_id].time_forward
                        = node->machines[machine_id - 1].time_forward
                        + instance_.processing_time(node->job_id, machine_id);
                    node->machines[machine_id].idle_time_forward += idle_time;
                } else {
                    node->machines[machine_id].time_forward
                        += instance_.processing_time(node->job_id, machine_id);
                }
                node->machines[machine_id].remaining_processing_time
                    -= instance_.processing_time(node->job_id, machine_id);
            }
        } else {
            node->machines[m - 1].time_backward += instance_.processing_time(node->job_id, m - 1);
            node->machines[m - 1].remaining_processing_time -= instance_.processing_time(node->job_id, m - 1);
            for (MachineId machine_id = m - 2; machine_id >= 0; --machine_id) {
                if (node->machines[machine_id + 1].time_backward
                        > parent->machines[machine_id].time_backward) {
                    Time idle_time = node->machines[machine_id + 1].time_backward
                        - parent->machines[machine_id].time_backward;
                    node->machines[machine_id].time_backward
                        = node->machines[machine_id + 1].time_backward
                        + instance_.processing_time(node->job_id, machine_id);
                    node->machines[machine_id].idle_time_backward += idle_time;
                } else {
                    node->machines[machine_id].time_backward
                        += instance_.processing_time(node->job_id, machine_id);
                }
                node->machines[machine_id].remaining_processing_time
                    -= instance_.processing_time(node->job_id, machine_id);
            }
        }
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& parent) const
    {

        // Compute parent's structures.
        if (parent->next_child_pos == 0 && parent->parent != nullptr)
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

        // Determine wether to use forward or backward.
        if (parent->next_child_pos != 0) {
        } else if (!parameters_.bidirectional) {
            parent->forward = true;
        } else if (parent->parent == nullptr) {
            parent->forward = true;
        } else if (parent->parent->parent == nullptr) {
            parent->forward = false;
        } else {
            MachineId m = instance_.number_of_machines();
            JobId n = instance_.number_of_jobs();
            JobPos n_forward = 0;
            JobPos n_backward = 0;
            Time bound_forward = 0;
            Time bound_backward = 0;
            for (JobId job_id_next = 0; job_id_next < n; ++job_id_next) {
                if (!parent->available_jobs[job_id_next])
                    continue;
                // Forward.
                Time bf = 0;
                Time t_prec = parent->machines[0].time_forward
                    + instance_.processing_time(job_id_next, 0);
                Time t = 0;
                bf = std::max(bf,
                        t_prec
                        + parent->machines[0].remaining_processing_time
                        - instance_.processing_time(job_id_next, 0)
                        + parent->machines[0].time_backward);
                for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                    if (t_prec > parent->machines[machine_id].time_forward) {
                        t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    } else {
                        t = parent->machines[machine_id].time_forward
                            + instance_.processing_time(job_id_next, machine_id);
                    }
                    bf = std::max(
                            bf,
                            t + parent->machines[machine_id].remaining_processing_time
                            - instance_.processing_time(job_id_next, machine_id)
                            + parent->machines[machine_id].time_backward);
                    t_prec = t;
                }
                if (best_node_->number_of_jobs != n
                        || bf < best_node_->bound) {
                    n_forward++;
                    bound_forward += bf;
                }
                // Backward.
                Time bb = 0;
                t_prec
                    = parent->machines[m - 1].time_backward
                    + instance_.processing_time(job_id_next, m - 1);
                bb = std::max(bb,
                        parent->machines[m - 1].time_forward
                        + parent->machines[m - 1].remaining_processing_time
                        - instance_.processing_time(job_id_next, m - 1)
                        + t_prec);
                for (MachineId machine_id = m - 2; machine_id >= 0; --machine_id) {
                    if (t_prec > parent->machines[machine_id].time_backward) {
                        t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    } else {
                        t = parent->machines[machine_id].time_backward
                            + instance_.processing_time(job_id_next, machine_id);
                    }
                    bb = std::max(
                            bb,
                            parent->machines[machine_id].time_forward
                            + parent->machines[machine_id].remaining_processing_time
                            - instance_.processing_time(job_id_next, machine_id)
                            + t);
                    t_prec = t;
                }
                if (best_node_->number_of_jobs != n
                        || bb < best_node_->bound) {
                    n_backward++;
                    bound_backward += bb;
                }
            }
            if (n_forward < n_backward) {
                parent->forward = true;
            } else if (n_forward > n_backward) {
                parent->forward = false;
            } else if (bound_forward > bound_backward) {
                parent->forward = true;
            } else if (bound_forward < bound_backward) {
                parent->forward = false;
            } else {
                parent->forward = !parent->parent->forward;
            }
        }

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
        auto child = std::shared_ptr<Node>(new BranchingSchemeBidirectional::Node());
        child->node_id = node_id_;
        node_id_++;
        child->parent = parent;
        child->job_id = job_id_next;
        child->number_of_jobs = parent->number_of_jobs + 1;
        // Update machines and idle_time.
        child->idle_time = parent->idle_time;
        Time t = 0;
        Time t_prec = 0;
        if (parent->forward) {
            t_prec = parent->machines[0].time_forward
                + instance_.processing_time(job_id_next, 0);
            Time remaining_processing_time
                = parent->machines[0].remaining_processing_time
                - instance_.processing_time(job_id_next, 0);
            child->weighted_idle_time += (parent->machines[0].time_backward == 0)? 1:
                (double)parent->machines[0].idle_time_backward / parent->machines[0].time_backward;
            child->bound = std::max(child->bound,
                    t_prec
                    + remaining_processing_time
                    + parent->machines[0].time_backward);
            for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                Time machine_idle_time = parent->machines[machine_id].idle_time_forward;
                if (t_prec > parent->machines[machine_id].time_forward) {
                    Time idle_time = t_prec - parent->machines[machine_id].time_forward;
                    t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    machine_idle_time += idle_time;
                    child->idle_time += idle_time;
                } else {
                    t = parent->machines[machine_id].time_forward
                        + instance_.processing_time(job_id_next, machine_id);
                }
                Time remaining_processing_time
                    = parent->machines[machine_id].remaining_processing_time
                    - instance_.processing_time(job_id_next, machine_id);
                child->weighted_idle_time += (t == 0)? 1:
                    (double)machine_idle_time / t;
                child->weighted_idle_time += (parent->machines[machine_id].time_backward == 0)? 1:
                    (double)parent->machines[machine_id].idle_time_backward
                    / parent->machines[machine_id].time_backward;
                child->bound = std::max(
                        child->bound,
                        t + remaining_processing_time
                        + parent->machines[machine_id].time_backward);
                t_prec = t;
            }
        } else {
            t_prec = parent->machines[m - 1].time_backward
                + instance_.processing_time(job_id_next, m - 1);
            Time remaining_processing_time
                = parent->machines[m - 1].remaining_processing_time
                - instance_.processing_time(job_id_next, m - 1);
            child->weighted_idle_time += (parent->machines[m - 1].time_forward == 0)? 1:
                (double)parent->machines[m - 1].idle_time_forward / parent->machines[m - 1].time_forward;
            child->bound = std::max(child->bound,
                    parent->machines[m - 1].time_forward
                    + remaining_processing_time
                    + t_prec);
            for (MachineId machine_id = m - 2; machine_id >= 0; --machine_id) {
                Time machine_idle_time = parent->machines[machine_id].idle_time_backward;
                if (t_prec > parent->machines[machine_id].time_backward) {
                    Time idle_time = t_prec - parent->machines[machine_id].time_backward;
                    t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    machine_idle_time += idle_time;
                    child->idle_time += idle_time;
                } else {
                    t = parent->machines[machine_id].time_backward
                        + instance_.processing_time(job_id_next, machine_id);
                }
                Time remaining_processing_time
                    = parent->machines[machine_id].remaining_processing_time
                    - instance_.processing_time(job_id_next, machine_id);
                child->weighted_idle_time += (parent->machines[machine_id].time_forward == 0)? 1:
                    (double)parent->machines[machine_id].idle_time_forward
                    / parent->machines[machine_id].time_forward;
                child->weighted_idle_time += (t == 0)? 1:
                    (double)machine_idle_time / t;
                child->bound = std::max(
                        child->bound,
                        parent->machines[machine_id].time_forward
                        + remaining_processing_time + t);
                t_prec = t;
            }
        }
        // Compute guide.
        double alpha = (double)child->number_of_jobs / n;
        switch (parameters_.guide_id) {
        case 0: {
            child->guide = child->bound;
            break;
        } case 1: {
            child->guide = child->idle_time;
            break;
        } case 2: {
            child->guide = alpha * child->bound
                + (1.0 - alpha) * child->idle_time * child->number_of_jobs / m;
            break;
        } case 3: {
            child->guide = alpha * child->bound
                + (1.0 - alpha) * child->weighted_idle_time * child->bound;
            break;
        } case 4: {
            double a1 = (best_node_->number_of_jobs == instance_.number_of_jobs())?
                (double)(best_node_->bound) / (best_node_->bound - child->bound):
                1 - alpha;
            double a2 = (best_node_->number_of_jobs == instance_.number_of_jobs())?
                (double)(best_node_->bound - child->bound) / best_node_->bound:
                alpha;
            child->guide = a1 * child->bound
                + a2 * child->weighted_idle_time;
            break;
        } default: {
        }
        }
        if (better(child, best_node_))
            best_node_ = child;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        return (node->next_child_pos == instance_.number_of_jobs());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_jobs != node_2->number_of_jobs)
            return node_1->number_of_jobs < node_2->number_of_jobs;
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
        if (node_1->bound >= node_2->bound)
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
        return node_1->bound < node_2->bound;
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
        const BranchingSchemeBidirectional& branching_scheme_;
        std::hash<std::vector<bool>> hasher;

        NodeHasher(const BranchingSchemeBidirectional& branching_scheme):
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
        (void)node_1;
        (void)node_2;
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
        ss << node->bound;
        return ss.str();
    }

    void solution_format(
            const std::shared_ptr<Node>& node,
            std::ostream& os,
            int verbosity_level) const
    {
        if (node->machines.empty())
            compute_structures(node);

        if (verbosity_level >= 1) {
            os
                << "Makespan:   " << node->bound << std::endl
                << "Idle time:  " << node->idle_time << std::endl
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

        std::vector<JobId> jobs_forward;
        std::vector<JobId> jobs_backward;
        for (auto node_tmp = node;
                node_tmp->parent != nullptr;
                node_tmp = node_tmp->parent) {
            if (node_tmp->parent->forward) {
                jobs_forward.push_back(node_tmp->job_id);
            } else {
                jobs_backward.push_back(node_tmp->job_id);
            }
        }
        std::reverse(jobs_forward.begin(), jobs_forward.end());
        jobs_forward.insert(jobs_forward.end(), jobs_backward.begin(), jobs_backward.end());
        for (JobId job_id: jobs_forward)
            file << job_id << " ";
    }


private:

    /** Instance. */
    const Instance& instance_;

    /** Parameters. */
    Parameters parameters_;

    /** Best node. */
    mutable std::shared_ptr<Node> best_node_;

    mutable NodeId node_id_ = 0;

};

}
}
