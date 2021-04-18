#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

/**
 * Permutation flow shop scheduling problem, Total tardiness.
 *
 * Input:
 * - m machines
 * - n jobs composed of m operations for each machine with pᵢⱼ the processing
 *   time of the operation of job j on machine i, j = 1..n, i = 1..m
 * Problem:
 * - Find a schedule of jobs such that:
 *   - all operations of all jobs are scheduled
 *   - for all job j = 1..n, i = 1..m - 1, operations (j, i + 1) starts after
 *     the end of operation (j, i)
 *   - the job sequence is the same on all machines
 * Objective:
 * - Minimize the total tardiness of the jobs
 *
 * Tree search:
 * - Forward branching
 */

namespace treesearchsolver
{

namespace permutationflowshopschedulingtt
{

typedef int64_t JobId;
typedef int64_t JobPos;
typedef int64_t MachineId;
typedef int64_t Time;
typedef int64_t GuideId;

struct Job
{
    std::vector<Time> processing_times;
    Time due_date;
};

class Instance
{

public:

    Instance(MachineId m, JobId n):
        jobs_(n) 
    {
        for (JobId j = 0; j < n; ++j)
            jobs_[j].processing_times.resize(m);
    }
    void set_due_date(JobId j, Time due_date) { jobs_[j].due_date = due_date; }
    void set_processing_time(
            JobId j,
            MachineId i,
            Time processing_time)
    {
        jobs_[j].processing_times[i] = processing_time;
    }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "vallada2008") {
            read_vallada2008(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline JobId job_number() const { return jobs_.size(); }
    inline MachineId machine_number() const { return jobs_[0].processing_times.size(); }
    inline const Job& job(JobId j) const { return jobs_[j]; }

    std::pair<bool, Time> check(std::string certificate_path)
    {
        std::ifstream file(certificate_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << certificate_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return {false, 0};
        }

        MachineId m = machine_number();
        JobId n = job_number();
        std::vector<Time> times(m, 0);
        JobId j = 0;
        optimizationtools::IndexedSet jobs(n);
        JobPos duplicates = 0;
        Time total_tardiness = 0;
        while (file >> j) {
            if (jobs.contains(j)) {
                duplicates++;
                std::cout << "Job " << j << " already scheduled." << std::endl;
            }
            jobs.add(j);
            times[0] = times[0] + job(j).processing_times[0];
            for (MachineId i = 1; i < m; ++i) {
                if (times[i - 1] > times[i]) {
                    times[i] = times[i - 1] + job(j).processing_times[i];
                } else {
                    times[i] = times[i] + job(j).processing_times[i];
                }
            }
            if (times[m - 1] > job(j).due_date)
                total_tardiness += (times[m - 1] - job(j).due_date);
            std::cout << "Job: " << j
                << "; Time: " << times[m - 1]
                << "; Total tardiness: " << total_tardiness
                << std::endl;
        }
        bool feasible
            = (jobs.size() == n)
            && (duplicates == 0);

        std::cout << "---" << std::endl;
        std::cout << "Job number:             " << jobs.size() << " / " << n  << std::endl;
        std::cout << "Duplicates:             " << duplicates << std::endl;
        std::cout << "Feasible:               " << feasible << std::endl;
        std::cout << "Total tardiness:        " << total_tardiness << std::endl;
        return {feasible, total_tardiness};
    }

private:

    void read_vallada2008(std::ifstream& file)
    {
        JobId n;
        MachineId m;
        file >> n >> m;
        jobs_ = std::vector<Job>(n);
        for (JobId j = 0; j < n; ++j)
            jobs_[j].processing_times.resize(m);

        for (MachineId i = 0; i < m; i++) {
            Time p = -1;
            MachineId i_tmp = -1;
            for (JobId j = 0; j < n; j++) {
                file >> i_tmp >> p;
                set_processing_time(j, i, p);
            }
        }
        std::string tmp;
        file >> tmp;
        for (JobId j = 0; j < n; ++j) {
            Time d = -1;
            file >> tmp >> d >> tmp >> tmp;
            set_due_date(j, d);
        }
    }

    std::vector<Job> jobs_;

};

std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "machine number " << instance.machine_number() << std::endl;
    os << "job number " << instance.job_number() << std::endl;
    for (JobId j = 0; j < instance.job_number(); ++j) {
        os << "job " << j << "; due date " << instance.job(j).due_date << "; processing times:";
        for (MachineId i = 0; i < instance.machine_number(); ++i)
            os << " " << instance.job(j).processing_times[i];
        os << std::endl;
    }
    return os;
}

class BranchingScheme
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

    BranchingScheme(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.machine_number();
        JobId n = instance_.job_number();
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
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
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
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

}

}

