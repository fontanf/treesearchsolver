#pragma once

#include "treesearchsolver/common.hpp"

#include <sstream>
#include <iomanip>

namespace treesearchsolver
{

template <typename BranchingScheme>
class AlgorithmFormatter
{

    using Node = typename BranchingScheme::Node;

public:

    /** Constructor. */
    AlgorithmFormatter(
            const BranchingScheme& branching_scheme,
            const Parameters<BranchingScheme>& parameters,
            Output<BranchingScheme>& output):
        branching_scheme_(branching_scheme),
        parameters_(parameters),
        output_(output),
        os_(parameters.create_os()) { }

    /** Print the header. */
    void start(
            const std::string& algorithm_name);

    /** Print the header. */
    void print_header();

    /** Print current state. */
    void print(
            const std::stringstream& s);

    /** Update the solution. */
    void update_solution(
            const std::shared_ptr<Node>& node);

    /** Method to call at the end of the algorithm. */
    void end();

private:

    /*
     * Private methods
     */

    /*
     * instance_format
     */

    template<typename, typename T>
    struct HasInstanceFormatMethod
    {
        static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");
    };

    template<typename C, typename Ret, typename... Args>
    struct HasInstanceFormatMethod<C, Ret(Args...)>
    {

    private:

        template<typename T>
        static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().instance_format(std::declval<Args>()...)), Ret>::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

    public:

        static constexpr bool value = type::value;

    };

    void instance_format(
            std::ostream&,
            int,
            std::false_type)
    {
    }

    void instance_format(
            std::ostream& os,
            int verbosity_level,
            std::true_type)
    {
        os << std::endl
            << "Instance" << std::endl
            << "--------" << std::endl
            ;
        return branching_scheme_.instance_format(
                os,
                verbosity_level);
    }

    void instance_format(
            std::ostream& os,
            int verbosity_level)
    {
        return instance_format(
                os,
                verbosity_level,
                std::integral_constant<
                    bool,
                    HasInstanceFormatMethod<BranchingScheme,
                    void(std::ostream&, int)>::value>());
    }

    /*
     * parameters_format
     */

    template<typename, typename T>
    struct HasParametersFormatMethod
    {
        static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");
    };

    template<typename C, typename Ret, typename... Args>
    struct HasParametersFormatMethod<C, Ret(Args...)>
    {

    private:

        template<typename T>
        static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().parameters_format(std::declval<Args>()...)), Ret>::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

    public:

        static constexpr bool value = type::value;

    };

    void parameters_format(
            std::ostream&,
            std::false_type)
    {
    }

    void parameters_format(
            std::ostream& os,
            std::true_type)
    {
        os << std::endl
            << "Branching scheme parameters" << std::endl
            << "---------------------------" << std::endl
            ;
        return branching_scheme_.parameters_format(os);
    }

    void parameters_format(
            std::ostream& os)
    {
        return parameters_format(
                os,
                std::integral_constant<
                    bool,
                    HasParametersFormatMethod<BranchingScheme,
                    void(std::ostream&)>::value>());
    }

    /*
     * solution_format
     */

    template<typename, typename T>
    struct HasSolutionFormatMethod
    {
        static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");
    };

    template<typename C, typename Ret, typename... Args>
    struct HasSolutionFormatMethod<C, Ret(Args...)>
    {

    private:

        template<typename T>
        static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().solution_format(std::declval<Args>()...)), Ret>::type;

        template<typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

    public:

        static constexpr bool value = type::value;

    };

    void solution_format(
            const std::shared_ptr<typename BranchingScheme::Node>&,
            std::ostream&,
            int,
            std::false_type)
    {
    }

    void solution_format(
            const std::shared_ptr<typename BranchingScheme::Node>& solution,
            std::ostream& os,
            int verbosity_level,
            std::true_type)
    {
        os << std::endl
            << "Solution" << std::endl
            << "--------" << std::endl
            ;
        return branching_scheme_.solution_format(
                solution,
                os,
                verbosity_level);
    }

    void solution_format(
            const std::shared_ptr<typename BranchingScheme::Node>& solution,
            std::ostream& os,
            int verbosity_level)
    {
        return solution_format(
                solution,
                os,
                verbosity_level,
                std::integral_constant<
                    bool,
                    HasSolutionFormatMethod<BranchingScheme,
                    void(const std::shared_ptr<typename BranchingScheme::Node>&, std::ostream&, int)>::value>());
    }

    /*
     * Private attributes
     */

    /** Branching scheme. */
    const BranchingScheme& branching_scheme_;

    /** Parameters. */
    const Parameters<BranchingScheme>& parameters_;

    /** Output. */
    Output<BranchingScheme>& output_;

    /** Output stream. */
    std::unique_ptr<optimizationtools::ComposeStream> os_;

};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Templates implementation ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename BranchingScheme>
void AlgorithmFormatter<BranchingScheme>::start(
        const std::string& algorithm_name)
{
    output_.json["Parameters"] = parameters_.to_json();

    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << "======================================" << std::endl
        << "           TreeSearchSolver           " << std::endl
        << "======================================" << std::endl;
    instance_format(*os_, parameters_.verbosity_level);
    parameters_format(*os_);
    *os_
        << std::endl
        << "Algorithm" << std::endl
        << "---------" << std::endl
        << algorithm_name << std::endl
        << std::endl
        << "Parameters" << std::endl
        << "----------" << std::endl;
    parameters_.format(*os_);
}

template <typename BranchingScheme>
void AlgorithmFormatter<BranchingScheme>::print_header()
{
    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << std::endl
        << std::right
        << std::setw(11) << "Time"
        << std::setw(32) << "Value"
        << std::setw(32) << "Comment"
        << std::endl
        << std::setw(11) << "----"
        << std::setw(32) << "-----"
        << std::setw(32) << "-------"
        << std::endl;
}

template <typename BranchingScheme>
void AlgorithmFormatter<BranchingScheme>::print(
        const std::stringstream& s)
{
    output_.time = parameters_.timer.elapsed_time();
    if (parameters_.verbosity_level == 0)
        return;
    std::streamsize precision = std::cout.precision();
    *os_
        << std::setw(11) << std::fixed << std::setprecision(3) << output_.time << std::defaultfloat << std::setprecision(precision)
        << std::setw(32) << branching_scheme_.display(output_.solution_pool.best())
        << std::setw(32) << s.str()
        << std::endl;
}

template <typename BranchingScheme>
void AlgorithmFormatter<BranchingScheme>::update_solution(
        const std::shared_ptr<Node>& node)
{
    if (output_.solution_pool.add(node) == 2) {
        output_.json["IntermediaryOutputs"].push_back(output_.to_json());
        parameters_.new_solution_callback(output_);
    }
}

template <typename BranchingScheme>
void AlgorithmFormatter<BranchingScheme>::end()
{
    output_.time = parameters_.timer.elapsed_time();
    output_.json["Output"] = output_.to_json();

    if (parameters_.verbosity_level == 0)
        return;
    *os_
        << std::endl
        << "Final statistics" << std::endl
        << "----------------" << std::endl;
    output_.format(*os_);
    solution_format(
            output_.solution_pool.best(),
            *os_,
            parameters_.verbosity_level);
}

}
