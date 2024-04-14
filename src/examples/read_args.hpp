#include "treesearchsolver/greedy.hpp"
#include "treesearchsolver/best_first_search.hpp"
#include "treesearchsolver/iterative_beam_search.hpp"
#include "treesearchsolver/iterative_beam_search_2.hpp"
#include "treesearchsolver/iterative_memory_bounded_best_first_search.hpp"
#include "treesearchsolver/anytime_column_search.hpp"

#include <boost/program_options.hpp>

namespace treesearchsolver
{

boost::program_options::options_description setup_args()
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("input,i", boost::program_options::value<std::string>()->required(), "set input path (required)")
        ("output,o", boost::program_options::value<std::string>()->default_value(""), "set JSON output path")
        ("certificate,c", boost::program_options::value<std::string>()->default_value(""), "set certificate path")
        ("format,f", boost::program_options::value<std::string>()->default_value(""), "set input file format (default: orlibrary)")
        ("algorithm,a", boost::program_options::value<std::string>()->default_value("iterative-beam-search"), "set algorithm")
        ("time-limit,t", boost::program_options::value<double>(), "set time limit in seconds\n  ex: 3600")
        ("verbosity-level,v", boost::program_options::value<int>(), "set verbosity level")
        ("only-write-at-the-end,e", "only write output and certificate files at the end")
        ("log,l", boost::program_options::value<std::string>(), "set log file")
        ("log-to-stderr", "write log to stderr")
        ("print-checker", boost::program_options::value<int>()->default_value(1), "print checker")

        ("maximum-number-of-nodes", boost::program_options::value<int>(), "set the maximum number of nodes")
        ("growth-factor", boost::program_options::value<double>(), "set the growth factor")
        ("minimum-size-of-the-queue", boost::program_options::value<int>(), "set the minimum size of the queue")
        ("maximum-size-of-the-queue", boost::program_options::value<int>(), "set the maximum size of the queue")
        ("initial-column-size", boost::program_options::value<int>(), "set the initial column size")
        ("maximum-number-of-iterations", boost::program_options::value<int>(), "set the maximum number of iterations")
        ;
    return desc;
}

template <typename BranchingScheme>
void read_args(
        Parameters<BranchingScheme>& parameters,
        const boost::program_options::variables_map& vm)
{
    parameters.timer.set_sigint_handler();
    parameters.messages_to_stdout = true;
    if (vm.count("time-limit"))
        parameters.timer.set_time_limit(vm["time-limit"].as<double>());
    if (vm.count("verbosity-level"))
        parameters.verbosity_level = vm["verbosity-level"].as<int>();
    if (vm.count("log"))
        parameters.log_path = vm["log"].as<std::string>();
    parameters.log_to_stderr = vm.count("log-to-stderr");
    bool only_write_at_the_end = vm.count("only-write-at-the-end");
    if (!only_write_at_the_end) {
        std::string certificate_path = vm["certificate"].as<std::string>();
        std::string json_output_path = vm["output"].as<std::string>();
        parameters.new_solution_callback = [
            json_output_path,
            certificate_path](
                    const Output<BranchingScheme>& output)
        {
            output.write_json_output(json_output_path);
            solution_write(
                    output.solution_pool.branching_scheme(),
                    output.solution_pool.best(),
                    certificate_path);
        };
    }
}

template <typename BranchingScheme>
void write_output(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm,
        const Output<BranchingScheme>& output)
{
    // Write solution.
    if (vm.count("certificate")) {
        solution_write(
                branching_scheme,
                output.solution_pool.best(),
                vm["certificate"].as<std::string>());
    }
    // Write JSON output.
    if (vm.count("output"))
        output.write_json_output(vm["output"].as<std::string>());
}

template <typename BranchingScheme>
const Output<BranchingScheme> run_greedy(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm)
{
    Parameters<BranchingScheme> parameters;
    read_args(parameters, vm);
    const Output<BranchingScheme> output = greedy(branching_scheme, parameters);
    write_output(branching_scheme, vm, output);
    return output;
}

