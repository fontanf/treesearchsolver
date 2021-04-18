#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"
#include "optimizationtools/indexed_set.hpp"

/**
 * Single machine scheduling problem with sequence-dependent setup times, Total
 * weighted Tardiness.
 *
 * Input:
 * - n jobs with (j = 1..n)
 *   - a due date dⱼ
 *   - a processing time pⱼ
 *   - a weight wⱼ
 * - an n×n matrix s containing the setup times between each pair of jobs
 * Problem:
 * - Find a sequance of jobs such that:
 *   - each job is scheduled exactly once
 * Objective:
 * - Minimize the total weighted tardiness of the schedule
 *
 * Tree search:
 * - forward branching
 *
 */

namespace treesearchsolver
{

namespace schedulingwithsdsttwt
{

typedef int64_t JobId;
typedef int64_t JobPos;
typedef int64_t Time;
typedef double Weight;
typedef int64_t GuideId;

struct Job
{
    Time processing_time = 0;
    Time due_date = 0;
    Weight weight = 1;
};

class Instance
{

public:

    Instance(JobId n):
        jobs_(n),
        setup_times_(n + 1, std::vector<Time>(n, -1))
    {
        for (JobId j = 0; j < n; ++j)
            setup_times_[j][j] = 0;
    }
    void set_processing_time(JobId j, Time processing_time) { jobs_[j].processing_time = processing_time; }
    void set_due_date(JobId j, Time due_date) { jobs_[j].due_date = due_date; }
    void set_weight(JobId j, Weight weight)
    {
        if (jobs_[j].weight == 0)
            zero_weight_job_number_--;
        jobs_[j].weight = weight;
        if (jobs_[j].weight == 0)
            zero_weight_job_number_++;
    }
    void set_setup_time(JobId j1, JobId j2, Time d)
    {
        if (j1 == -1)
            j1 = job_number();
        setup_times_[j1][j2] = d;
    }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "cicirello2005") {
            read_cicirello2005(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline JobId job_number() const { return jobs_.size(); }
    inline JobId zero_weight_job_number() const { return zero_weight_job_number_; }
    inline const Job& job(JobId j) const { return jobs_[j]; }
    inline Time setup_time(JobId j1, JobId j2) const { return setup_times_[j1][j2]; }

    std::pair<bool, Time> check(std::string certificate_path)
    {
        std::ifstream file(certificate_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << certificate_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return {false, 0};
        }

        JobId n = job_number();
        JobId j = -1;
        JobId j_prec = n;
        optimizationtools::IndexedSet jobs(n);
        JobPos duplicates = 0;
        Time current_time = 0;
        Weight total_weighted_tardiness = 0;
        while (file >> j) {
            if (jobs.contains(j)) {
                duplicates++;
                std::cout << "Job " << j << " is already scheduled." << std::endl;
            }
            jobs.add(j);
            current_time += setup_time(j_prec, j);
            if (current_time > job(j).due_date)
                total_weighted_tardiness += (current_time - job(j).due_date);
            std::cout << "Job: " << j
                << "; Time: " << current_time
                << "; Total weighted tardiness: " << total_weighted_tardiness
                << std::endl;
        }
        bool feasible
            = (jobs.size() == n)
            && (duplicates == 0);

        std::cout << "---" << std::endl;
        std::cout << "Job number:                " << jobs.size() << " / " << n  << std::endl;
        std::cout << "Duplicates:                " << duplicates << std::endl;
        std::cout << "Feasible:                  " << feasible << std::endl;
        std::cout << "Total weighted tardiness:  " << total_weighted_tardiness << std::endl;
        return {feasible, total_weighted_tardiness};
    }

private:

    void read_cicirello2005(std::ifstream& file)
    {
        std::string tmp;
        JobId n = -1;
        file >> tmp >> tmp >> tmp;
        file >> tmp >> tmp >> n;

        jobs_ = std::vector<Job>(n);
        setup_times_ = std::vector<std::vector<Time>>(n + 1, std::vector<Time>(n, -1));
        for (JobId j = 0; j < n; ++j)
            setup_times_[j][j] = 0;

        file >> tmp >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp;
        file >> tmp >> tmp >> tmp;
        file >> tmp >> tmp >> tmp;

        file >> tmp >> tmp;
        Time p = -1;
        for (JobId j = 0; j < n; ++j) {
            file >> p;
            set_processing_time(j, p);
        }

        file >> tmp;
        Weight w = -1;
        for (JobId j = 0; j < n; ++j) {
            file >> w;
            set_weight(j, w);
        }

        file >> tmp;
        Time d = -1;
        for (JobId j = 0; j < n; ++j) {
            file >> d;
            set_due_date(j, d);
        }

        file >> tmp >> tmp;
        Time st = -1;
        for (JobId j1 = -1; j1 < n; ++j1) {
            for (JobId j2 = 0; j2 < n; ++j2) {
                if (j1 == j2)
                    continue;
                file >> tmp >> tmp >> st;
                set_setup_time(j1, j2, st);
            }
        }
    }

    std::vector<Job> jobs_;
    std::vector<std::vector<Time>> setup_times_;
    JobPos zero_weight_job_number_ = 0;

};

std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "job number: " << instance.job_number() << std::endl;
    for (JobId j = 0; j < instance.job_number(); ++j)
        os << "job: " << j
            << "; processing time: " << instance.job(j).processing_time
            << "; due date: " << instance.job(j).due_date
            << "; weight: " << instance.job(j).weight
            << std::endl;
    for (JobId j1 = 0; j1 <= instance.job_number(); ++j1) {
        os << "job " << j1 << ":";
        for (JobId j2 = 0; j2 < instance.job_number(); ++j2)
            os << " " << instance.setup_time(j1, j2);
        os << std::endl;
    }
    return os;
}

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
        std::vector<bool> visited;
        JobId j = 0;
        JobId job_number = 0;
        Time current_time = 0;
        double total_weighted_earliness = 0;
        double total_weighted_tardiness = 0;
        Time bound = 0;
        double guide = 0;
        JobPos next_child_pos = 0;
    };

