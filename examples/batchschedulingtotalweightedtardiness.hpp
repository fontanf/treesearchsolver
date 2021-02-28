#pragma once

#include "optimizationtools/info.hpp"

/**
 * Single machine batch scheduling problem, Total weighted tardiness.
 *
 * Input:
 * - n jobs with processing time pⱼ, size sⱼ, release date rⱼ, due date dⱼ and
 *   weight wⱼ (j = 1..n)
 * - a batch capacity Q
 * Problem:
 * - partition the jobs into batches and sequence the batches such that:
 *   - each job must be in exactly one of the batches)
 *   - the processing time of a batch is equal to the longest processing time
 *     among all jobs it contains
 *   - the total size of the jobs in a batch does not exceed its capacity
 *   - each job starts after its release date (each jobs start at the start
 *     date of its batch)
 * Objective:
 * - minimize the total weighted tardiness of the schedule
 *
 * Notes:
 * - If the end of the last batch can increase, then the tardiness of the jobs
 *   of this last batch might increase as well. Therefore, in order to be able
 *   to compute the current total weighted tardiness of a partial schedule,
 *   the duration of a batch is fixed by the first job added to it.
 * - However, it is not dominant to always start a batch with the longest job
 *   of the batch. In particular, when the start of the batch is strictly
 *   greater than the end of the previous batch (because of a release date).
 *   Therefore, we allow increasing the duration of a batch if it only contains
 *   one job yet and there is idle time after the previous batch. Furthermore,
 *   we mark the nodes with a possibly increasing batch as not comparable.
 * - About dominances:
 *
 *   Case 1:
 *
 *   j   p   r   s
 *   0   2   0   19
 *   1   1   0   19
 *   ...
 *   In this case, there is no dominance between [0] [1] and [1] [0]
 *
 *   Example 1:
 *   j   p   r   s
 *   0   2   0   19
 *   1   1   0   19
 *   2   1   2   1
 *
 *   Example 2:
 *   j   p   r   s
 *   0   2   0   19
 *   1   1   0   19
 *   2   2   0   1
 *
 *   Case 2:
 *
 *   j   p   r   s
 *   0   4   0  10
 *   1   1   0  10
 *   2   3   0  10
 *   ...
 *   In this case, there is no dominance between [0,1] [2] and [0,2] [1].
 *
 *   Example 1:
 *   j   p   r   s
 *   0   4   0   10
 *   1   1   0   10
 *   2   3   0   10
 *   3   2   0   10
 *   [0,1] [2,3]
 *   [0,2] [1,3] is not possible because p3 > p1.
 *
 *   Example 2:
 *   j   p   r   s
 *   0   4   0   10
 *   1   1   0   10
 *   2   3   0   10
 *   3   1   0   10
 *   [0,2] [1,3]
 *
 * Tree search:
 * - forward branching
 * - guide: bound
 *
 */

namespace treesearchsolver
{

namespace batchschedulingtotalweightedtardiness
{

typedef int64_t JobId;
typedef int64_t JobPos;
typedef int64_t Time;
typedef int64_t Weight;
typedef int64_t Size;
typedef int64_t Area;

struct Job
{
    JobId id;
    Time processing_time;
    Time release_date;
    Time due_date;
    Size size;
    Weight weight;
};

class Instance
{

public:

    Instance() { }
    void add_job(Time p, Time r, Time d, Size s, Weight w)
    {
        Job job;
        job.id = jobs_.size();
        job.processing_time = p;
        job.release_date = r;
        job.due_date = d;
        job.size = s;
        job.weight = w;
        jobs_.push_back(job);
    }
    void set_capacity(Size q) { capacity_ = q; }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "queiroga2020") {
            read_queiroga2020(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline JobId job_number() const { return jobs_.size(); }
    inline const Job& job(JobId j) const { return jobs_[j]; }
    inline Size capacity() const { return capacity_; }

private:

    void read_queiroga2020(std::ifstream& file)
    {
        JobId n = -1;
        Size c = -1;
        file >> n >> c;
        set_capacity(c);

        Time p, r, d;
        Size s;
        Weight w;
        for (JobId j = 0; j < n; ++j) {
            file >> p >> d >> s >> w >> r;
            add_job(p, r, d, s, w);
        }
    }

    std::vector<Job> jobs_;
    Size capacity_ = 0;

};

class BranchingScheme
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> jobs;
        JobId j = -1;
        bool new_batch = false;
        JobId job_number = 0;
        Time current_batch_start = 0;
        Time current_batch_end = 0;
        Size current_batch_size = 0;
        Weight total_weighted_tardiness = 0;
        Weight bound = 0;
        double guide = 0;
        JobPos next_child_pos = 0;
        Time earliest_end_date = -1;
    };

