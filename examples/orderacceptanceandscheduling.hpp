#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/sorted_on_demand_array.hpp"
#include "optimizationtools/indexed_set.hpp"

/**
 * Single machine order acceptance and scheduling problem with
 * sequence-dependent setup times.
 *
 * Input:
 * - n jobs with
 *   - a release date rⱼ
 *   - a due date dⱼ
 *   - a deadline đⱼ
 *   - a processing time pⱼ
 *   - a weight wⱼ
 *   - a profit vⱼ
 * - an n×n symmetric matrix s containing the setup times between each
 *   pair of jobs
 * Problem:
 * - find a sequence of jobs starting with job 1 and ending with job n such
 *   that:
 *   - each scheduled job starts after its release date
 *     !!! The start date of a job is before its setup time !!!
 *   - each scheduled job terminates before its deadline
 * Objective:
 * - maximize the profit of the scheduled jobs minus their weighted tardiness
 *
 * Tree search
 * - forward branching
 * - guide: time / profit
 */

namespace treesearchsolver
{

namespace orderacceptanceandscheduling
{

typedef int64_t JobId;
typedef int64_t JobPos;
typedef int64_t Time;
typedef double Weight;
typedef double Profit;
typedef int64_t GuideId;

struct Job
{
    JobId id;
    Time release_date;
    Time due_date;
    Time deadline;
    Time processing_time;
    Weight weight;
    Profit profit;
};

class Instance
{

public:

    Instance(JobId n):
        jobs_(n),
        setup_times_(n, std::vector<Time>(n))
    {
        for (JobId j = 0; j < n; ++j)
            jobs_[j].id = j;
    }
    void set_job(
            JobId j,
            Time release_date,
            Time due_date,
            Time deadline,
            Time processing_time,
            Weight weight,
            Profit profit)
    {
        jobs_[j].release_date = release_date;
        jobs_[j].due_date = due_date;
        jobs_[j].deadline = deadline;
        jobs_[j].processing_time = processing_time;
        jobs_[j].weight = weight;
        jobs_[j].profit = profit;
    }
    void set_setup_time(JobId j1, JobId j2, Time setup_time)
    {
        setup_times_[j1][j2] = setup_time;
    }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "cesaret2012") {
            read_cesaret2012(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline JobId job_number() const { return jobs_.size(); }
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
        JobId j_prec = 0;
        optimizationtools::IndexedSet jobs(n);
        JobPos duplicates = 0;
        JobPos deadline_violation_number = 0;
        Time current_time = 0;
        Profit profit = 0;
        Weight total_weighted_tardiness = 0;
        while (file >> j) {
            if (jobs.contains(j)) {
                duplicates++;
                std::cout << "Job " << j << " is already scheduled." << std::endl;
            }
            jobs.add(j);
            current_time = std::max(current_time, job(j).release_date)
                + setup_time(j_prec, j) + job(j).processing_time;
            profit += job(j).profit;
            if (current_time > job(j).due_date)
                total_weighted_tardiness += (current_time - job(j).due_date);
            if (current_time > job(j).deadline) {
                deadline_violation_number++;
                std::cout << "Job " << j << " ends after its deadline: "
                    << current_time << " / " << job(j).deadline << "." << std::endl;
            }
            std::cout << "Job: " << j
                << "; Time: " << current_time
                << "; Profit: " << profit
                << "; Total weighted tardiness: " << total_weighted_tardiness
                << std::endl;
        }
        bool feasible
            = (duplicates == 0)
            && (deadline_violation_number == 0);

        std::cout << "---" << std::endl;
        std::cout << "Job number:                 " << jobs.size() << " / " << n  << std::endl;
        std::cout << "Duplicates:                 " << duplicates << std::endl;
        std::cout << "Deadline violation number:  " << duplicates << std::endl;
        std::cout << "Feasible:                   " << feasible << std::endl;
        std::cout << "Profit:                     " << profit << std::endl;
        std::cout << "Total weighted tardiness:   " << total_weighted_tardiness << std::endl;
        std::cout << "Objective:                  " << profit - total_weighted_tardiness << std::endl;
        return {feasible, profit - total_weighted_tardiness};
    }

private:

    void read_cesaret2012(std::ifstream& file)
    {
        std::string tmp;
        std::vector<std::string> line;

        getline(file, tmp);
        line = optimizationtools::split(tmp, ',');
        JobId n = line.size();
        jobs_.resize(n);
        setup_times_.resize(n, std::vector<Time>(n));
        for (JobId j = 0; j < n; ++j)
            jobs_[j].release_date = std::stol(line[j]);
        getline(file, tmp);
        line = optimizationtools::split(tmp, ',');
        for (JobId j = 0; j < n; ++j)
            jobs_[j].processing_time = std::stol(line[j]);
        getline(file, tmp);
        line = optimizationtools::split(tmp, ',');
        for (JobId j = 0; j < n; ++j)
            jobs_[j].due_date = std::stol(line[j]);
        getline(file, tmp);
        line = optimizationtools::split(tmp, ',');
        for (JobId j = 0; j < n; ++j)
            jobs_[j].deadline = std::stol(line[j]);
        getline(file, tmp);
        line = optimizationtools::split(tmp, ',');
        for (JobId j = 0; j < n; ++j)
            jobs_[j].profit = std::stod(line[j]);
        getline(file, tmp);
        line = optimizationtools::split(tmp, ',');
        for (JobId j = 0; j < n; ++j)
            jobs_[j].weight = std::stod(line[j]);
        for (JobId j1 = 0; j1 < n; ++j1) {
            getline(file, tmp);
            line = optimizationtools::split(tmp, ',');
            for (JobId j2 = 0; j2 < n; ++j2)
                setup_times_[j1][j2] = std::stol(line[j2]);
        }
    }

    std::vector<Job> jobs_;
    std::vector<std::vector<Time>> setup_times_;

};

std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "job number: " << instance.job_number() << std::endl;
    for (JobId j = 0; j < instance.job_number(); ++j)
        os << "job: " << j
            << "; processing time: " << instance.job(j).processing_time
            << "; release date: " << instance.job(j).release_date
            << "; due date: " << instance.job(j).due_date
            << "; deadline: " << instance.job(j).deadline
            << "; weight: " << instance.job(j).weight
            << "; profit: " << instance.job(j).profit
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

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        JobId j = 0;
        JobId job_number = 0;
        Time time = 0;
        Profit profit = 0;
        Weight weighted_tardiness = 0;
        double guide = 0;
        JobPos next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 0;
    };

