/**
 * Permutation flow shop scheduling problem, Makespan.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/permutationflowshopschedulingmakespan.hpp
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

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/permutationflowshopschedulingmakespan.hpp"

namespace treesearchsolver
{

namespace permutationflowshopschedulingmakespan
{

using namespace orproblems::permutationflowshopschedulingmakespan;

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
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        bool forward = true;
        JobId job_id = -1;
        JobId number_of_jobs = 0;
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
        auto father = node->father;
        node->available_jobs = father->available_jobs;
        node->available_jobs[node->job_id] = false;
        node->machines = father->machines;
        if (father->forward) {
            node->machines[0].time_forward
                += instance_.processing_time(node->job_id, 0);
            node->machines[0].remaining_processing_time
                -= instance_.processing_time(node->job_id, 0);
            for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                if (node->machines[machine_id - 1].time_forward
                        > father->machines[machine_id].time_forward) {
                    Time idle_time = node->machines[machine_id - 1].time_forward
                        - father->machines[machine_id].time_forward;
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
                        > father->machines[machine_id].time_backward) {
                    Time idle_time = node->machines[machine_id + 1].time_backward
                        - father->machines[machine_id].time_backward;
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
        //        << " n " << father->number_of_jobs
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
            MachineId m = instance_.number_of_machines();
            JobId n = instance_.number_of_jobs();
            JobPos n_forward = 0;
            JobPos n_backward = 0;
            Time bound_forward = 0;
            Time bound_backward = 0;
            for (JobId job_id_next = 0; job_id_next < n; ++job_id_next) {
                if (!father->available_jobs[job_id_next])
                    continue;
                // Forward.
                Time bf = 0;
                Time t_prec = father->machines[0].time_forward
                    + instance_.processing_time(job_id_next, 0);
                Time t = 0;
                bf = std::max(bf,
                        t_prec
                        + father->machines[0].remaining_processing_time
                        - instance_.processing_time(job_id_next, 0)
                        + father->machines[0].time_backward);
                for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                    if (t_prec > father->machines[machine_id].time_forward) {
                        t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    } else {
                        t = father->machines[machine_id].time_forward
                            + instance_.processing_time(job_id_next, machine_id);
                    }
                    bf = std::max(
                            bf,
                            t + father->machines[machine_id].remaining_processing_time
                            - instance_.processing_time(job_id_next, machine_id)
                            + father->machines[machine_id].time_backward);
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
                    = father->machines[m - 1].time_backward
                    + instance_.processing_time(job_id_next, m - 1);
                bb = std::max(bb,
                        father->machines[m - 1].time_forward
                        + father->machines[m - 1].remaining_processing_time
                        - instance_.processing_time(job_id_next, m - 1)
                        + t_prec);
                for (MachineId machine_id = m - 2; machine_id >= 0; --machine_id) {
                    if (t_prec > father->machines[machine_id].time_backward) {
                        t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    } else {
                        t = father->machines[machine_id].time_backward
                            + instance_.processing_time(job_id_next, machine_id);
                    }
                    bb = std::max(
                            bb,
                            father->machines[machine_id].time_forward
                            + father->machines[machine_id].remaining_processing_time
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

        JobId job_id_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        // Check job availibility.
        if (!father->available_jobs[job_id_next])
            return nullptr;

        // Compute new child.
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto child = std::shared_ptr<Node>(new BranchingSchemeBidirectional::Node());
        child->father = father;
        child->job_id = job_id_next;
        child->number_of_jobs = father->number_of_jobs + 1;
        // Update machines and idle_time.
        child->idle_time = father->idle_time;
        Time t = 0;
        Time t_prec = 0;
        if (father->forward) {
            t_prec = father->machines[0].time_forward
                + instance_.processing_time(job_id_next, 0);
            Time remaining_processing_time
                = father->machines[0].remaining_processing_time
                - instance_.processing_time(job_id_next, 0);
            child->weighted_idle_time += (father->machines[0].time_backward == 0)? 1:
                (double)father->machines[0].idle_time_backward / father->machines[0].time_backward;
            child->bound = std::max(child->bound,
                    t_prec
                    + remaining_processing_time
                    + father->machines[0].time_backward);
            for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                Time machine_idle_time = father->machines[machine_id].idle_time_forward;
                if (t_prec > father->machines[machine_id].time_forward) {
                    Time idle_time = t_prec - father->machines[machine_id].time_forward;
                    t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    machine_idle_time += idle_time;
                    child->idle_time += idle_time;
                } else {
                    t = father->machines[machine_id].time_forward
                        + instance_.processing_time(job_id_next, machine_id);
                }
                Time remaining_processing_time
                    = father->machines[machine_id].remaining_processing_time
                    - instance_.processing_time(job_id_next, machine_id);
                child->weighted_idle_time += (t == 0)? 1:
                    (double)machine_idle_time / t;
                child->weighted_idle_time += (father->machines[machine_id].time_backward == 0)? 1:
                    (double)father->machines[machine_id].idle_time_backward
                    / father->machines[machine_id].time_backward;
                child->bound = std::max(
                        child->bound,
                        t + remaining_processing_time
                        + father->machines[machine_id].time_backward);
                t_prec = t;
            }
        } else {
            t_prec = father->machines[m - 1].time_backward
                + instance_.processing_time(job_id_next, m - 1);
            Time remaining_processing_time
                = father->machines[m - 1].remaining_processing_time
                - instance_.processing_time(job_id_next, m - 1);
            child->weighted_idle_time += (father->machines[m - 1].time_forward == 0)? 1:
                (double)father->machines[m - 1].idle_time_forward / father->machines[m - 1].time_forward;
            child->bound = std::max(child->bound,
                    father->machines[m - 1].time_forward
                    + remaining_processing_time
                    + t_prec);
            for (MachineId machine_id = m - 2; machine_id >= 0; --machine_id) {
                Time machine_idle_time = father->machines[machine_id].idle_time_backward;
                if (t_prec > father->machines[machine_id].time_backward) {
                    Time idle_time = t_prec - father->machines[machine_id].time_backward;
                    t = t_prec + instance_.processing_time(job_id_next, machine_id);
                    machine_idle_time += idle_time;
                    child->idle_time += idle_time;
                } else {
                    t = father->machines[machine_id].time_backward
                        + instance_.processing_time(job_id_next, machine_id);
                }
                Time remaining_processing_time
                    = father->machines[machine_id].remaining_processing_time
                    - instance_.processing_time(job_id_next, machine_id);
                child->weighted_idle_time += (father->machines[machine_id].time_forward == 0)? 1:
                    (double)father->machines[machine_id].idle_time_forward
                    / father->machines[machine_id].time_forward;
                child->weighted_idle_time += (t == 0)? 1:
                    (double)machine_idle_time / t;
                child->bound = std::max(
                        child->bound,
                        father->machines[machine_id].time_forward
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
     * Outputs.
     */

    std::string display(const std::shared_ptr<Node>& node) const
    {
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->bound;
        return ss.str();
    }

    std::ostream& print_solution(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        MachineId m = instance_.number_of_machines();
        if (node->machines.empty())
            compute_structures(node);
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->number_of_jobs
                << " f " << node_tmp->forward
                << " cf0 " << node_tmp->machines[0].time_forward
                << " cfm " << node_tmp->machines[m - 1].time_forward
                << " cb0 " << node_tmp->machines[0].time_backward
                << " cbm " << node_tmp->machines[m - 1].time_backward
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->job_id
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

        std::vector<JobId> jobs_forward;
        std::vector<JobId> jobs_backward;
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            if (node_tmp->father->forward) {
                jobs_forward.push_back(node_tmp->job_id);
            } else {
                jobs_backward.push_back(node_tmp->job_id);
            }
        }
        std::reverse(jobs_forward.begin(), jobs_forward.end());
        jobs_forward.insert(jobs_forward.end(), jobs_backward.begin(), jobs_backward.end());
        for (JobId job_id: jobs_forward)
            cert << job_id << " ";
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
        JobId number_of_jobs = 0;
        Time makespan = 0;
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
        heads_(instance.number_of_jobs(), std::vector<Time>(instance.number_of_machines() + 1, 0)),
        tails_(instance.number_of_jobs(), std::vector<Time>(instance.number_of_machines() + 1, 0)),
        completion_times_(instance.number_of_jobs(), std::vector<Time>(instance.number_of_machines() + 1, 0))
    {
        // Compute processing_time_sums_;
        for (JobId job_id = 0;
                job_id < instance.number_of_jobs();
                ++job_id) {
            for (MachineId machine_id = 0;
                    machine_id < instance.number_of_machines();
                    ++machine_id) {
                processing_time_sums_[job_id]
                    += instance.processing_time(job_id, machine_id);
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
        MachineId m = instance_.number_of_machines();

        // Compute father's structures.
        if (father->father != nullptr && father->jobs.empty())
            compute_structures(father);

        JobId job_id_next = sorted_jobs_[instance_.number_of_jobs() - father->number_of_jobs - 1];
        JobPos pos = father->next_child_pos;
        // Update father
        father->next_child_pos++;

        if (pos == 0) {
            // Compute heads_.
            for (JobPos job_pos = 0;
                    job_pos < (JobPos)father->jobs.size();
                    ++job_pos) {
                JobId job_id = father->jobs[job_pos];
                heads_[job_pos + 1][0] = heads_[job_pos][0]
                    + instance_.processing_time(job_id, 0);
                for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                    if (heads_[job_pos + 1][machine_id - 1] > heads_[job_pos][machine_id]) {
                        heads_[job_pos + 1][machine_id] = heads_[job_pos + 1][machine_id - 1]
                            + instance_.processing_time(job_id, machine_id);
                    } else {
                        heads_[job_pos + 1][machine_id] = heads_[job_pos][machine_id]
                            + instance_.processing_time(job_id, machine_id);
                    }
                }
            }
            // Compute completion_times_.
            for (JobPos job_pos = 0;
                    job_pos <= (JobPos)father->jobs.size();
                    ++job_pos) {
                completion_times_[job_pos][0] = heads_[job_pos][0]
                    + instance_.processing_time(job_id_next, 0);
                for (MachineId machine_id = 1; machine_id < m; ++machine_id) {
                    if (heads_[job_pos][machine_id] > completion_times_[job_pos][machine_id - 1]) {
                        completion_times_[job_pos][machine_id] = heads_[job_pos][machine_id]
                            + instance_.processing_time(job_id_next, machine_id);
                    } else {
                        completion_times_[job_pos][machine_id] = completion_times_[job_pos][machine_id - 1]
                            + instance_.processing_time(job_id_next, machine_id);
                    }
                }
            }
            // Update tails_.
            for (MachineId machine_id = m - 1; machine_id >= 0; --machine_id)
                tails_[father->jobs.size()][machine_id] = 0;
            for (JobPos job_pos = (JobPos)father->jobs.size() - 1;
                    job_pos >= 0;
                    --job_pos) {
                JobId job_id = father->jobs[job_pos];
                tails_[job_pos][m - 1] = tails_[job_pos + 1][m - 1]
                    + instance_.processing_time(job_id, m - 1);
                for (MachineId machine_id = m - 2; machine_id >= 0; --machine_id) {
                    if (tails_[job_pos][machine_id + 1] > tails_[job_pos + 1][machine_id]) {
                        tails_[job_pos][machine_id] = tails_[job_pos][machine_id + 1]
                            + instance_.processing_time(job_id, machine_id);
                    } else {
                        tails_[job_pos][machine_id] = tails_[job_pos + 1][machine_id]
                            + instance_.processing_time(job_id, machine_id);
                    }
                }
            }
        }

        auto child = std::shared_ptr<Node>(new BranchingSchemeInsertion::Node());
        child->father = father;
        child->pos = pos;
        child->number_of_jobs = father->number_of_jobs + 1;
        for (MachineId machine_id = 0; machine_id < m; ++machine_id) {
            child->makespan = std::max(
                    child->makespan,
                    completion_times_[pos][machine_id] + tails_[pos][machine_id]);
        }
        if (father->number_of_jobs == 0)
            child->makespan = 0;
        child->guide = child->makespan;
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
        if (node_1->makespan >= node_2->makespan)
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
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->makespan;
        return ss.str();
    }

    std::ostream& print_solution(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            os << "node_tmp"
                << " n " << node_tmp->number_of_jobs
                << " c " << node_tmp->makespan
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
    mutable std::vector<std::vector<Time>> heads_;
    mutable std::vector<std::vector<Time>> tails_;
    mutable std::vector<std::vector<Time>> completion_times_;

};

}

}

