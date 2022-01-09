#include "examples/simpleassemblylinebalancing1.hpp"
#include "treesearchsolver/read_args.hpp"

using namespace treesearchsolver;
using namespace simpleassemblylinebalancing1;

inline BranchingScheme::Parameters read_branching_scheme_args(
        const std::vector<char*> argv)
{
    BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        //("guide,g", boost::program_options::value<GuideId>(&parameters.guide), "")
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

int main(int argc, char *argv[])
{
    auto main_args = read_args(argc, argv);

    // Create instance.
    Instance instance(main_args.instance_path, main_args.format);
    if (main_args.print_instance)
        std::cout << instance << std::endl;

    // Create branching scheme.
    auto parameters = read_branching_scheme_args(main_args.branching_scheme_argv);
    BranchingScheme branching_scheme(instance, parameters);

    // Run algorithm.
    auto solution_pool =
        (strcmp(main_args.algorithm_argv[0], "greedy") == 0)?
        run_greedy(main_args, branching_scheme, main_args.info):
        (strcmp(main_args.algorithm_argv[0], "best_first_search") == 0)?
        run_best_first_search(main_args, branching_scheme, main_args.info):
        (strcmp(main_args.algorithm_argv[0], "iterative_beam_search") == 0)?
        run_iterative_beam_search(main_args, branching_scheme, main_args.info):
        run_iterative_memory_bounded_best_first_search(main_args, branching_scheme, main_args.info);

    // Write solution.
    branching_scheme.write(solution_pool.best(), main_args.info.output->certificate_path);
    if (main_args.print_solution)
        branching_scheme.print(std::cout, solution_pool.best());

    // Run checker.
    if (main_args.info.output->certificate_path != "") {
        std::cout << std::endl;
        instance.check(main_args.info.output->certificate_path);
    }

    return 0;
}

