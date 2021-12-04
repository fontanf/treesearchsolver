#include "examples/travellingsalesman.hpp"
#include "treesearchsolver/read_args.hpp"

using namespace treesearchsolver;
using namespace travellingsalesman;

inline BranchingSchemeForward::Parameters read_branching_scheme_forward_args(
        const std::vector<char*> argv)
{
    BranchingSchemeForward::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("bound,b", boost::program_options::value<GuideId>(&parameters.bound_id), "")
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

inline BranchingSchemeInsertion::Parameters read_branching_scheme_insertion_args(
        const std::vector<char*> argv)
{
    BranchingSchemeInsertion::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
        ("sort,s", boost::program_options::value<GuideId>(&parameters.sort_criterion_id), "")
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

    if (strcmp(main_args.branching_scheme_argv[0], "forward") == 0) {

        // Create branching scheme.
        auto parameters = read_branching_scheme_forward_args(main_args.branching_scheme_argv);
        BranchingSchemeForward branching_scheme(instance, parameters);

        // Run algorithm.
        auto solution_pool =
            (strcmp(main_args.algorithm_argv[0], "best_first_search") == 0)?
            run_best_first_search(main_args.algorithm_argv, branching_scheme, main_args.info):
            (strcmp(main_args.algorithm_argv[0], "iterative_beam_search") == 0)?
            run_iterative_beam_search(main_args.algorithm_argv, branching_scheme, main_args.info):
            run_iterative_memory_bounded_best_first_search(main_args.algorithm_argv, branching_scheme, main_args.info);

        // Write solution.
        branching_scheme.write(solution_pool.best(), main_args.info.output->certificate_path);
        if (main_args.print_solution)
            branching_scheme.print(std::cout, solution_pool.best());

    } else {

        // Create branching scheme.
        auto parameters = read_branching_scheme_insertion_args(main_args.branching_scheme_argv);
        BranchingSchemeInsertion branching_scheme(instance, parameters);

        // Run algorithm.
        auto solution_pool =
            (strcmp(main_args.algorithm_argv[0], "best_first_search") == 0)?
            run_best_first_search(main_args.algorithm_argv, branching_scheme, main_args.info):
            (strcmp(main_args.algorithm_argv[0], "iterative_beam_search") == 0)?
            run_iterative_beam_search(main_args.algorithm_argv, branching_scheme, main_args.info):
            run_iterative_memory_bounded_best_first_search(main_args.algorithm_argv, branching_scheme, main_args.info);

        // Write solution.
        branching_scheme.write(solution_pool.best(), main_args.info.output->certificate_path);
        if (main_args.print_solution)
            branching_scheme.print(std::cout, solution_pool.best());

    }

    // Run checker.
    if (main_args.info.output->certificate_path != "") {
        std::cout << std::endl;
        instance.check(main_args.info.output->certificate_path);
    }

    return 0;
}

