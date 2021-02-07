#include "treesearchsolver/algorithms/a_star.hpp"
#include "treesearchsolver/algorithms/iterative_beam_search.hpp"
#include "treesearchsolver/algorithms/iterative_memory_bounded_a_star.hpp"

#include <boost/program_options.hpp>

namespace treesearchsolver
{

inline AStarOptionalParameters read_astar_args(
        const std::vector<char*> argv)
{
    AStarOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("node-number-max,n", boost::program_options::value<NodeId>(&parameters.node_number_max), "")
        ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        boost::program_options::notify(vm);
    } catch (const boost::program_options::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

inline IterativeBeamSearchOptionalParameters read_iterativebeamsearch_args(
        const std::vector<char*> argv)
{
    IterativeBeamSearchOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("node-number-max,n", boost::program_options::value<NodeId>(&parameters.node_number_max), "")
        ("growth-factor,f", boost::program_options::value<double>(&parameters.growth_factor), "")
        ("queue-size-min,m", boost::program_options::value<NodeId>(&parameters.queue_size_min), "")
        ("queue-size-max,M", boost::program_options::value<NodeId>(&parameters.queue_size_max), "")
        ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        boost::program_options::notify(vm);
    } catch (const boost::program_options::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

inline IterativeMemoryBoundedAStarOptionalParameters read_iterativememoryboundedastar_args(
        const std::vector<char*> argv)
{
    IterativeMemoryBoundedAStarOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("node-number-max,n", boost::program_options::value<NodeId>(&parameters.node_number_max), "")
        ("growth-factor,f", boost::program_options::value<double>(&parameters.growth_factor), "")
        ("queue-size-min,m", boost::program_options::value<NodeId>(&parameters.queue_size_min), "")
        ("queue-size-max,M", boost::program_options::value<NodeId>(&parameters.queue_size_max), "")
        ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        boost::program_options::notify(vm);
    } catch (const boost::program_options::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return parameters;
}

}
