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
        //branching_scheme.write(solution_pool.best(), certificate_path);

        if (vm.count("print-solution")) {
            for (auto node_tmp = solution_pool.best(); node_tmp->father != nullptr; node_tmp = node_tmp->father)
                std::cout << "j " << node_tmp->j
                    << " wj " << instance.item(node_tmp->j).weight
                    << " pj " << instance.item(node_tmp->j).profit
                    << std::endl;
        }
    } else if (problem == "travellingsalesman") {
        travellingsalesman::Instance instance(instance_path, format);
        auto parameters = read_travellingsalesman_args(branching_scheme_argv);
        travellingsalesman::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
    } else if (problem == "sequentialordering") {
        sequentialordering::Instance instance(instance_path, format);
        auto parameters = read_sequentialordering_args(branching_scheme_argv);
        sequentialordering::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
    } else if (problem == "thieforienteering") {
        thieforienteering::Instance instance(instance_path, format);
        auto parameters = read_thieforienteering_args(branching_scheme_argv);
        thieforienteering::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);
    } else if (problem == "orderacceptanceandscheduling") {
        orderacceptanceandscheduling::Instance instance(instance_path, format);
        auto parameters = read_orderacceptanceandscheduling_args(branching_scheme_argv);
        orderacceptanceandscheduling::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);

        if (vm.count("print-solution")) {
            for (auto node_tmp = solution_pool.best(); node_tmp->father != nullptr;
                    node_tmp = node_tmp->father)
                std::cout << "node_tmp"
                    << " n " << node_tmp->job_number
                    << " t " << node_tmp->time
                    << " p " << node_tmp->profit
                    << " w " << node_tmp->weight
                    << " j " << node_tmp->j
                    << " rj " << instance.job(node_tmp->j).release_date
                    << " dj " << instance.job(node_tmp->j).due_date
                    << " dj " << instance.job(node_tmp->j).deadline
                    << " pj " << instance.job(node_tmp->j).processing_time
                    << " vj " << instance.job(node_tmp->j).profit
                    << " wj " << instance.job(node_tmp->j).weight
                    << " sij " << instance.setup_time(node_tmp->father->j, node_tmp->j)
                    << std::endl;
        }
    } else if (problem == "batchschedulingtotalweightedtardiness") {
        batchschedulingtotalweightedtardiness::Instance instance(instance_path, format);
        batchschedulingtotalweightedtardiness::BranchingScheme branching_scheme(instance);
        auto solution_pool = run(algorithm, branching_scheme, info);
        //branching_scheme.write(solution_pool.best(), certificate_path);

        if (vm.count("print-solution")) {
            batchschedulingtotalweightedtardiness::Time current_batch_end
                = solution_pool.best()->current_batch_end;
            for (auto node_tmp = solution_pool.best(); node_tmp->father != nullptr;
                    node_tmp = node_tmp->father) {
                batchschedulingtotalweightedtardiness::Weight wtj
                    = (current_batch_end <= instance.job(node_tmp->j).due_date)? 0:
                    instance.job(node_tmp->j).weight * (current_batch_end - instance.job(node_tmp->j).due_date);
                std::cout << "node_tmp"
                    << " n " << node_tmp->job_number
                    << " bs " << node_tmp->current_batch_start
                    << " be " << node_tmp->current_batch_end
                    << " rbe " << current_batch_end
                    << " s " << node_tmp->current_batch_size
                    << " twt " << node_tmp->total_weighted_tardiness
                    << " bnd " << node_tmp->bound
                    << " j " << node_tmp->j
                    << " nb " << node_tmp->new_batch
                    << " pj " << instance.job(node_tmp->j).processing_time
                    << " sj " << instance.job(node_tmp->j).size
                    << " rj " << instance.job(node_tmp->j).release_date
                    << " dj " << instance.job(node_tmp->j).due_date
                    << " wj " << instance.job(node_tmp->j).weight
                    << " wtj " << wtj
                    << std::endl;
                if (node_tmp->new_batch)
                    current_batch_end = node_tmp->father->current_batch_end;
            }
        }
    } else if (problem == "permutationflowshopschedulingtct") {
        permutationflowshopschedulingtct::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;

        auto parameters = read_permutationflowshopschedulingtct_args(branching_scheme_argv);
        permutationflowshopschedulingtct::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        branching_scheme.write(solution_pool.best(), certificate_path);

        if (vm.count("print-solution")) {
            for (auto node_tmp = solution_pool.best(); node_tmp->father != nullptr;
                    node_tmp = node_tmp->father) {
                std::cout << "node_tmp"
                    << " n " << node_tmp->job_number
                    << " c " << node_tmp->times[instance.machine_number() - 1]
                    << " tct " << node_tmp->total_completion_time
                    << " bnd " << node_tmp->bound
                    << " j " << node_tmp->j
                    << std::endl;
            }
        }
    } else if (problem == "simpleassemblylinebalancing1") {
        simpleassemblylinebalancing1::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;

        auto parameters = read_simpleassemblylinebalancing1_args(branching_scheme_argv);
        simpleassemblylinebalancing1::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        //branching_scheme.write(solution_pool.best(), certificate_path);

        if (vm.count("print-solution")) {
            simpleassemblylinebalancing1::StationId m = solution_pool.best()->station_number;
            std::vector<std::vector<simpleassemblylinebalancing1::JobId>> stations(m);
            std::vector<simpleassemblylinebalancing1::Time> times(m, 0);
            for (auto node_tmp = solution_pool.best(); node_tmp->father != nullptr; node_tmp = node_tmp->father) {
                stations[node_tmp->station_number - 1].push_back(node_tmp->j);
                times[node_tmp->station_number - 1] += instance.job(node_tmp->j).processing_time;
            }
            for (simpleassemblylinebalancing1::StationId i = 0; i < m; ++i) {
                std::cout << "Station " << i << " " << times[i] << "/" << instance.cycle_time() << ":";
                std::reverse(stations[i].begin(), stations[i].end());
                for (simpleassemblylinebalancing1::JobId j: stations[i])
                    std::cout << " " << j;
                std::cout << std::endl;
            }
        }
    } else if (problem == "ushapedassemblylinebalancing1") {
        ushapedassemblylinebalancing1::Instance instance(instance_path, format);
        if (vm.count("print-instance"))
            std::cout << instance << std::endl;

        auto parameters = read_ushapedassemblylinebalancing1_args(branching_scheme_argv);
        ushapedassemblylinebalancing1::BranchingScheme branching_scheme(instance, parameters);
        auto solution_pool = run(algorithm, branching_scheme, info);
        //branching_scheme.write(solution_pool.best(), certificate_path);

        if (vm.count("print-solution")) {
            ushapedassemblylinebalancing1::StationId m = solution_pool.best()->station_number;
            std::vector<std::vector<ushapedassemblylinebalancing1::JobId>> stations(m);
            std::vector<ushapedassemblylinebalancing1::Time> times(m, 0);
            for (auto node_tmp = solution_pool.best(); node_tmp->father != nullptr; node_tmp = node_tmp->father) {
                stations[node_tmp->station_number - 1].push_back(node_tmp->j);
                times[node_tmp->station_number - 1] += instance.job(node_tmp->j).processing_time;
            }
            for (ushapedassemblylinebalancing1::StationId i = 0; i < m; ++i) {
                std::cout << "Station " << i << " " << times[i] << "/" << instance.cycle_time() << ":";
                std::reverse(stations[i].begin(), stations[i].end());
                for (ushapedassemblylinebalancing1::JobId j: stations[i])
                    std::cout << " " << j;
                std::cout << std::endl;
            }
        }
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown problem: '" << problem << "'.\033[0m" << std::endl;
        return 1;
    }

    return 0;
}

