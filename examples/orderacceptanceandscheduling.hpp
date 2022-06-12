#pragma once

/**
 * Single machine order acceptance and scheduling problem with
 * sequence-dependent setup times.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/orderacceptanceandscheduling.hpp
 *
 * Tree search
 * - forward branching
 * - guide: time / profit
 */

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/orderacceptanceandscheduling.hpp"

namespace treesearchsolver
{

namespace orderacceptanceandscheduling
{

using namespace orproblems::orderacceptanceandscheduling;

using GuideId = int64_t;

class BranchingScheme
{

public:

    struct Node
    {
        std::shared_ptr<Node> father = nullptr;
        std::vector<bool> available_jobs;
        JobId j = 0;
        JobId number_of_jobs = 0;
        Time time = 0;
        Profit profit = 0;
        Weight weighted_tardiness = 0;
        double guide = 0;
        JobPos next_child_pos = 0;
    };

    struct Parameters
    {
        GuideId guide_id = 1;
    };

    BranchingScheme(const Instance& instance, Parameters parameters):
        instance_(instance),
        parameters_(parameters),
        sorted_jobs_(instance.number_of_jobs() + 1),
        generator_(0)
    {
        // Initialize sorted_jobs_.
        for (JobId j = 0; j < instance_.number_of_jobs(); ++j) {
            sorted_jobs_[j].reset(instance.number_of_jobs());
            for (JobId j2 = 0; j2 < instance_.number_of_jobs(); ++j2) {
                double c = (double)instance_.setup_time(j, j2) / instance_.job(j2).profit;
                sorted_jobs_[j].set_cost(j2, c);
            }
        }
    }

    inline JobId neighbor(JobId j, JobPos pos) const
    {
        assert(j < instance_.number_of_jobs());
        assert(pos < instance_.number_of_jobs());
        return sorted_jobs_[j].get(pos, generator_);
    }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->available_jobs.resize(instance_.number_of_jobs(), true);
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
        child->number_of_jobs = father->number_of_jobs + 1;
        child->time = start + p;
        child->profit = father->profit + instance_.job(j_next).profit;
        child->weighted_tardiness = father->weighted_tardiness;
        Time d = instance_.job(j_next).due_date;
        if (child->time > d)
            child->weighted_tardiness += instance_.job(j_next).weight * (child->time - d);
        for (JobId j = 0; j < instance_.number_of_jobs(); ++j) {
            if (!child->available_jobs[j])
                continue;
            if (child->time
                    + instance_.setup_time(j_next, j)
                    + instance_.job(j).processing_time
                    > instance_.job(j).deadline)
                child->available_jobs[j] = false;
        }
        // Guide.
        child->guide =
            (parameters_.guide_id == 0)? child->weighted_tardiness - child->profit:
                                         (double)child->time / (child->profit - child->weighted_tardiness);
        return child;
    }

    inline bool infertile(
            const std::shared_ptr<Node>& node) const
    {
        assert(node != nullptr);
        return (node->next_child_pos == instance_.number_of_jobs() - 1);
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
        return node->number_of_jobs == instance_.number_of_jobs();
    }

    bool bound(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        (void)node_1;
        (void)node_2;
        return false;
    }

    /*
     * Solution pool.
     */

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
            << " (n" << node->number_of_jobs
            << " p" << node->profit
            << " w" << node->weighted_tardiness
            << ")";
        return ss.str();
    }

    std::ostream& print_solution(
            std::ostream &os,
            const std::shared_ptr<Node>& node)
    {
        for (auto node_tmp = node; node_tmp->father != nullptr;
                node_tmp = node_tmp->father)
            os << "node_tmp"
                << " n " << node_tmp->number_of_jobs
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

