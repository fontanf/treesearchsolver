#include "examples/travellingsalesman.hpp"
#include "examples/sequentialordering.hpp"
#include "examples/thieforienteering.hpp"

#include "treesearchsolver/read_args.hpp"

#include <boost/program_options.hpp>

using namespace treesearchsolver;

namespace po = boost::program_options;

inline GuideId read_guide(
        const std::vector<char*> argv)
{
    GuideId guide_id = 0;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("guide,g", boost::program_options::value<GuideId>(&guide_id), "")
        ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line((Counter)argv.size(), argv.data(), desc), vm);
    try {
        boost::program_options::notify(vm);
    } catch (const boost::program_options::required_option& e) {
        std::cout << desc << std::endl;;
        throw "";
    }
    return guide_id;
}

inline thieforienteering::BranchingScheme::Parameters read_thieforienteering_args(
        const std::vector<char*> argv)
{
    thieforienteering::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
        ("exponent-time,t", boost::program_options::value<double>(&parameters.exponent_time), "")
        ("exponent-weight,w", boost::program_options::value<double>(&parameters.exponent_weight), "")
        ("exponent-profit,p", boost::program_options::value<double>(&parameters.exponent_profit), "")
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
SolutionPool<BranchingScheme> run(
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
        return astar(branching_scheme, parameters).solution_pool;
    } else if (algorithm_args[0] == "iterativebeamsearch") {
        auto parameters = read_iterativebeamsearch_args(algorithm_argv);
        parameters.info = info;
        return iterativebeamsearch(branching_scheme, parameters).solution_pool;
    } else if (algorithm_args[0] == "iterativememoryboundedastar") {
        auto parameters = read_iterativememoryboundedastar_args(algorithm_argv);
        parameters.info = info;
        return iterativememoryboundedastar(branching_scheme, parameters).solution_pool;
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown algorithm: '" << algorithm_args[0] << "'.\033[0m" << std::endl;
    }
    return SolutionPool<BranchingScheme>(branching_scheme, 1);
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

    std::vector<std::string> branching_scheme_args
        = boost::program_options::split_unix(branching_scheme_parameters);
    std::vector<char*> branching_scheme_argv;
    std::string dummy = "dummy";
    branching_scheme_argv.push_back(const_cast<char*>(dummy.c_str()));
    for (Counter i = 0; i < (Counter)branching_scheme_args.size(); ++i)
        branching_scheme_argv.push_back(const_cast<char*>(branching_scheme_args[i].c_str()));

    if (problem == "travellingsalesman") {
        travellingsalesman::Instance instance(instance_path, format);
        travellingsalesman::BranchingScheme branching_scheme(instance);
        auto solution_pool = run(algorithm, branching_scheme, info);
    } else if (problem == "sequentialordering") {
        sequentialordering::Instance instance(instance_path, format);
        sequentialordering::BranchingScheme branching_scheme(instance);
        auto solution_pool = run(algorithm, branching_scheme, info);
    } else if (problem == "thieforienteering") {
        thieforienteering::Instance instance(instance_path, format);
        auto parameters = read_thieforienteering_args(branching_scheme_argv);
        thieforienteering::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown problem: '" << problem << "'.\033[0m" << std::endl;
        return 1;
    }

    return 0;
}

