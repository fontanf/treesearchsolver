#include "examples/simpleassemblylinebalancing1.hpp"
#include "treesearchsolver/read_args.hpp"

using namespace treesearchsolver;
using namespace simpleassemblylinebalancing1;

int main(int argc, char *argv[])
{
    // Setup options.
    boost::program_options::options_description desc = setup_args();
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

    // Create instance.
    InstanceBuilder instance_builder;
    instance_builder.read(
            vm["input"].as<std::string>(),
            vm["format"].as<std::string>());
    Instance instance = instance_builder.build();

    // Create branching scheme.
    BranchingScheme branching_scheme(instance);

    // Run algorithm.
    std::string algorithm = vm["algorithm"].as<std::string>();
    Output<BranchingScheme> output =
        (algorithm == "greedy")?
        run_greedy(branching_scheme, vm):
        (algorithm == "best-first-search")?
        run_best_first_search(branching_scheme, vm):
        (algorithm == "iterative-beam-search")?
        run_iterative_beam_search(branching_scheme, vm):
        (algorithm == "anytime-column-search")?
        run_anytime_column_search(branching_scheme, vm):
        run_iterative_memory_bounded_best_first_search(branching_scheme, vm);

    // Run checker.
    if (vm["print-checker"].as<int>() > 0
            && vm["certificate"].as<std::string>() != "") {
        std::cout << std::endl
            << "Checker" << std::endl
            << "-------" << std::endl;
        instance.check(
                vm["certificate"].as<std::string>(),
                std::cout,
                vm["print-checker"].as<int>());
    }

    return 0;
}

