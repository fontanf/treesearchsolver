/**
 * Permutation flow shop scheduling problem, total tardiness
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/permutationflowshopschedulingtt.hpp
 *
 * Tree search:
 * - Forward branching
 */

#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/permutationflowshopschedulingtt.hpp"

namespace treesearchsolver
{

namespace permutationflowshopschedulingtt
{

using namespace orproblems::permutationflowshopschedulingtt;

using GuideId = int64_t;

class BranchingSchemeForward
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        JobId job_id = -1;
        JobId number_of_jobs = 0;
        std::vector<Time> times;
        Time total_tardiness = 0;
        Time total_earliness = 0;
        Time idle_time = 0;
        double weighted_idle_time = 0;
        Time bound = 0;
        double guide = 0;
        JobId next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 0;
    };

    BranchingSchemeForward(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters) { }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto r = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        r->available_jobs.resize(n, true);
        r->times.resize(m, 0);
        r->bound = 0;
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        MachineId m = instance_.number_of_machines();
        auto father = node->father;
        node->available_jobs = father->available_jobs;
        node->available_jobs[node->job_id] = false;
        node->times = father->times;
        node->times[0] = father->times[0]
            + instance_.job(node->job_id).processing_times[0];
        for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
            if (node->times[machine_id - 1] > father->times[machine_id]) {
                node->times[machine_id] = node->times[machine_id - 1]
                    + instance_.job(node->job_id).processing_times[machine_id];
            } else {
                node->times[machine_id] = father->times[machine_id]
                    + instance_.job(node->job_id).processing_times[machine_id];
            }
        }
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        // Compute father's structures.
        if (father->times.empty())
            compute_structures(father);

        //if (father->next_child_pos == 0)
        //    std::cout << "father"
        //        << " j " << father->j
        //        << " n " << father->number_of_jobs
        //        << " ct " << father->total_completion_time
        //        << " it " << father->idle_time
        //        << " wit " << father->weighted_idle_time
        //        << " guide " << father->guide
        //        << std::endl;

        JobId job_id_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        // Check job availibility.
        if (!father->available_jobs[job_id_next])
            return nullptr;

        // Compute new child.
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto child = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        child->father = father;
        child->job_id = job_id_next;
        child->number_of_jobs = father->number_of_jobs + 1;
        child->idle_time = father->idle_time;
        child->weighted_idle_time = father->weighted_idle_time;
        Time t_prec = father->times[0]
            + instance_.job(job_id_next).processing_times[0];
        Time t = 0;
        for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
            if (t_prec > father->times[machine_id]) {
                Time idle_time = t_prec - father->times[machine_id];
                t = t_prec
                    + instance_.job(job_id_next).processing_times[machine_id];
                child->idle_time += idle_time;
                child->weighted_idle_time += ((double)father->number_of_jobs / n + 1) * (m - machine_id) * idle_time;
            } else {
                t = father->times[machine_id]
                    + instance_.job(job_id_next).processing_times[machine_id];
            }
            t_prec = t;
        }
        child->total_tardiness = father->total_tardiness
            + std::max((Time)0, t - instance_.job(job_id_next).due_date);
        child->total_earliness = father->total_earliness
            + std::max((Time)0, instance_.job(job_id_next).due_date - t);
        // Compute bound.
        child->bound = child->total_tardiness;
        // Compute guide.
        double alpha = (double)child->number_of_jobs / instance_.number_of_jobs();
        child->guide
            = (0.5 + alpha / 2) * child->total_tardiness
            + (1.0 - alpha / 2) * child->total_earliness
            + (1.0 - alpha) * child->weighted_idle_time;
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
        if (node_1->bound >= node_2->total_tardiness)
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
        return node_1->total_tardiness < node_2->total_tardiness;
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
        const BranchingSchemeForward& branching_scheme_;
        std::hash<std::vector<bool>> hasher;

        NodeHasher(const BranchingSchemeForward& branching_scheme):
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
        if (node_1->total_tardiness <= node_2->total_tardiness) {
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
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->total_tardiness
            << " (e" << node->total_earliness
            << " i" << node->idle_time
            << ")";
        return ss.str();
    }

    std::ostream& print_solution(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        if (node->times.empty())
            compute_structures(node);
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->number_of_jobs
                << " c " << node_tmp->times[instance_.number_of_machines() - 1]
                << " tt " << node_tmp->total_tardiness
                << " te " << node_tmp->total_earliness
                << " it " << node_tmp->idle_time
                << " wit " << node_tmp->weighted_idle_time
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->job_id
                << " dj " << instance_.job(node_tmp->job_id).due_date
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

        std::vector<JobId> jobs;
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            jobs.push_back(node_tmp->job_id);
        }
        std::reverse(jobs.begin(), jobs.end());
        for (JobId job_id: jobs)
            cert << job_id << " ";
    }


