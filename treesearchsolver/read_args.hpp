#include "treesearchsolver/best_first_search.hpp"
#include "treesearchsolver/iterative_beam_search.hpp"
#include "treesearchsolver/iterative_memory_bounded_best_first_search.hpp"

#include <boost/program_options.hpp>

namespace treesearchsolver
{

inline BestFirstSearchOptionalParameters read_best_first_search_args(
        const std::vector<char*> argv)
{
    BestFirstSearchOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("maximum-number-of-nodes,n", boost::program_options::value<NodeId>(&parameters.maximum_number_of_nodes), "")
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

inline IterativeBeamSearchOptionalParameters read_iterative_beam_search_args(
        const std::vector<char*> argv)
{
    IterativeBeamSearchOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("maximum-number-of-nodes,n", boost::program_options::value<NodeId>(&parameters.maximum_number_of_nodes), "")
        ("growth-factor,f", boost::program_options::value<double>(&parameters.growth_factor), "")
        ("minimum-size-of-the-queue,m", boost::program_options::value<NodeId>(&parameters.minimum_size_of_the_queue), "")
        ("maximum-size-of-the-queue,M", boost::program_options::value<NodeId>(&parameters.maximum_size_of_the_queue), "")
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

inline IterativeMemoryBoundedBestFirstSearchOptionalParameters read_iterative_memory_bounded_best_first_search_args(
        const std::vector<char*> argv)
{
    IterativeMemoryBoundedBestFirstSearchOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("maximum-number-of-nodes,n", boost::program_options::value<NodeId>(&parameters.maximum_number_of_nodes), "")
        ("growth-factor,f", boost::program_options::value<double>(&parameters.growth_factor), "")
        ("minimum-size-of-the-queue,m", boost::program_options::value<NodeId>(&parameters.minimum_size_of_the_queue), "")
        ("maximum-size-of-the-queue,M", boost::program_options::value<NodeId>(&parameters.maximum_size_of_the_queue), "")
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