    BranchingScheme(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters)
    { }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->visited.resize(instance_.job_number(), false);
        r->guide = r->bound;
        r->j = instance_.job_number();
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        JobId j_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        if (father->visited[j_next])
            return nullptr;
        Weight wj = instance_.job(j_next).weight;
        if (wj == 0 && father->job_number < instance_.job_number() - instance_.zero_weight_job_number())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->visited = father->visited;
        child->visited[j_next] = true;
        child->j = j_next;
        child->job_number = father->job_number + 1;
        child->current_time = father->current_time
            + instance_.setup_time(father->j, j_next)
            + instance_.job(j_next).processing_time;

        child->total_weighted_tardiness = father->total_weighted_tardiness;
        if (child->current_time > instance_.job(j_next).due_date)
            child->total_weighted_tardiness += (child->current_time - instance_.job(j_next).due_date) * wj;

        child->total_weighted_earliness = father->total_weighted_earliness;
        if (child->current_time < instance_.job(j_next).due_date)
            child->total_weighted_earliness += (double)(instance_.job(j_next).due_date - child->current_time) / wj;

        child->guide = 10 * child->current_time
            + child->total_weighted_earliness
            + child->total_weighted_tardiness;
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
        return node_1->total_weighted_tardiness < node_2->total_weighted_tardiness;
    }

    bool equals(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        (void)node_1;
        (void)node_2;
        return false;
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

    struct NodeHasher
    {
        std::hash<JobId> hasher_1;
        std::hash<std::vector<bool>> hasher_2;

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (node_1->j != node_2->j)
                return false;
            return node_1->visited == node_2->visited;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher_1(node->j);
            optimizationtools::hash_combine(hash, hasher_2(node->visited));
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->current_time <= node_2->current_time
                && node_1->total_weighted_tardiness <= node_2->total_weighted_tardiness)
            return true;
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
        ss << node->total_weighted_tardiness;
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
                << " t " << node_tmp->current_time
                << " twt " << node_tmp->total_weighted_tardiness
                << " twe " << node_tmp->total_weighted_earliness
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->j
                << " dj " << instance_.job(node_tmp->j).due_date
                << " wj " << instance_.job(node_tmp->j).weight
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