template <typename BranchingScheme>
const Output<BranchingScheme> run_best_first_search(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm)
{
    BestFirstSearchParameters<BranchingScheme> parameters;
    read_args(parameters, vm);
    if (vm.count("maximum-number-of-nodes"))
        parameters.maximum_number_of_nodes = vm["maximum-number-of-nodes"].as<int>();
    const Output<BranchingScheme> output = best_first_search(branching_scheme, parameters);
    write_output(branching_scheme, vm, output);
    return output;
}

template <typename BranchingScheme>
const Output<BranchingScheme> run_iterative_beam_search(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm)
{
    IterativeBeamSearchParameters<BranchingScheme> parameters;
    read_args(parameters, vm);
    if (vm.count("growth-factor"))
        parameters.growth_factor = vm["growth-factor"].as<double>();
    if (vm.count("minimum-size-of-the-queue"))
        parameters.minimum_size_of_the_queue = vm["minimum-size-of-the-queue"].as<int>();
    if (vm.count("maximum-size-of-the-queue"))
        parameters.maximum_size_of_the_queue = vm["maximum-size-of-the-queue"].as<int>();
    if (vm.count("maximum-number-of-nodes"))
        parameters.maximum_number_of_nodes = vm["maximum-number-of-nodes"].as<int>();
    const Output<BranchingScheme> output = iterative_beam_search(branching_scheme, parameters);
    write_output(branching_scheme, vm, output);
    return output;
}

template <typename BranchingScheme>
const Output<BranchingScheme> run_iterative_beam_search_2(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm)
{
    IterativeBeamSearch2Parameters<BranchingScheme> parameters;
    read_args(parameters, vm);
    if (vm.count("growth-factor"))
        parameters.growth_factor = vm["growth-factor"].as<double>();
    if (vm.count("minimum-size-of-the-queue"))
        parameters.minimum_size_of_the_queue = vm["minimum-size-of-the-queue"].as<int>();
    if (vm.count("maximum-size-of-the-queue"))
        parameters.maximum_size_of_the_queue = vm["maximum-size-of-the-queue"].as<int>();
    if (vm.count("maximum-number-of-nodes"))
        parameters.maximum_number_of_nodes_expanded = vm["maximum-number-of-nodes"].as<int>();
    const Output<BranchingScheme> output = iterative_beam_search_2(branching_scheme, parameters);
    write_output(branching_scheme, vm, output);
    return output;
}

template <typename BranchingScheme>
const Output<BranchingScheme> run_iterative_memory_bounded_best_first_search(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm)
{
    IterativeMemoryBoundedBestFirstSearchParameters<BranchingScheme> parameters;
    read_args(parameters, vm);
    if (vm.count("growth-factor"))
        parameters.growth_factor = vm["growth-factor"].as<double>();
    if (vm.count("minimum-size-of-the-queue"))
        parameters.minimum_size_of_the_queue = vm["minimum-size-of-the-queue"].as<int>();
    if (vm.count("maximum-size-of-the-queue"))
        parameters.maximum_size_of_the_queue = vm["maximum-size-of-the-queue"].as<int>();
    if (vm.count("maximum-number-of-nodes"))
        parameters.maximum_number_of_nodes = vm["maximum-number-of-nodes"].as<int>();
    const Output<BranchingScheme> output = iterative_memory_bounded_best_first_search(branching_scheme, parameters);
    write_output(branching_scheme, vm, output);
    return output;
}

template <typename BranchingScheme>
const Output<BranchingScheme> run_anytime_column_search(
        const BranchingScheme& branching_scheme,
        const boost::program_options::variables_map& vm)
{
    AnytimeColumnSearchParameters<BranchingScheme> parameters;
    read_args(parameters, vm);
    parameters.initial_column_size = vm["initial-column-size"].as<int>();
    if (vm.count("growth-factor"))
        parameters.column_size_growth_factor = vm["growth-factor"].as<double>();
    if (vm.count("maximum-number-of-nodes"))
        parameters.maximum_number_of_nodes = vm["maximum-number-of-nodes"].as<int>();
    if (vm.count("maximum-number-of-iterations"))
        parameters.maximum_number_of_iterations = vm["maximum-number-of-iterations"].as<int>();
    const Output<BranchingScheme> output = anytime_column_search(branching_scheme, parameters);
    write_output(branching_scheme, vm, output);
    return output;
}

}
