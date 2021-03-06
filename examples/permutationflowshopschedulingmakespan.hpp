#pragma once

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

/**
 * Permutation flow shop scheduling problem, Makespan.
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
 * - Minimize the makespan of the schedule
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

namespace treesearchsolver
{

namespace permutationflowshopschedulingmakespan
{

typedef int64_t JobId;
typedef int64_t JobPos;
typedef int64_t MachineId;
typedef int64_t Time;
typedef int64_t GuideId;

class Instance
{

public:

    Instance(MachineId m, JobId n):
        processing_times_(n, std::vector<Time>(m, 0)) { }
    void set_processing_time(
            JobId j,
            MachineId i,
            Time processing_time)
    {
        processing_times_[j][i] = processing_time;
    }

    Instance(std::string instance_path, std::string format = "")
    {
        std::ifstream file(instance_path);
        if (!file.good()) {
            std::cerr << "\033[31m" << "ERROR, unable to open file \"" << instance_path << "\"" << "\033[0m" << std::endl;
            assert(false);
            return;
        }
        if (format == "" || format == "default") {
            read_default(file);
        } else {
            std::cerr << "\033[31m" << "ERROR, unknown instance format \"" << format << "\"" << "\033[0m" << std::endl;
        }
        file.close();
    }

    virtual ~Instance() { }

    inline JobId job_number() const { return processing_times_.size(); }
    inline MachineId machine_number() const { return processing_times_[0].size(); }
    inline Time processing_time(JobId j, MachineId i) const { return processing_times_[j][i]; }

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
        while (file >> j) {
            if (jobs.contains(j)) {
                duplicates++;
                std::cout << "Job " << j << " already scheduled." << std::endl;
            }
            jobs.add(j);
            times[0] = times[0] + processing_time(j, 0);
            for (MachineId i = 1; i < m; ++i) {
                if (times[i - 1] > times[i]) {
                    times[i] = times[i - 1] + processing_time(j, i);
                } else {
                    times[i] = times[i] + processing_time(j, i);
                }
            }
            std::cout << "Job: " << j
                << "; Time: " << times[m - 1]
                << std::endl;
        }
        bool feasible
            = (jobs.size() == n)
            && (duplicates == 0);

        std::cout << "---" << std::endl;
        std::cout << "Job number:  " << jobs.size() << " / " << n << std::endl;
        std::cout << "Duplicates:  " << duplicates << std::endl;
        std::cout << "Feasible:    " << feasible << std::endl;
        std::cout << "Makespan:    " << times[m - 1] << std::endl;
        return {feasible, times[m - 1]};
    }

private:

    void read_default(std::ifstream& file)
    {
        JobId n;
        MachineId m;
        file >> n;
        file >> m;
        processing_times_ = std::vector<std::vector<Time>>(n, std::vector<Time>(m, 0));

        for (MachineId i = 0; i < m; i++) {
            Time p;
            for (JobId j = 0; j < n; j++) {
                file >> p;
                set_processing_time(j, i, p);
            }
        }
    }

    std::vector<std::vector<Time>> processing_times_;
    Time machine_total_processing_time_max = 0;
    Time job_total_processing_time_max = 0;

};

std::ostream& operator<<(
        std::ostream &os, const Instance& instance)
{
    os << "machine number " << instance.machine_number() << std::endl;
    os << "job number " << instance.job_number() << std::endl;
    for (JobId j = 0; j < instance.job_number(); ++j) {
        os << "job " << j << ":";
        for (MachineId i = 0; i < instance.machine_number(); ++i)
            os << " " << instance.processing_time(j, i);
        os << std::endl;
    }
    return os;
}

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
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        bool forward = true;
        JobId j = -1;
        JobId job_number = 0;
        std::vector<NodeMachine> machines;
        Time idle_time = 0;
        double weighted_idle_time = 0;
        Time bound = 0;
        double guide = 0;
        JobId next_child_pos = 0;
    };

    struct Parameters
    {
        bool bidirectional = true;
        GuideId guide_id = 3;
    };

    BranchingSchemeBidirectional(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.machine_number();
        JobId n = instance_.job_number();
        auto r = std::shared_ptr<Node>(new BranchingSchemeBidirectional::Node());
        r->available_jobs.resize(n, true);
        r->machines.resize(m);
        for (JobId j = 0; j < n; ++j)
            for (MachineId i = 0; i < m; ++i)
                r->machines[i].remaining_processing_time += instance_.processing_time(j, i);
        r->bound = 0;
        for (JobId j = 0; j < n; ++j)
            r->bound += instance_.processing_time(j, m - 1);
        if (best_node_ == nullptr)
            best_node_ = r;
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        MachineId m = instance_.machine_number();
        auto father = node->father;
        node->available_jobs = father->available_jobs;
        node->available_jobs[node->j] = false;
        node->machines = father->machines;
        if (father->forward) {
            node->machines[0].time_forward += instance_.processing_time(node->j, 0);
            node->machines[0].remaining_processing_time -= instance_.processing_time(node->j, 0);
            for (MachineId i = 1; i < m; ++i) {
                if (node->machines[i - 1].time_forward > father->machines[i].time_forward) {
                    Time idle_time = node->machines[i - 1].time_forward - father->machines[i].time_forward;
                    node->machines[i].time_forward = node->machines[i - 1].time_forward
                        + instance_.processing_time(node->j, i);
                    node->machines[i].idle_time_forward += idle_time;
                } else {
                    node->machines[i].time_forward += + instance_.processing_time(node->j, i);
                }
                node->machines[i].remaining_processing_time -= instance_.processing_time(node->j, i);
            }
        } else {
            node->machines[m - 1].time_backward += instance_.processing_time(node->j, m - 1);
            node->machines[m - 1].remaining_processing_time -= instance_.processing_time(node->j, m - 1);
            for (MachineId i = m - 2; i >= 0; --i) {
                if (node->machines[i + 1].time_backward > father->machines[i].time_backward) {
                    Time idle_time = node->machines[i + 1].time_backward - father->machines[i].time_backward;
                    node->machines[i].time_backward = node->machines[i + 1].time_backward
                        + instance_.processing_time(node->j, i);
                    node->machines[i].idle_time_backward += idle_time;
                } else {
                    node->machines[i].time_backward += instance_.processing_time(node->j, i);
                }
                node->machines[i].remaining_processing_time -= instance_.processing_time(node->j, i);
            }
        }
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        // Compute father's structures.
        if (father->next_child_pos == 0 && father->father != nullptr)
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

        // Determine wether to use forward or backward.
        if (father->next_child_pos != 0) {
        } else if (!parameters_.bidirectional) {
            father->forward = true;
        } else if (father->father == nullptr) {
            father->forward = true;
        } else if (father->father->father == nullptr) {
            father->forward = false;
        } else {
            MachineId m = instance_.machine_number();
            JobId n = instance_.job_number();
            JobPos n_forward = 0;
            JobPos n_backward = 0;
            Time bound_forward = 0;
            Time bound_backward = 0;
            for (JobId j_next = 0; j_next < n; ++j_next) {
                if (!father->available_jobs[j_next])
                    continue;
                // Forward.
                Time bf = 0;
                Time t_prec = father->machines[0].time_forward
                    + instance_.processing_time(j_next, 0);
                Time t = 0;
                bf = std::max(bf,
                        t_prec
                        + father->machines[0].remaining_processing_time
                        - instance_.processing_time(j_next, 0)
                        + father->machines[0].time_backward);
                for (MachineId i = 1; i < m; ++i) {
                    if (t_prec > father->machines[i].time_forward) {
                        t = t_prec
                            + instance_.processing_time(j_next, i);
                    } else {
                        t = father->machines[i].time_forward
                            + instance_.processing_time(j_next, i);
                    }
                    bf = std::max(bf,
                            t
                            + father->machines[i].remaining_processing_time
                            - instance_.processing_time(j_next, i)
                            + father->machines[i].time_backward);
                    t_prec = t;
                }
                if (best_node_->job_number != n
                        || bf < best_node_->bound) {
                    n_forward++;
                    bound_forward += bf;
                }
                // Backward.
                Time bb = 0;
                t_prec
                    = father->machines[m - 1].time_backward
                    + instance_.processing_time(j_next, m - 1);
                bb = std::max(bb,
                        father->machines[m - 1].time_forward
                        + father->machines[m - 1].remaining_processing_time
                        - instance_.processing_time(j_next, m - 1)
                        + t_prec);
                for (MachineId i = m - 2; i >= 0; --i) {
                    if (t_prec > father->machines[i].time_backward) {
                        t = t_prec
                            + instance_.processing_time(j_next, i);
                    } else {
                        t = father->machines[i].time_backward
                            + instance_.processing_time(j_next, i);
                    }
                    bb = std::max(bb,
                            father->machines[i].time_forward
                            + father->machines[i].remaining_processing_time
                            - instance_.processing_time(j_next, i)
                            + t);
                    t_prec = t;
                }
                if (best_node_->job_number != n
                        || bb < best_node_->bound) {
                    n_backward++;
                    bound_backward += bb;
                }
            }
            if (n_forward < n_backward) {
                father->forward = true;
            } else if (n_forward > n_backward) {
                father->forward = false;
            } else if (bound_forward > bound_backward) {
                father->forward = true;
            } else if (bound_forward < bound_backward) {
                father->forward = false;
            } else {
                father->forward = !father->father->forward;
            }
        }

        JobId j_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        // Check job availibility.
        if (!father->available_jobs[j_next])
            return nullptr;

        // Compute new child.
        MachineId m = instance_.machine_number();
        JobId n = instance_.job_number();
        auto child = std::shared_ptr<Node>(new BranchingSchemeBidirectional::Node());
        child->father = father;
        child->j = j_next;
        child->job_number = father->job_number + 1;
        // Update machines and idle_time.
        child->idle_time = father->idle_time;
        Time t = 0;
        Time t_prec = 0;
        if (father->forward) {
            t_prec = father->machines[0].time_forward
                + instance_.processing_time(j_next, 0);
            Time remaining_processing_time
                = father->machines[0].remaining_processing_time
                - instance_.processing_time(j_next, 0);
            child->weighted_idle_time += (father->machines[0].time_backward == 0)? 1:
                (double)father->machines[0].idle_time_backward / father->machines[0].time_backward;
            child->bound = std::max(child->bound,
                    t_prec
                    + remaining_processing_time
                    + father->machines[0].time_backward);
            for (MachineId i = 1; i < m; ++i) {
                Time machine_idle_time = father->machines[i].idle_time_forward;
                if (t_prec > father->machines[i].time_forward) {
                    Time idle_time = t_prec - father->machines[i].time_forward;
                    t = t_prec
                        + instance_.processing_time(j_next, i);
                    machine_idle_time += idle_time;
                    child->idle_time += idle_time;
                } else {
                    t = father->machines[i].time_forward
                        + instance_.processing_time(j_next, i);
                }
                Time remaining_processing_time
                    = father->machines[i].remaining_processing_time
                    - instance_.processing_time(j_next, i);
                child->weighted_idle_time += (t == 0)? 1:
                    (double)machine_idle_time / t;
                child->weighted_idle_time += (father->machines[i].time_backward == 0)? 1:
                    (double)father->machines[i].idle_time_backward / father->machines[i].time_backward;
                child->bound = std::max(child->bound,
                        t
                        + remaining_processing_time
                        + father->machines[i].time_backward);
                t_prec = t;
            }
        } else {
            t_prec = father->machines[m - 1].time_backward
                + instance_.processing_time(j_next, m - 1);
            Time remaining_processing_time
                = father->machines[m - 1].remaining_processing_time
                - instance_.processing_time(j_next, m - 1);
            child->weighted_idle_time += (father->machines[m - 1].time_forward == 0)? 1:
                (double)father->machines[m - 1].idle_time_forward / father->machines[m - 1].time_forward;
            child->bound = std::max(child->bound,
                    father->machines[m - 1].time_forward
                    + remaining_processing_time
                    + t_prec);
            for (MachineId i = m - 2; i >= 0; --i) {
                Time machine_idle_time = father->machines[i].idle_time_backward;
                if (t_prec > father->machines[i].time_backward) {
                    Time idle_time = t_prec - father->machines[i].time_backward;
                    t = t_prec
                        + instance_.processing_time(j_next, i);
                    machine_idle_time += idle_time;
                    child->idle_time += idle_time;
                } else {
                    t = father->machines[i].time_backward
                        + instance_.processing_time(j_next, i);
                }
                Time remaining_processing_time
                    = father->machines[i].remaining_processing_time
                    - instance_.processing_time(j_next, i);
                child->weighted_idle_time += (father->machines[i].time_forward == 0)? 1:
                    (double)father->machines[i].idle_time_forward / father->machines[i].time_forward;
                child->weighted_idle_time += (t == 0)? 1:
                    (double)machine_idle_time / t;
                child->bound = std::max(child->bound,
                        father->machines[i].time_forward
                        + remaining_processing_time
                        + t);
                t_prec = t;
            }
        }
        // Compute guide.
        double alpha = (double)child->job_number / n;
        switch (parameters_.guide_id) {
        case 0: {
            child->guide = child->bound;
            break;
        } case 1: {
            child->guide = child->idle_time;
            break;
        } case 2: {
            child->guide = alpha * child->bound
                + (1.0 - alpha) * child->idle_time * child->job_number / m;
            break;
        } case 3: {
            child->guide = alpha * child->bound
                + (1.0 - alpha) * child->weighted_idle_time * child->bound;
            break;
        } case 4: {
            double a1 = (best_node_->job_number == instance_.job_number())?
                (double)(best_node_->bound) / (best_node_->bound - child->bound):
                1 - alpha;
            double a2 = (best_node_->job_number == instance_.job_number())?
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
        if (node_1->bound >= node_2->bound)
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
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->job_number != instance_.job_number())
            return "";
        std::stringstream ss;
        ss << node->bound;
        return ss.str();
    }

    std::ostream& print(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        MachineId m = instance_.machine_number();
        if (node->machines.empty())
            compute_structures(node);
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->job_number
                << " f " << node_tmp->forward
                << " cf0 " << node_tmp->machines[0].time_forward
                << " cfm " << node_tmp->machines[m - 1].time_forward
                << " cb0 " << node_tmp->machines[0].time_backward
                << " cbm " << node_tmp->machines[m - 1].time_backward
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->j
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

        std::vector<JobId> jobs_forward;
        std::vector<JobId> jobs_backward;
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            if (node_tmp->father->forward) {
                jobs_forward.push_back(node_tmp->j);
            } else {
                jobs_backward.push_back(node_tmp->j);
            }
        }
        std::reverse(jobs_forward.begin(), jobs_forward.end());
        jobs_forward.insert(jobs_forward.end(), jobs_backward.begin(), jobs_backward.end());
        for (JobId j: jobs_forward)
            cert << j << " ";
    }


private:

    const Instance& instance_;
    Parameters parameters_;
    mutable std::shared_ptr<Node> best_node_;

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
        heads_(instance.job_number(), std::vector<Time>(instance.machine_number() + 1, 0)),
        tails_(instance.job_number(), std::vector<Time>(instance.machine_number() + 1, 0)),
        completion_times_(instance.job_number(), std::vector<Time>(instance.machine_number() + 1, 0))
    {
        // Compute processing_time_sums_;
        for (JobId j = 0; j < instance.job_number(); ++j)
            for (MachineId i = 0; i < instance.machine_number(); ++i)
                processing_time_sums_[j] += instance.processing_time(j, i);

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
        MachineId m = instance_.machine_number();

        // Compute father's structures.
        if (father->father != nullptr && father->jobs.empty())
            compute_structures(father);

        JobId j_next = sorted_jobs_[instance_.job_number() - father->job_number - 1];
        JobPos pos = father->next_child_pos;
        // Update father
        father->next_child_pos++;

        if (pos == 0) {
            // Compute heads_.
            for (JobPos j_pos = 0; j_pos < (JobPos)father->jobs.size(); ++j_pos) {
                JobId j = father->jobs[j_pos];
                heads_[j_pos + 1][0] = heads_[j_pos][0]
                    + instance_.processing_time(j, 0);
                for (MachineId i = 1; i < m; ++i) {
                    if (heads_[j_pos + 1][i - 1] > heads_[j_pos][i]) {
                        heads_[j_pos + 1][i] = heads_[j_pos + 1][i - 1]
                            + instance_.processing_time(j, i);
                    } else {
                        heads_[j_pos + 1][i] = heads_[j_pos][i]
                            + instance_.processing_time(j, i);
                    }
                }
            }
            // Compute completion_times_.
            for (JobPos j_pos = 0; j_pos <= (JobPos)father->jobs.size(); ++j_pos) {
                completion_times_[j_pos][0] = heads_[j_pos][0]
                    + instance_.processing_time(j_next, 0);
                for (MachineId i = 1; i < m; ++i) {
                    if (heads_[j_pos][i] > completion_times_[j_pos][i - 1]) {
                        completion_times_[j_pos][i] = heads_[j_pos][i]
                            + instance_.processing_time(j_next, i);
                    } else {
                        completion_times_[j_pos][i] = completion_times_[j_pos][i - 1]
                            + instance_.processing_time(j_next, i);
                    }
                }
            }
            // Update tails_.
            for (MachineId i = m - 1; i >= 0; --i)
                tails_[father->jobs.size()][i] = 0;
            for (JobPos j_pos = (JobPos)father->jobs.size() - 1; j_pos >= 0; --j_pos) {
                JobId j = father->jobs[j_pos];
                tails_[j_pos][m - 1] = tails_[j_pos + 1][m - 1]
                    + instance_.processing_time(j, m - 1);
                for (MachineId i = m - 2; i >= 0; --i) {
                    if (tails_[j_pos][i + 1] > tails_[j_pos + 1][i]) {
                        tails_[j_pos][i] = tails_[j_pos][i + 1]
                            + instance_.processing_time(j, i);
                    } else {
                        tails_[j_pos][i] = tails_[j_pos + 1][i]
                            + instance_.processing_time(j, i);
                    }
                }
            }
        }

        auto child = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        child->father = father;
        child->pos = pos;
        child->job_number = father->job_number + 1;
        for (MachineId i = 0; i < m; ++i)
            child->makespan = std::max(child->makespan,
                    completion_times_[pos][i] + tails_[pos][i]);
        if (father->job_number == 0)
            child->makespan = 0;
        child->guide = child->makespan;
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
        if (node_1->makespan >= node_2->makespan)
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
        return node_1->makespan < node_2->makespan;
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
        ss << node->makespan;
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
    mutable std::vector<std::vector<Time>> heads_;
    mutable std::vector<std::vector<Time>> tails_;
    mutable std::vector<std::vector<Time>> completion_times_;

};

}

}

