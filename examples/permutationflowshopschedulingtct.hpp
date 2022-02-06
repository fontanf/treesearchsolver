#pragma once

/**
 * Permutation flow shop scheduling problem, Total completion time.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/permutationflowshopschedulingtct.hpp
 *
 * Tree search:
 * - Forward branching
 * - Guide:
 *   - 0: total completion time
 *   - 1: idle time
 *   - 2: weighted idle time
 *   - 3: total completion time and weighted idle time
 */

#include "optimizationtools/info.hpp"
#include "optimizationtools/utils.hpp"
#include "optimizationtools/indexed_set.hpp"

#include "orproblems/permutationflowshopschedulingtct.hpp"

namespace treesearchsolver
{

namespace permutationflowshopschedulingtct
{

using namespace orproblems::permutationflowshopschedulingtct;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        JobId j = -1;
        JobId number_of_jobs = 0;
        std::vector<Time> times;
        Time total_completion_time = 0;
        Time idle_time = 0;
        double weighted_idle_time = 0;
        Time bound = 0;
        double guide = 0;
        JobId next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 2;
    };

    BranchingScheme(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    {
    }

    inline const std::shared_ptr<Node> root() const
    {
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_jobs.resize(n, true);
        r->times.resize(m, 0);
        r->bound = 0;
        for (JobId j = 0; j < n; ++j)
            r->bound += instance_.processing_time(j, m - 1);
        return r;
    }

    inline void compute_structures(
            const std::shared_ptr<Node>& node) const
    {
        MachineId m = instance_.number_of_machines();
        auto father = node->father;
        node->available_jobs = father->available_jobs;
        node->available_jobs[node->j] = false;
        node->times = father->times;
        node->times[0] = father->times[0]
            + instance_.processing_time(node->j, 0);
        for (MachineId i = 1; i < m; ++i) {
            if (node->times[i - 1] > father->times[i]) {
                node->times[i] = node->times[i - 1]
                    + instance_.processing_time(node->j, i);
            } else {
                node->times[i] = father->times[i]
                    + instance_.processing_time(node->j, i);
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

        JobId j_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        // Check job availibility.
        if (!father->available_jobs[j_next])
            return nullptr;

        // Compute new child.
        MachineId m = instance_.number_of_machines();
        JobId n = instance_.number_of_jobs();
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->j = j_next;
        child->number_of_jobs = father->number_of_jobs + 1;
        child->idle_time = father->idle_time;
        child->weighted_idle_time = father->weighted_idle_time;
        Time t_prec = father->times[0]
            + instance_.processing_time(j_next, 0);
        Time t = 0;
        for (MachineId i = 1; i < m; ++i) {
            if (t_prec > father->times[i]) {
                Time idle_time = t_prec - father->times[i];
                t = t_prec
                    + instance_.processing_time(j_next, i);
                child->idle_time += idle_time;
                child->weighted_idle_time += ((double)father->number_of_jobs / n + 1) * (m - i) * idle_time;
            } else {
                t = father->times[i]
                    + instance_.processing_time(j_next, i);
            }
            t_prec = t;
        }
        child->total_completion_time = father->total_completion_time + t;
        // Compute bound.
        child->bound = father->bound
            + (n - father->number_of_jobs) * (t - father->times[m - 1])
            - instance_.processing_time(j_next, m - 1);
        // Compute guide.
        double alpha = (double)child->number_of_jobs / instance_.number_of_jobs();
        switch (parameters_.guide_id) {
        case 0: {
            child->guide = child->bound;
            break;
        } case 1: {
            child->guide = child->idle_time;
            break;
        } case 2: {
            child->guide = alpha * child->total_completion_time
                + (1.0 - alpha) * child->idle_time * child->number_of_jobs / m;
            break;
        } case 3: {
            //child->guide = alpha * child->total_completion_time
            //    + (1.0 - alpha) * (child->weighted_idle_time + m * child->idle_time) / 2;
            child->guide = alpha * child->total_completion_time
                + (1.0 - alpha) * (child->weighted_idle_time / m + child->idle_time) / 2 * child->number_of_jobs / m;
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
        if (node_1->bound >= node_2->total_completion_time)
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
        return node_1->total_completion_time < node_2->total_completion_time;
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
        if (node_1->total_completion_time <= node_2->total_completion_time) {
            bool dominates = true;
            for (MachineId i = 0; i < instance_.number_of_machines(); ++i) {
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
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->total_completion_time;
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
                << " n " << node_tmp->number_of_jobs
                << " c " << node_tmp->times[instance_.number_of_machines() - 1]
                << " tct " << node_tmp->total_completion_time
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