    BranchingScheme(const Instance& instance):
        instance_(instance),
        sorted_jobs_(instance.job_number())
    {
        // Initialize sorted_jobs_.
        std::iota(sorted_jobs_.begin(), sorted_jobs_.end(), 0);
        sort(sorted_jobs_.begin(), sorted_jobs_.end(),
                [&instance](JobId j1, JobId j2) -> bool
                {
                    return instance.job(j1).release_date < instance.job(j2).release_date;
                });
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->jobs.resize(instance_.job_number(), false);
        r->next_child_pos = instance_.job_number();
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        JobId j_next = sorted_jobs_[father->next_child_pos % instance_.job_number()];
        bool new_batch = (father->next_child_pos >= instance_.job_number());
        bool x = (father->new_batch && father->current_batch_start > father->father->current_batch_end);
        // Update father
        father->next_child_pos++;

        if (father->jobs[j_next])
            return nullptr;
        Time r = instance_.job(j_next).release_date;
        if (!new_batch && r > father->current_batch_start)
            return nullptr;
        Time p = instance_.job(j_next).processing_time;
        if (!new_batch
                && !x
                && (p > instance_.job(father->j).processing_time
                    || (p == instance_.job(father->j).processing_time && j_next < father->j)))
            return nullptr;
        Size s = instance_.job(j_next).size;
        if (!new_batch
                && father->current_batch_size + s > instance_.capacity())
            return nullptr;
        if (new_batch
                && father->earliest_end_date != -1
                && std::max(father->current_batch_end, r) >= father->earliest_end_date)
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->j = j_next;
        child->new_batch = new_batch;
        child->job_number = father->job_number + 1;
        child->jobs = father->jobs;
        child->jobs[j_next] = true;
        if (!new_batch) {
            child->current_batch_start = father->current_batch_start;
            if (father->current_batch_end >= father->current_batch_start + p) {
                child->current_batch_end = father->current_batch_end;
            } else {
                child->current_batch_end = father->current_batch_start + p;
            }
            child->current_batch_size = father->current_batch_size + s;
        } else {
            child->current_batch_start = std::max(father->current_batch_end, r);
            child->current_batch_end = child->current_batch_start + p;
            child->current_batch_size = s;
        }
        if (father->earliest_end_date == -1
                || father->earliest_end_date > child->current_batch_end)
            father->earliest_end_date = child->current_batch_end;

        // Compute total_waited_tardiness.
        auto node_tmp = child;
        child->total_weighted_tardiness = 0;
        Time current_batch_end = child->current_batch_end;
        while (node_tmp->father != nullptr) {
            JobId j = node_tmp->j;
            if (current_batch_end > instance_.job(j).due_date)
                child->total_weighted_tardiness += instance_.job(j).weight
                    * (current_batch_end - instance_.job(j).due_date);
            if (node_tmp->new_batch)
                current_batch_end = node_tmp->father->current_batch_end;
            node_tmp = node_tmp->father;
        }

        // Compute bound.
        bool x_child = (child->new_batch && child->current_batch_start > child->father->current_batch_end);
        child->bound = child->total_weighted_tardiness;
        for (JobId j = 0; j < instance_.job_number(); ++j) {
            if (!child->jobs[j]) {
                Time e = -1;
                if (child->current_batch_size + instance_.job(j).size <= instance_.capacity()
                        && instance_.job(j).release_date <= child->current_batch_start
                        && (x_child
                            || instance_.job(j).processing_time < p
                            || (instance_.job(j).processing_time == p && j > j_next))) {
                    e = std::max(child->current_batch_end, child->current_batch_start + instance_.job(j).processing_time);
                } else {
                    e = std::max(child->current_batch_end, instance_.job(j).release_date) + instance_.job(j).processing_time;
                }
                if (e > instance_.job(j).due_date) {
                    child->bound += instance_.job(j).weight
                        * (e - instance_.job(j).due_date);
                }
            }
        }

        child->guide = (double)child->bound;
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == 2 * instance_.job_number());
    }

    inline bool operator()(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        assert(node_1 != nullptr);
        assert(node_2 != nullptr);
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
        return node_1->bound >= node_2->total_weighted_tardiness;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->job_number < instance_.job_number())
            return false;
        if (node_2->job_number < instance_.job_number())
            return true;
        return node_1->total_weighted_tardiness
            < node_2->total_weighted_tardiness;
    }

    bool equals(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        (void)node_1;
        (void)node_2;
        return false;
    }

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->job_number != instance_.job_number())
            return "";
        return std::to_string(node->total_weighted_tardiness);
    }

    /**
     * Dominances.
     */

    inline bool comparable(
            const std::shared_ptr<Node>& node) const
    {
        if (node->new_batch && node->current_batch_start > node->father->current_batch_end)
            return false;
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
        if (node_1->total_weighted_tardiness > node_2->total_weighted_tardiness)
            return false;
        if (node_1->current_batch_end <= node_2->current_batch_start)
            return true;
        if (node_1->current_batch_start != node_2->current_batch_start)
            return false;
        if (node_1->current_batch_end != node_2->current_batch_end)
            return false;
        if (node_1->current_batch_size > node_2->current_batch_size)
            return false;
        return true;
    }

private:

    const Instance& instance_;

    mutable std::vector<JobId> sorted_jobs_;

};

}

}

