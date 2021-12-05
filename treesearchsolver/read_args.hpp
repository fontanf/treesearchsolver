#include "treesearchsolver/greedy.hpp"
#include "treesearchsolver/best_first_search.hpp"
#include "treesearchsolver/iterative_beam_search.hpp"
#include "treesearchsolver/iterative_memory_bounded_best_first_search.hpp"

#include <boost/program_options.hpp>

namespace treesearchsolver
{

inline GreedyOptionalParameters read_greedy_args(
        const std::vector<char*> argv)
{
    GreedyOptionalParameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
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

template <typename BranchingScheme>
SolutionPool<BranchingScheme> run_greedy(
        const std::vector<char*>& algorithm_argv,
        const BranchingScheme& branching_scheme,
        const optimizationtools::Info& info)
{
    auto parameters = read_greedy_args(algorithm_argv);
    parameters.info = info;
    return greedy(branching_scheme, parameters).solution_pool;
}

template <typename BranchingScheme>
SolutionPool<BranchingScheme> run_best_first_search(
        const std::vector<char*>& algorithm_argv,
        const BranchingScheme& branching_scheme,
        const optimizationtools::Info& info)
{
    auto parameters = read_best_first_search_args(algorithm_argv);
    parameters.info = info;
    return best_first_search(branching_scheme, parameters).solution_pool;
}

template <typename BranchingScheme>
SolutionPool<BranchingScheme> run_iterative_beam_search(
        const std::vector<char*>& algorithm_argv,
        const BranchingScheme& branching_scheme,
        const optimizationtools::Info& info)
{
    auto parameters = read_iterative_beam_search_args(algorithm_argv);
    parameters.info = info;
    return iterative_beam_search(branching_scheme, parameters).solution_pool;
}

template <typename BranchingScheme>
SolutionPool<BranchingScheme> run_iterative_memory_bounded_best_first_search(
        const std::vector<char*>& algorithm_argv,
        const BranchingScheme& branching_scheme,
        const optimizationtools::Info& info)
{
    auto parameters = read_iterative_memory_bounded_best_first_search_args(algorithm_argv);
    parameters.info = info;
    return iterative_memory_bounded_best_first_search(branching_scheme, parameters).solution_pool;
}

struct MainArgs
{
    std::string instance_path = "";
    std::string format = "";
    std::vector<std::string> algorithm_args;
    std::vector<char*> algorithm_argv;
    std::vector<std::string> branching_scheme_args;
    std::vector<char*> branching_scheme_argv;
    optimizationtools::Info info = optimizationtools::Info();
    bool print_instance = false;
    bool print_solution = false;
};

MainArgs read_args(int argc, char *argv[])
{
    MainArgs main_args;
    std::string output_path = "";
    std::string certificate_path = "";
    std::string algorithm = "iterative_beam_search";
    std::string branching_scheme_parameters = "forward";
    double time_limit = std::numeric_limits<double>::infinity();

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("input,i", boost::program_options::value<std::string>(&main_args.instance_path)->required(), "set input path (required)")
        ("output,o", boost::program_options::value<std::string>(&output_path), "set JSON output path")
        ("certificate,c", boost::program_options::value<std::string>(&certificate_path), "set certificate path")
        ("format,f", boost::program_options::value<std::string>(&main_args.format), "set input file format (default: orlibrary)")
        ("algorithm,a", boost::program_options::value<std::string>(&algorithm), "set algorithm")
        ("branching-scheme,b", boost::program_options::value<std::string>(&branching_scheme_parameters), "set branchingscheme parameters")
        ("time-limit,t", boost::program_options::value<double>(&time_limit), "Time limit in seconds\n  ex: 3600")
        ("only-write-at-the-end,e", "Only write output and certificate files at the end")
        ("verbose,v", "")
        ("print-instance", "")
        ("print-solution", "")
        ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;;
        throw "";
    }
    try {
        boost::program_options::notify(vm);
    } catch (const boost::program_options::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }

    main_args.print_instance = (vm.count("print-instance"));
    main_args.print_solution = (vm.count("print-solution"));

    main_args.algorithm_args = boost::program_options::split_unix(algorithm);
    for (std::string& s: main_args.algorithm_args)
        main_args.algorithm_argv.push_back(const_cast<char*>(s.c_str()));

    main_args.branching_scheme_args = boost::program_options::split_unix(branching_scheme_parameters);
    for (std::string& s: main_args.branching_scheme_args)
        main_args.branching_scheme_argv.push_back(const_cast<char*>(s.c_str()));

    main_args.info = optimizationtools::Info()
        .set_verbose(vm.count("verbose"))
        .set_time_limit(time_limit)
        .set_certificate_path(certificate_path)
        .set_json_output_path(output_path)
        .set_only_write_at_the_end(vm.count("only-write-at-the-end"))
        .set_only_write_at_the_end(true)
        .set_sigint_handler()
        ;

    return main_args;
}

}
