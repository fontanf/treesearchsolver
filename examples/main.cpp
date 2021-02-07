#include "examples/travellingsalesman.hpp"
#include "examples/sequentialordering.hpp"

#include "treesearchsolver/read_args.hpp"

#include <boost/program_options.hpp>

using namespace treesearchsolver;

namespace po = boost::program_options;

template <typename BranchingScheme>
void run(
        std::string algorithm,
        const BranchingScheme& branching_scheme,
        const optimizationtools::Info& info)
{
    std::vector<std::string> algorithm_args
        = boost::program_options::split_unix(algorithm);
    std::vector<char*> algorithm_argv;
    for (Counter i = 0; i < (Counter)algorithm_args.size(); ++i)
        algorithm_argv.push_back(const_cast<char*>(algorithm_args[i].c_str()));

    if (algorithm_args[0] == "astar") {
        auto parameters = read_astar_args(algorithm_argv);
        parameters.info = info;
        astar(branching_scheme, parameters);
    } else if (algorithm_args[0] == "iterativebeamsearch") {
        auto parameters = read_iterativebeamsearch_args(algorithm_argv);
        parameters.info = info;
        iterativebeamsearch(branching_scheme, parameters);
    } else if (algorithm_args[0] == "iterativememoryboundedastar") {
        auto parameters = read_iterativememoryboundedastar_args(algorithm_argv);
        parameters.info = info;
        iterativememoryboundedastar(branching_scheme, parameters);
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: '" << algorithm_args[0] << "'.\033[0m" << std::endl;
    }
}

int main(int argc, char *argv[])
{

    // Parse program options

    std::string problem = "travellingsalesman";
    std::string instance_path = "";
    std::string output_path = "";
    std::string certificate_path = "";
    std::string format = "";
    std::string algorithm = "astar";
    std::string branching_scheme_parameters = "";
    std::string columngeneration_args_string = "";
    double time_limit = std::numeric_limits<double>::infinity();

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("problem,p", po::value<std::string>(&problem)->required(), "set problem (required)")
        ("input,i", po::value<std::string>(&instance_path)->required(), "set input path (required)")
        ("output,o", po::value<std::string>(&output_path), "set JSON output path")
        ("certificate,c", po::value<std::string>(&certificate_path), "set certificate path")
        ("format,f", po::value<std::string>(&format), "set input file format (default: orlibrary)")
        ("algorithm,a", po::value<std::string>(&algorithm), "set algorithm")
        ("branching-scheme,b", po::value<std::string>(&branching_scheme_parameters), "set branchingscheme parameters")
        ("time-limit,t", po::value<double>(&time_limit), "Time limit in seconds\n  ex: 3600")
        ("only-write-at-the-end,e", "Only write output and certificate files at the end")
        ("verbose,v", "")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;;
        return 1;
    }
    try {
        po::notify(vm);
    } catch (const po::required_option& e) {
        std::cout << desc << std::endl;;
        return 1;
    }

    optimizationtools::Info info = optimizationtools::Info()
        .set_verbose(vm.count("verbose"))
        .set_timelimit(time_limit)
        .set_outputfile(output_path)
        .set_onlywriteattheend(vm.count("only-write-at-the-end"))
        ;

    // Run algorithm

    if (problem == "travellingsalesman") {
        travellingsalesman::Instance instance(instance_path, format);
        travellingsalesman::BranchingScheme branching_scheme(instance);
        run(algorithm, branching_scheme, info);
    } else if (problem == "sequentialordering") {
        sequentialordering::Instance instance(instance_path, format);
        sequentialordering::BranchingScheme branching_scheme(instance);
        run(algorithm, branching_scheme, info);
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown problem: '" << problem << "'.\033[0m" << std::endl;
        return 1;
    }

    return 0;
}

