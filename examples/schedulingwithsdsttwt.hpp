/**
 * Single machine scheduling problem with sequence-dependent setup times, Total
 * weighted Tardiness.
 *
 * Problem description:
 * See https://github.com/fontanf/orproblems/blob/main/orproblems/schedulingwithsdsttwt.hpp
 *
 * Tree search:
 * - forward branching
 *
 */

#pragma once

#include "optimizationtools/utils/info.hpp"
#include "optimizationtools/utils/utils.hpp"
#include "optimizationtools/containers/sorted_on_demand_array.hpp"
#include "optimizationtools/containers/indexed_set.hpp"

#include "orproblems/schedulingwithsdsttwt.hpp"

namespace treesearchsolver
{

namespace schedulingwithsdsttwt
{

using namespace orproblems::schedulingwithsdsttwt;

using GuideId = int64_t;

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
        JobId job_id = 0;
        JobId number_of_jobs = 0;
        Time current_time = 0;
        double total_weighted_earliness = 0;
        double total_weighted_tardiness = 0;
        Time bound = 0;
        double guide = 0;
        JobPos next_child_pos = 0;
    };

    BranchingScheme(
            const Instance& instance,
            Parameters parameters):
        instance_(instance),
        parameters_(parameters)
    { }

    inline const std::shared_ptr<Node> root() const
    {
        auto r = std::shared_ptr<Node>(new BranchingScheme::Node());
        r->visited.resize(instance_.number_of_jobs(), false);
        r->guide = r->bound;
        r->job_id = instance_.number_of_jobs();
        return r;
    }

    inline std::shared_ptr<Node> next_child(
            const std::shared_ptr<Node>& father) const
    {
        assert(!infertile(father));
        assert(!leaf(father));

        JobId job_id_next = father->next_child_pos;
        // Update father
        father->next_child_pos++;
        if (father->visited[job_id_next])
            return nullptr;
        Weight wj = instance_.job(job_id_next).weight;
        if (wj == 0 && father->number_of_jobs < instance_.number_of_jobs() - instance_.number_of_zero_weight_jobs())
            return nullptr;

        // Compute new child.
        auto child = std::shared_ptr<Node>(new BranchingScheme::Node());
        child->father = father;
        child->visited = father->visited;
        child->visited[job_id_next] = true;
        child->job_id = job_id_next;
        child->number_of_jobs = father->number_of_jobs + 1;
        child->current_time = father->current_time
            + instance_.setup_time(father->job_id, job_id_next)
            + instance_.job(job_id_next).processing_time;

        child->total_weighted_tardiness = father->total_weighted_tardiness;
        if (child->current_time > instance_.job(job_id_next).due_date)
            child->total_weighted_tardiness += (child->current_time - instance_.job(job_id_next).due_date) * wj;

        child->total_weighted_earliness = father->total_weighted_earliness;
        if (child->current_time < instance_.job(job_id_next).due_date)
            child->total_weighted_earliness += (double)(instance_.job(job_id_next).due_date - child->current_time) / wj;

        child->guide = 10 * child->current_time
            + child->total_weighted_earliness
            + child->total_weighted_tardiness;
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
        return node_1->bound >= node_2->total_weighted_tardiness;
    }

    /*
     * Solution pool.
     */

    bool better(
            const std::shared_ptr<Node>& node_1,
            const std::shared_ptr<Node>& node_2) const
    {
        if (node_1->number_of_jobs < instance_.number_of_jobs())
            return false;
        if (node_2->number_of_jobs < instance_.number_of_jobs())
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
        std::hash<JobId> hasher_1;
        std::hash<std::vector<bool>> hasher_2;

        inline bool operator()(
                const std::shared_ptr<Node>& node_1,
                const std::shared_ptr<Node>& node_2) const
        {
            if (node_1->job_id != node_2->job_id)
                return false;
            return node_1->visited == node_2->visited;
        }

        inline std::size_t operator()(
                const std::shared_ptr<Node>& node) const
        {
            size_t hash = hasher_1(node->job_id);
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
        if (node->number_of_jobs != instance_.number_of_jobs())
            return "";
        std::stringstream ss;
        ss << node->total_weighted_tardiness;
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
                << " t " << node_tmp->current_time
                << " twt " << node_tmp->total_weighted_tardiness
                << " twe " << node_tmp->total_weighted_earliness
                << " bnd " << node_tmp->bound
                << " j " << node_tmp->job_id
                << " dj " << instance_.job(node_tmp->job_id).due_date
                << " wj " << instance_.job(node_tmp->job_id).weight
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

        std::vector<JobId> jobs;
        for (auto node_tmp = node;
                node_tmp->father != nullptr;
                node_tmp = node_tmp->father) {
            jobs.push_back(node_tmp->job_id);
        }
        std::reverse(jobs.begin(), jobs.end());
        for (JobId job_id: jobs)
            cert << job_id << " ";
    }

private:

    const Instance& instance_;
    Parameters parameters_;

};

}

}

