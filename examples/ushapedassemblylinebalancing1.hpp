#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

/**
 * U-shaped Assembly Line Balancing Problem of Type 1.
 *
 * Input:
 * - n jobs with processing time pⱼ (j = 1..n)
 * - a cycle time c (station capacity)
 * - a directed acyclic graph G such that each node corresponds to a job
 * Problem:
 * - Find an assignment of jobs to stations such that:
 *   - each job is assigned to exactly one station
 *   - the sum of the processing times of the jobs assigned to each station
 *     does not exceed the cycle time
 *   - if there exists an arc from j₁ to j₂ in G, then j₁ must not be assigned
 *     to a station located after the station assigned to j₂
 * Objective:
 * - Minimize the number of stations
 *
 */

namespace treesearchsolver
{

namespace ushapedassemblylinebalancing1
{

typedef int64_t JobId;
typedef int64_t JobPos;
typedef int64_t StationId;
typedef int64_t Time;
typedef int64_t GuideId;

struct Job
{
    JobId id;
    Time processing_time;
    std::vector<JobId> predecessors;
    std::vector<JobId> successors;
};

class Instance
{

public:

    Instance() { }
    void add_job(Time p)
    {
        Job job;
        job.id = jobs_.size();
        job.processing_time = p;
        jobs_.push_back(job);
        processing_time_sum_ += p;
    }
    void add_predecessor(JobId j1, JobId j2)
    {
        assert(j1 >= 0);
        assert(j2 >= 0);
        assert(j1 < job_number());
        assert(j2 < job_number());
        jobs_[j1].predecessors.push_back(j2);
        jobs_[j2].successors.push_back(j1);
    }
    void set_cycle_time(Time cycle_time) { cycle_time_ = cycle_time; }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "scholl1993") {
            read_scholl1993(file);
        } else if (format == "otto2013") {
            read_otto2013(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline JobId job_number() const { return jobs_.size(); }
    inline const Job& job(JobId j) const { return jobs_[j]; }
    inline Time cycle_time() const { return cycle_time_; }
    inline Time processing_time_sum() const { return processing_time_sum_; }

    std::pair<bool, Time> check(std::string certificate_path)
    {
        std::ifstream file(certificate_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << certificate_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return {false, 0};
        }

        JobId n = job_number();
        JobPos s = -1;
        optimizationtools::IndexedSet jobs(n);
        JobPos duplicates = 0;
        JobPos precedence_violation_number = 0;
        StationId overloaded_station_number = 0;
        StationId station_number = 0;
        while (file >> s) {
            JobId j = -1;
            Time t = 0;
            station_number++;
            std::cout << "Station: " << station_number - 1 << "; Jobs";
            for (JobPos j_pos = 0; j_pos < s; ++j_pos) {
                file >> j;
                // Check duplicates.
                if (jobs.contains(j)) {
                    duplicates++;
                    std::cout << std::endl << "Job " << j << " already scheduled." << std::endl;
                }
                // Check predecessors.
                for (JobId j_pred: job(j).predecessors) {
                    if (!jobs.contains(j_pred)) {
                        for (JobId j_succ: job(j).successors) {
                            if (!jobs.contains(j_succ)) {
                                precedence_violation_number++;
                                std::cout << std::endl << "Job " << j << " violates precedence constraints." << std::endl;
                                break;
                            }
                        }
                        break;
                    }
                }
                std::cout << " " << j;
                jobs.add(j);
                t += job(j).processing_time;
            }
            std::cout << "; Cycle time: " << t << " / " << cycle_time() << std::endl;
            if (t > cycle_time()) {
                overloaded_station_number++;
                std::cout << "Station " << station_number - 1 << " is overloaded." << std::endl;
            }
        }
        bool feasible
            = (jobs.size() == n)
            && (duplicates == 0)
            && (precedence_violation_number == 0)
            && (overloaded_station_number == 0);

        std::cout << "---" << std::endl;
        std::cout << "Job number:                   " << jobs.size() << " / " << n  << std::endl;
        std::cout << "Duplicates:                   " << duplicates << std::endl;
        std::cout << "Precedence violation number:  " << precedence_violation_number << std::endl;
        std::cout << "Overloaded station number:    " << overloaded_station_number << std::endl;
        std::cout << "Feasible:                     " << feasible << std::endl;
        std::cout << "Station number:               " << station_number << std::endl;
        return {feasible, station_number};
    }

private:

    void read_scholl1993(std::ifstream& file)
    {
        JobId n = -1;
        file >> n;
        Time p = -1;
        for (JobId j = 0; j < n; ++j) {
            file >> p;
            add_job(p);
        }

        Time cycle_time = -1;
        file >> cycle_time;
        set_cycle_time(cycle_time);

        std::string tmp;
        std::vector<std::string> line;
        getline(file, tmp);
        for (;;) {
            getline(file, tmp);
            line = optimizationtools::split(tmp, ',');
            if (std::stol(line[0]) == -1)
                break;
            add_predecessor(std::stol(line[1]) - 1, std::stol(line[0]) - 1);
        }
    }

    void read_otto2013(std::ifstream& file)
    {
        std::string tmp;
        std::vector<std::string> line;
        JobId n = -1;
        JobId j_tmp = -1;
        double d_tmp = -1;
        while (getline(file, tmp)) {
            line = optimizationtools::split(tmp, ' ');
            if (line.size() == 0) {
            } else if (tmp.rfind("<number of tasks>", 0) == 0) {
                file >> n;
            } else if (tmp.rfind("<cycle time>", 0) == 0) {
                Time cycle_time = -1;
                file >> cycle_time;
                set_cycle_time(cycle_time);
            } else if (tmp.rfind("<order strength>", 0) == 0) {
                file >> d_tmp;
            } else if (tmp.rfind("<task times>", 0) == 0) {
                Time p = -1;
                for (JobId j = 0; j < n; ++j) {
                    file >> j_tmp >> p;
                    add_job(p);
                }
            } else if (tmp.rfind("<precedence relations>", 0) == 0) {
                for (;;) {
                    getline(file, tmp);
                    if (tmp.size() <= 1)
                        break;
                    line = optimizationtools::split(tmp, ',');
                    add_predecessor(std::stol(line[1]) - 1, std::stol(line[0]) - 1);
                }
            } else if (tmp.rfind("<end>", 0) == 0) {
                break;
            }
        }
    }

    std::vector<Job> jobs_;
    Time cycle_time_ = 0;
    Time processing_time_sum_ = 0;

};

std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "cycle time " << instance.cycle_time() << std::endl;
    os << "job number " << instance.job_number() << std::endl;
    for (JobId j = 0; j < instance.job_number(); ++j) {
        os << "job " << j
            << " p " << instance.job(j).processing_time
            << " pred";
        for (JobId j2: instance.job(j).predecessors)
            os << " " << j2;
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
        std::vector<bool> jobs;
        JobId j = -1;
        JobId job_number = 0;
        StationId station_number = 0;
        Time current_station_time = 0;
        Time processing_time_sum = 0;
        StationId bound = -1;
        double guide = 0;
        JobPos next_child_pos = 0;
        bool added_in_current_station = false;
    };

    BranchingScheme(const Instance& instance, const Parameters& parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_jobs_(instance.job_number())
    {
        // Initialize sorted_jobs_.
        std::iota(sorted_jobs_.begin(), sorted_jobs_.end(), 0);
        sort(sorted_jobs_.begin(), sorted_jobs_.end(),
                [&instance](JobId j1, JobId j2) -> bool
                {
                    return instance.job(j1).processing_time < instance.job(j2).processing_time;
                });
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->jobs.resize(instance_.job_number(), false);
        r->current_station_time = instance_.cycle_time();
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        JobId j_next = sorted_jobs_[father->next_child_pos];
        // Update father
        father->next_child_pos++;

        if (father->jobs[j_next])
            return nullptr;
        for (JobId j_pred: instance_.job(j_next).predecessors) {
            if (!father->jobs[j_pred]) {
                for (JobId j_succ: instance_.job(j_next).successors)
                    if (!father->jobs[j_succ])
                        return nullptr;
                break;
            }
        }
        Time p = instance_.job(j_next).processing_time;
        if (father->added_in_current_station
               && father->current_station_time + p > instance_.cycle_time()) {
            father->next_child_pos = instance_.job_number();
            return nullptr;
        }
        if (father->current_station_time + p <= instance_.cycle_time())
            father->added_in_current_station = true;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->j = j_next;
        child->job_number = father->job_number + 1;
        child->jobs = father->jobs;
        child->jobs[j_next] = true;
        child->processing_time_sum = father->processing_time_sum + p;
        if (father->current_station_time + p <= instance_.cycle_time()) {
            child->current_station_time = father->current_station_time + p;
            child->station_number = father->station_number;
        } else {
            child->current_station_time = p;
            child->station_number = father->station_number + 1;
        }
        Time total_time = (child->station_number - 1) * instance_.cycle_time()
                + child->current_station_time;
        Time idle_time = total_time - child->processing_time_sum;
        child->bound = std::ceil(
                (double)(idle_time + instance_.processing_time_sum())
                / instance_.cycle_time());
        double mean_job_processing_time = (double)child->processing_time_sum
            / child->job_number;
        child->guide = (double)idle_time / total_time
            / std::pow(mean_job_processing_time, 2);
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
        assert(node_1 != nullptr);
        assert(node_2 != nullptr);
        assert(!infertile(node_1));
        assert(!infertile(node_2));
        //if (node_1->job_number != node_2->job_number)
        //    return node_1->job_number < node_2->job_number;
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
        return node_1->bound >= node_2->station_number;
    }

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->job_number < instance_.job_number())
            return false;
        if (node_2->job_number < instance_.job_number())
            return true;
        return node_1->station_number < node_2->station_number;
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
        if (node_1->station_number < node_2->station_number)
            return true;
        if (node_1->station_number == node_2->station_number
                && node_1->current_station_time <= node_2->current_station_time)
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
        return std::to_string(node->station_number);
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        StationId m = node->station_number;
        std::vector<std::vector<JobId>> stations(m);
        std::vector<Time> times(m, 0);
        for (auto node_tmp = node; node_tmp->father != nullptr; node_tmp = node_tmp->father) {
            stations[node_tmp->station_number - 1].push_back(node_tmp->j);
            times[node_tmp->station_number - 1] += instance_.job(node_tmp->j).processing_time;
        }
        for (ushapedassemblylinebalancing1::StationId i = 0; i < m; ++i) {
            os << "Station " << i << " " << times[i] << "/" << instance_.cycle_time() << ":";
            std::reverse(stations[i].begin(), stations[i].end());
            for (ushapedassemblylinebalancing1::JobId j: stations[i])
                os << " " << j;
            os << std::endl;
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

        StationId m = node->station_number;
        std::vector<std::vector<JobId>> stations(m);
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            stations[node_tmp->station_number - 1].push_back(node_tmp->j);
        for (StationId i = 0; i < m; ++i) {
            std::reverse(stations[i].begin(), stations[i].end());
            cert << stations[i].size();
            for (JobId j: stations[i])
                cert << " " << j;
            cert << std::endl;
        }
    }

private:

    const Instance& instance_;
    const Parameters& parameters_;

    mutable std::vector<JobId> sorted_jobs_;

};

}

}

