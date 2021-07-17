#pragma once

/**
 * Permutation flow shop scheduling problem, Total tardiness.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/permutationflowshopschedulingtt.hpp
 *
 * Tree search:
 * - Forward branching
 */

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

#include "orproblems/permutationflowshopschedulingtt.hpp"

namespace treesearchsolver
{

namespace permutationflowshopschedulingtt
{

using namespace orproblems::permutationflowshopschedulingtt;

typedef int64_t GuideId;

class BranchingSchemeForward
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        JobId j = -1;
        JobId job_number = 0;
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

    BranchingSchemeForward(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.machine_number();
        JobId n = instance_.job_number();
        auto r = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        r->available_jobs.resize(n, true);
        r->times.resize(m, 0);
        r->bound = 0;
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        MachineId m = instance_.machine_number();
        auto father = node->father;
        node->available_jobs = father->available_jobs;
        node->available_jobs[node->j] = false;
        node->times = father->times;
        node->times[0] = father->times[0]
            + instance_.job(node->j).processing_times[0];
        for (MachineId i = 1; i < m; ++i) {
            if (node->times[i - 1] > father->times[i]) {
                node->times[i] = node->times[i - 1]
                    + instance_.job(node->j).processing_times[i];
            } else {
                node->times[i] = father->times[i]
                    + instance_.job(node->j).processing_times[i];
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
        //        << " n " << father->job_number
        //        << " ct " << father->total_completion_time
        //        << " it " << father->idle_time
        //        << " wit " << father->weighted_idle_time
        //        << " guide " << father->guide
        //        << std::endl;

        JobId j_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        // Check job availibility.
        if (!father->available_jobs[j_next])
            return nullptr;

        // Compute new child.
        MachineId m = instance_.machine_number();
        JobId n = instance_.job_number();
        auto child = std::shared_ptr<Node>(new BranchingSchemeForward::Node());
        child->father = father;
        child->j = j_next;
        child->job_number = father->job_number + 1;
        child->idle_time = father->idle_time;
        child->weighted_idle_time = father->weighted_idle_time;
        Time t_prec = father->times[0]
            + instance_.job(j_next).processing_times[0];
        Time t = 0;
        for (MachineId i = 1; i < m; ++i) {
            if (t_prec > father->times[i]) {
                Time idle_time = t_prec - father->times[i];
                t = t_prec
                    + instance_.job(j_next).processing_times[i];
                child->idle_time += idle_time;
                child->weighted_idle_time += ((double)father->job_number / n + 1) * (m - i) * idle_time;
            } else {
                t = father->times[i]
                    + instance_.job(j_next).processing_times[i];
            }
            t_prec = t;
        }
        child->total_tardiness = father->total_tardiness
            + std::max((Time)0, t - instance_.job(j_next).due_date);
        child->total_earliness = father->total_earliness
            + std::max((Time)0, instance_.job(j_next).due_date - t);
        // Compute bound.
        child->bound = child->total_tardiness;
        // Compute guide.
        double alpha = (double)child->job_number / instance_.job_number();
        switch (parameters_.guide_id) {
        case 0: {
            child->guide
                = (0.5 + alpha / 2) * child->total_tardiness
                + (1.0 - alpha / 2) * child->total_earliness
                + (1.0 - alpha) * child->weighted_idle_time;
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
        return (node->next_child_pos == instance_.job_number());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->job_number != node_2->job_number)
            return node_1->job_number < node_2->job_number;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->job_number == instance_.job_number();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->job_number != instance_.job_number())
            return false;
        if (node_1->bound >= node_2->total_tardiness)
            return true;
        return false;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->job_number != instance_.job_number())
            return false;
        if (node_2->job_number != instance_.job_number())
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
            for (MachineId i = 0; i < instance_.machine_number(); ++i) {
                if (node_1->times[i] > node_2->times[i]) {
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
        if (node->job_number != instance_.job_number())
            return "";
        std::stringstream ss;
        ss << node->total_tardiness
            << " (e" << node->total_earliness
            << " i" << node->idle_time
            << ")";
        return ss.str();
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        if (node->times.empty())
            compute_structures(node);
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->job_number
                << " c " << node_tmp->times[instance_.machine_number() - 1]
                << " tt " << node_tmp->total_tardiness
                << " te " << node_tmp->total_earliness
                << " it " << node_tmp->idle_time
                << " wit " << node_tmp->weighted_idle_time
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->j
                << " dj " << instance_.job(node_tmp->j).due_date
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

        std::vector<JobId> jobs;
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            jobs.push_back(node_tmp->j);
        std::reverse(jobs.begin(), jobs.end());
        for (JobId j: jobs)
            cert << j << " ";
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
        JobId job_number = 0;
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

    BranchingSchemeInsertion(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_jobs_(instance.job_number()),
        processing_time_sums_(instance.job_number(), 0),
        heads_(instance.machine_number(), 0),
        completion_times_(instance.machine_number(), 0)
    {
        // Compute processing_time_sums_;
        for (JobId j = 0; j < instance.job_number(); ++j)
            for (MachineId i = 0; i < instance.machine_number(); ++i)
                processing_time_sums_[j] += instance.job(j).processing_times[i];

        // Initialize sorted_jobs_.
        std::iota(sorted_jobs_.begin(), sorted_jobs_.end(), 0);
        switch (parameters_.sort_criterion_id) {
        case 0: {
            std::random_shuffle(sorted_jobs_.begin(), sorted_jobs_.end());
            break;
        } case 1: {
            std::sort(sorted_jobs_.begin(), sorted_jobs_.end(),
                    [this](JobId j1, JobId j2) -> bool
                    {
                        return processing_time_sums_[j1] < processing_time_sums_[j2];
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
        node->jobs.push_back(sorted_jobs_[instance_.job_number() - node->job_number]);
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
        JobId n = instance_.job_number();
        MachineId m = instance_.machine_number();

        // Compute father's structures.
        if (father->father != nullptr && father->jobs.empty())
            compute_structures(father);

        JobId j_next = sorted_jobs_[instance_.job_number() - father->job_number - 1];
        JobPos pos = father->next_child_pos;
        // Update father
        father->next_child_pos++;

        auto child = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        child->father = father;
        child->pos = pos;
        child->job_number = father->job_number + 1;

        // Update heads_.
        if (pos == 0) {
            for (MachineId i = 0; i < m; ++i)
                heads_[i] = 0;
            head_total_earliness_ = 0;
            head_total_tardiness_ = 0;
        } else {
            JobId j = father->jobs[pos - 1];
            heads_[0] = heads_[0]
                + instance_.job(j).processing_times[0];
            for (MachineId i = 1; i < m; ++i) {
                if (heads_[i - 1] > heads_[i]) {
                    heads_[i] = heads_[i - 1]
                        + instance_.job(j).processing_times[i];
                } else {
                    heads_[i] = heads_[i]
                        + instance_.job(j).processing_times[i];
                }
            }
            if (heads_[m - 1] < instance_.job(j).due_date)
                head_total_earliness_ += (instance_.job(j).due_date
                        - heads_[m - 1]);
            if (heads_[m - 1] > instance_.job(j).due_date)
                head_total_tardiness_ += (heads_[m - 1]
                        - instance_.job(j).due_date);
        }
        // Update completion_times_ of j_next.
        child->total_earliness = head_total_earliness_;
        child->total_tardiness = head_total_tardiness_;
        completion_times_[0] = heads_[0]
            + instance_.job(j_next).processing_times[0];
        for (MachineId i = 1; i < m; ++i) {
            if (completion_times_[i - 1] > heads_[i]) {
                completion_times_[i] = completion_times_[i - 1]
                    + instance_.job(j_next).processing_times[i];
            } else {
                completion_times_[i] = heads_[i]
                    + instance_.job(j_next).processing_times[i];
            }
        }
        if (completion_times_[m - 1] < instance_.job(j_next).due_date)
            child->total_earliness += (instance_.job(j_next).due_date
                    - completion_times_[m - 1]);
        if (completion_times_[m - 1] > instance_.job(j_next).due_date)
            child->total_tardiness += (completion_times_[m - 1]
                    - instance_.job(j_next).due_date);
        // Update completion_times_ of tail jobs.
        for (JobPos j_pos = pos; j_pos < (JobPos)father->jobs.size(); ++j_pos) {
            JobId j = father->jobs[j_pos];
            completion_times_[0] = completion_times_[0]
                + instance_.job(j).processing_times[0];
            for (MachineId i = 1; i < m; ++i) {
                if (completion_times_[i - 1] > completion_times_[i]) {
                    completion_times_[i] = completion_times_[i - 1]
                        + instance_.job(j).processing_times[i];
                } else {
                    completion_times_[i] = completion_times_[i]
                        + instance_.job(j).processing_times[i];
                }
            }
            if (completion_times_[m - 1] < instance_.job(j).due_date)
                child->total_earliness += (instance_.job(j).due_date
                        - completion_times_[m - 1]);
            if (completion_times_[m - 1] > instance_.job(j).due_date)
                child->total_tardiness += (completion_times_[m - 1]
                        - instance_.job(j).due_date);
        }
        child->makespan = completion_times_[m - 1];

        double alpha = (double)child->job_number / n;
        switch (parameters_.guide_id) {
        case 0: {
            child->guide
                = (0.5 + alpha / 2) * child->total_tardiness
                + (1.0 - alpha / 2) * child->total_earliness
                + (1.0 - alpha) * child->makespan;
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
        return (node->next_child_pos == node->job_number + 1);
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        if (node_1->job_number != node_2->job_number)
            return node_1->job_number < node_2->job_number;
        if (node_1->guide != node_2->guide)
            return node_1->guide < node_2->guide;
        return node_1.get() < node_2.get();
    }

    inline bool leaf(
            const std::shared_ptr<Node>& node) const
    {
        return node->job_number == instance_.job_number();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_2->job_number != instance_.job_number())
            return false;
        if (node_1->total_tardiness >= node_2->total_tardiness)
            return true;
        return false;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->job_number != instance_.job_number())
            return false;
        if (node_2->job_number != instance_.job_number())
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
        if (node->job_number != instance_.job_number())
            return "";
        std::stringstream ss;
        ss << node->total_tardiness
            << " (e" << node->total_earliness
            << " c" << node->makespan
            << ")";
        return ss.str();
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->job_number
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
            std::string filepath) const
    {
        if (filepath.empty())
            return;
        std::ofstream cert(filepath);
        if (!cert.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << filepath << "\"" << "\033[0m" << std::endl;
            return;
        }

        if (node->father != nullptr && node->jobs.empty())
            compute_structures(node);
        for (JobId j: node->jobs)
            cert << j << " ";
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