private:

    const Instance& instance_;
    Parameters parameters_;

};

class BranchingSchemeInsertion
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<JobId> jobs;
        JobPos pos = 0;
        JobId number_of_jobs = 0;
        Time makespan = 0;
        Time total_earliness = 0;
        Time total_tardiness = 0;
        double guide = 0;
        JobId next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 0;
        GuideId sort_criterion_id = 1;
    };

    BranchingSchemeInsertion(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_jobs_(instance.number_of_jobs()),
        processing_time_sums_(instance.number_of_jobs(), 0),
        heads_(instance.number_of_machines(), 0),
        completion_times_(instance.number_of_machines(), 0)
    {
        // Compute processing_time_sums_;
        for (JobId job_id = 0;
                job_id < instance.number_of_jobs();
                ++job_id) {
            const Job& job = instance.job(job_id);
            for (MachineId machine_id = 0;
                    machine_id < instance.number_of_machines();
                    ++machine_id) {
                processing_time_sums_[job_id] += job.processing_times[machine_id];
            }
        }

        // Initialize sorted_jobs_.
        std::iota(sorted_jobs_.begin(), sorted_jobs_.end(), 0);
        switch (parameters_.sort_criterion_id) {
        case 0: {
            std::random_shuffle(sorted_jobs_.begin(), sorted_jobs_.end());
            break;
        } case 1: {
            std::sort(sorted_jobs_.begin(), sorted_jobs_.end(),
                    [this](JobId job_id_1, JobId job_id_2) -> bool
                    {
                        return processing_time_sums_[job_id_1]
                            < processing_time_sums_[job_id_2];
                    });
            break;
        } default: {
            assert(false);
            break;
        }
        }
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        auto father = node->father;
        node->jobs.insert(
                node->jobs.end(),
                father->jobs.begin(),
                father->jobs.begin() + node->pos);
        node->jobs.push_back(sorted_jobs_[instance_.number_of_jobs() - node->number_of_jobs]);
        node->jobs.insert(
                node->jobs.end(),
                father->jobs.begin() + node->pos,
                father->jobs.end());
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));
        JobId n = instance_.number_of_jobs();
        MachineId m = instance_.number_of_machines();

        // Compute father's structures.
        if (father->father != nullptr && father->jobs.empty())
            compute_structures(father);

        JobId job_id_next = sorted_jobs_[instance_.number_of_jobs() - father->number_of_jobs - 1];
        JobPos pos = father->next_child_pos;
        // Update father
        father->next_child_pos++;

        auto child = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        child->father = father;
        child->pos = pos;
        child->number_of_jobs = father->number_of_jobs + 1;

        // Update heads_.
        if (pos == 0) {
            for (MachineId machine_id = 0; machine_id < m; ++machine_id)
                heads_[machine_id] = 0;
            head_total_earliness_ = 0;
            head_total_tardiness_ = 0;
        } else {
            JobId job_id = father->jobs[pos - 1];
            heads_[0] = heads_[0]
                + instance_.job(job_id).processing_times[0];
            for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                if (heads_[machine_id - 1] > heads_[machine_id]) {
                    heads_[machine_id] = heads_[machine_id - 1]
                        + instance_.job(job_id).processing_times[machine_id];
                } else {
                    heads_[machine_id] = heads_[machine_id]
                        + instance_.job(job_id).processing_times[machine_id];
                }
            }
            if (heads_[m - 1] < instance_.job(job_id).due_date)
                head_total_earliness_ += (instance_.job(job_id).due_date
                        - heads_[m - 1]);
            if (heads_[m - 1] > instance_.job(job_id).due_date)
                head_total_tardiness_ += (heads_[m - 1]
                        - instance_.job(job_id).due_date);
        }
        // Update completion_times_ of job_id_next.
        child->total_earliness = head_total_earliness_;
        child->total_tardiness = head_total_tardiness_;
        completion_times_[0] = heads_[0]
            + instance_.job(job_id_next).processing_times[0];
        for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
            if (completion_times_[machine_id - 1] > heads_[machine_id]) {
                completion_times_[machine_id] = completion_times_[machine_id - 1]
                    + instance_.job(job_id_next).processing_times[machine_id];
            } else {
                completion_times_[machine_id] = heads_[machine_id]
                    + instance_.job(job_id_next).processing_times[machine_id];
            }
        }
        if (completion_times_[m - 1] < instance_.job(job_id_next).due_date)
            child->total_earliness += (instance_.job(job_id_next).due_date
                    - completion_times_[m - 1]);
        if (completion_times_[m - 1] > instance_.job(job_id_next).due_date)
            child->total_tardiness += (completion_times_[m - 1]
                    - instance_.job(job_id_next).due_date);
        // Update completion_times_ of tail jobs.
        for (JobPos job_pos = pos; job_pos < (JobPos)father->jobs.size(); ++job_pos) {
            JobId job_id = father->jobs[job_pos];
            completion_times_[0] = completion_times_[0]
                + instance_.job(job_id).processing_times[0];
            for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                if (completion_times_[machine_id - 1] > completion_times_[machine_id]) {
                    completion_times_[machine_id] = completion_times_[machine_id - 1]
                        + instance_.job(job_id).processing_times[machine_id];
                } else {
                    completion_times_[machine_id] = completion_times_[machine_id]
                        + instance_.job(job_id).processing_times[machine_id];
                }
            }
            if (completion_times_[m - 1] < instance_.job(job_id).due_date)
                child->total_earliness += (instance_.job(job_id).due_date
                        - completion_times_[m - 1]);
            if (completion_times_[m - 1] > instance_.job(job_id).due_date)
                child->total_tardiness += (completion_times_[m - 1]
                        - instance_.job(job_id).due_date);
        }
        child->makespan = completion_times_[m - 1];

        double alpha = (double)child->number_of_jobs / n;
        child->guide
            = (0.5 + alpha / 2) * child->total_tardiness
            + (1.0 - alpha / 2) * child->total_earliness
            + (1.0 - alpha) * child->makespan;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == node->number_of_jobs + 1);
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
        if (node_1->total_tardiness >= node_2->total_tardiness)
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
        return node_1->total_tardiness < node_2->total_tardiness;
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

    const Instance& instance() const { return instance_; }

    struct NodeHasher
    {
        const BranchingSchemeInsertion& branching_scheme_;

        NodeHasher(const BranchingSchemeInsertion& branching_scheme):
            branching_scheme_(branching_scheme) { }

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

    inline NodeHasher node_hasher() const { return NodeHasher(*this); }

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
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->total_tardiness
            << " (e" << node->total_earliness
            << " c" << node->makespan
            << ")";
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
                << " c " << node_tmp->makespan
                << " te " << node_tmp->total_earliness
                << " tt " << node_tmp->total_tardiness
                << " pos " << node_tmp->pos
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

        if (node->father != nullptr && node->jobs.empty())
            compute_structures(node);
        for (JobId job_id: node->jobs)
            cert << job_id << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;
    mutable std::vector<JobId> sorted_jobs_;

    std::vector<Time> processing_time_sums_;
    mutable std::vector<Time> heads_;
    mutable Time head_total_earliness_;
    mutable Time head_total_tardiness_;
    mutable std::vector<Time> completion_times_;

};

}

}