    BranchingScheme(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_jobs_(instance.job_number() + 1),
        generator_(0)
    {
        // Initialize sorted_jobs_.
        for (JobId j = 0; j < instance_.job_number(); ++j) {
            sorted_jobs_[j].reset(instance.job_number());
            for (JobId j2 = 0; j2 < instance_.job_number(); ++j2) {
                double c = (double)instance_.setup_time(j, j2) / instance_.job(j2).profit;
                sorted_jobs_[j].set_cost(j2, c);
            }
        }
    }

    inline JobId neighbor(JobId j, JobPos pos) const
    {
        assert(j < instance_.job_number());
        assert(pos < instance_.job_number());
        return sorted_jobs_[j].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_jobs.resize(instance_.job_number(), true);
        r->available_jobs[0] = false;
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        JobId j_next = neighbor(father->j, father->next_child_pos);

        //std::cout << "father "
        //    << " j " << father->j
        //    << " t " << father->time
        //    << " p " << father->profit
        //    << " w " << father->weight
        //    << " j_next " << j_next
        //    << " s " << instance_.setup_time(father->j, j_next)
        //    << " r " << instance_.job(j_next).release_date
        //    << " d " << instance_.job(j_next).deadline
        //    << std::endl;

        // Update father
        father->next_child_pos++;
        // Check job availibility.
        if (!father->available_jobs[j_next])
            return nullptr;
        // Check deadline.
        Time start = std::max(father->time, instance_.job(j_next).release_date);
        Time p = instance_.setup_time(father->j, j_next) + instance_.job(j_next).processing_time;
        if (start + p > instance_.job(j_next).deadline)
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->available_jobs = father->available_jobs;
        child->available_jobs[j_next] = false;
        child->j = j_next;
        child->job_number = father->job_number + 1;
        child->time = start + p;
        child->profit = father->profit + instance_.job(j_next).profit;
        child->weighted_tardiness = father->weighted_tardiness;
        Time d = instance_.job(j_next).due_date;
        if (child->time > d)
            child->weighted_tardiness += instance_.job(j_next).weight * (child->time - d);
        child->guide = (double)child->time / (child->profit - child->weighted_tardiness);
        for (JobId j = 0; j < instance_.job_number(); ++j) {
            if (!child->available_jobs[j])
                continue;
            if (child->time + instance_.setup_time(j_next, j)
                    + instance_.job(j).processing_time > instance_.job(j).deadline)
                child->available_jobs[j] = false;
        }
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == instance_.job_number() - 1);
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
        (void)node_1;
        (void)node_2;
        return false;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        return node_1->profit - node_1->weighted_tardiness
            > node_2->profit - node_2->weighted_tardiness;
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

    const Instance& instance() const { return instance_; }

    struct NodeHasher
    {
        const BranchingScheme& branching_scheme_;
        std::hash<JobId> hasher_1;
        std::hash<std::vector<bool>> hasher_2;

        NodeHasher(const BranchingScheme& branching_scheme):
            branching_scheme_(branching_scheme) { }

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (node_1->j != node_2->j)
                return false;
            if (node_1->available_jobs != node_2->available_jobs)
                return false;
            return true;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher_1(node->j);
            optimizationtools::hash_combine(hash, hasher_2(node->available_jobs));
            return hash;
        }
    };

    inline NodeHasher node_hasher() const { return NodeHasher(*this); }

    inline bool dominates(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->time <= node_2->time
                && node_1->profit - node_1->weighted_tardiness >= node_2->profit - node_2->weighted_tardiness)
            return true;
        return false;
    }

    /*
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        std::stringstream ss;
        ss << node->profit - node->weighted_tardiness
            << " (n" << node->job_number
            << " p" << node->profit
            << " w" << node->weighted_tardiness
            << ")";
        return ss.str();
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            os << "node_tmp"
                << " n " << node_tmp->job_number
                << " t " << node_tmp->time
                << " p " << node_tmp->profit
                << " w " << node_tmp->weighted_tardiness
                << " j " << node_tmp->j
                << " rj " << instance_.job(node_tmp->j).release_date
                << " dj " << instance_.job(node_tmp->j).due_date
                << " dj " << instance_.job(node_tmp->j).deadline
                << " pj " << instance_.job(node_tmp->j).processing_time
                << " vj " << instance_.job(node_tmp->j).profit
                << " wj " << instance_.job(node_tmp->j).weight
                << " sij " << instance_.setup_time(node_tmp->father->j, node_tmp->j)
                << std::endl;
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

    mutable std::vector<optimizationtools::SortedOnDemandArray> sorted_jobs_;
    mutable std::mt19937_64 generator_;

};

}

}

