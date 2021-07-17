#include "treesearchsolver/read_args.hpp"
#include "examples/read_args.hpp"

using namespace treesearchsolver;

namespace po = boost::program_options;

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
    std::string algorithm = "iterativebeamsearch";
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
        ("print-instance", "")
        ("print-solution", "")
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
        //.set_onlywriteattheend(vm.count("only-write-at-the-end"))
        .set_onlywriteattheend(true)
        ;

    // Run algorithm

    std::vector<std::string> branching_scheme_args
        = boost::program_options::split_unix(branching_scheme_parameters);
    std::vector<char*> branching_scheme_argv;
    std::string dummy = "dummy";
    branching_scheme_argv.push_back(const_cast<char*>(dummy.c_str()));
    for (Counter i = 0; i < (Counter)branching_scheme_args.size(); ++i)
        branching_scheme_argv.push_back(const_cast<char*>(branching_scheme_args[i].c_str()));

    if (problem == "knapsackwithconflicts") {
        knapsackwithconflicts::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_knapsackwithconflicts_args(branching_scheme_argv);
        knapsackwithconflicts::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "travellingsalesman") {
        travellingsalesman::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        if (branching_scheme_args.empty())
            branching_scheme_args.push_back("insertion");
        if (branching_scheme_args[0] == "forward") {
            auto parameters = read_travellingsalesman_forward_args(branching_scheme_argv);
            travellingsalesman::BranchingSchemeForward branching_scheme(instance, parameters);
            auto solution_pool = run(algorithm, branching_scheme, info);
            branching_scheme.write(solution_pool.best(), certificate_path);
            if (vm.count("print-solution"))
                branching_scheme.print(std::cout, solution_pool.best());
        } else {
            auto parameters = read_travellingsalesman_insertion_args(branching_scheme_argv);
            travellingsalesman::BranchingSchemeInsertion branching_scheme(instance, parameters);
            auto solution_pool = run(algorithm, branching_scheme, info);
            branching_scheme.write(solution_pool.best(), certificate_path);
            if (vm.count("print-solution"))
                branching_scheme.print(std::cout, solution_pool.best());
        }

    } else if (problem == "sequentialordering") {
        sequentialordering::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_sequentialordering_args(branching_scheme_argv);
        sequentialordering::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "travellingrepairman") {
        travellingrepairman::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_travellingrepairman_forward_args(branching_scheme_argv);
        travellingrepairman::BranchingSchemeForward branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "thieforienteering") {
        thieforienteering::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_thieforienteering_args(branching_scheme_argv);
        thieforienteering::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "schedulingwithsdsttwt") {
        schedulingwithsdsttwt::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_schedulingwithsdsttwt_args(branching_scheme_argv);
        schedulingwithsdsttwt::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "orderacceptanceandscheduling") {
        orderacceptanceandscheduling::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_orderacceptanceandscheduling_args(branching_scheme_argv);
        orderacceptanceandscheduling::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "batchschedulingtotalweightedtardiness") {
        batchschedulingtotalweightedtardiness::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_batchschedulingtotalweightedtardiness_args(branching_scheme_argv);
        batchschedulingtotalweightedtardiness::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "permutationflowshopschedulingmakespan") {
        permutationflowshopschedulingmakespan::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        if (branching_scheme_args.empty())
            branching_scheme_args.push_back("bidirectional");
        if (branching_scheme_args[0] == "bidirectional") {
            auto parameters = read_permutationflowshopschedulingmakespan_bidirectional_args(branching_scheme_argv);
            permutationflowshopschedulingmakespan::BranchingSchemeBidirectional branching_scheme(instance, parameters);
            auto solution_pool = run(algorithm, branching_scheme, info);
            branching_scheme.write(solution_pool.best(), certificate_path);
            if (vm.count("print-solution"))
                branching_scheme.print(std::cout, solution_pool.best());
        } else {
            auto parameters = read_permutationflowshopschedulingmakespan_insertion_args(branching_scheme_argv);
            permutationflowshopschedulingmakespan::BranchingSchemeInsertion branching_scheme(instance, parameters);
            auto solution_pool = run(algorithm, branching_scheme, info);
            branching_scheme.write(solution_pool.best(), certificate_path);
            if (vm.count("print-solution"))
                branching_scheme.print(std::cout, solution_pool.best());
        }

    } else if (problem == "permutationflowshopschedulingtct") {
        permutationflowshopschedulingtct::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_permutationflowshopschedulingtct_args(branching_scheme_argv);
        permutationflowshopschedulingtct::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "permutationflowshopschedulingtt") {
        permutationflowshopschedulingtt::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        if (branching_scheme_args.empty())
            branching_scheme_args.push_back("forward");
        if (branching_scheme_args[0] == "forward") {
            auto parameters = read_permutationflowshopschedulingtt_forward_args(branching_scheme_argv);
            permutationflowshopschedulingtt::BranchingSchemeForward branching_scheme(instance, parameters);
            auto solution_pool = run(algorithm, branching_scheme, info);
            branching_scheme.write(solution_pool.best(), certificate_path);
            if (vm.count("print-solution"))
                branching_scheme.print(std::cout, solution_pool.best());
        } else {
            auto parameters = read_permutationflowshopschedulingtt_insertion_args(branching_scheme_argv);
            permutationflowshopschedulingtt::BranchingSchemeInsertion branching_scheme(instance, parameters);
            auto solution_pool = run(algorithm, branching_scheme, info);
            branching_scheme.write(solution_pool.best(), certificate_path);
            if (vm.count("print-solution"))
                branching_scheme.print(std::cout, solution_pool.best());
        }

    } else if (problem == "simpleassemblylinebalancing1") {
        simpleassemblylinebalancing1::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_simpleassemblylinebalancing1_args(branching_scheme_argv);
        simpleassemblylinebalancing1::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else if (problem == "ushapedassemblylinebalancing1") {
        ushapedassemblylinebalancing1::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;
        auto parameters = read_ushapedassemblylinebalancing1_args(branching_scheme_argv);
        ushapedassemblylinebalancing1::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
        if (vm.count("print-solution"))
            branching_scheme.print(std::cout, solution_pool.best());

    } else {
        std::cerr << "\033[31m" << "ERROR, unknown problem: '" << problem << "'.\033[0m" << std::endl;
        return 1;
    }

    return 0;
}

