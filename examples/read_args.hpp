#pragma once

#include "examples/knapsackwithconflicts.hpp"
#include "examples/travellingsalesman.hpp"
#include "examples/sequentialordering.hpp"
#include "examples/thieforienteering.hpp"
#include "examples/schedulingwithsdsttwt.hpp"
#include "examples/orderacceptanceandscheduling.hpp"
#include "examples/batchschedulingtotalweightedtardiness.hpp"
#include "examples/permutationflowshopschedulingmakespan.hpp"
#include "examples/permutationflowshopschedulingtct.hpp"
#include "examples/permutationflowshopschedulingtt.hpp"
#include "examples/simpleassemblylinebalancing1.hpp"
#include "examples/ushapedassemblylinebalancing1.hpp"

#include <boost/program_options.hpp>

namespace treesearchsolver
{

typedef int64_t GuideId;
typedef int64_t Counter;

inline knapsackwithconflicts::BranchingScheme::Parameters read_knapsackwithconflicts_args(
        const std::vector<char*> argv)
{
    knapsackwithconflicts::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("force-order,f", boost::program_options::value<bool>(&parameters.force_order), "")
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

inline travellingsalesman::BranchingScheme::Parameters read_travellingsalesman_args(
        const std::vector<char*> argv)
{
    travellingsalesman::BranchingScheme::Parameters parameters;
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

inline sequentialordering::BranchingScheme::Parameters read_sequentialordering_args(
        const std::vector<char*> argv)
{
    sequentialordering::BranchingScheme::Parameters parameters;
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

inline schedulingwithsdsttwt::BranchingScheme::Parameters read_schedulingwithsdsttwt_args(
        const std::vector<char*> argv)
{
    schedulingwithsdsttwt::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
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

inline orderacceptanceandscheduling::BranchingScheme::Parameters read_orderacceptanceandscheduling_args(
        const std::vector<char*> argv)
{
    orderacceptanceandscheduling::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        //("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
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

inline batchschedulingtotalweightedtardiness::BranchingScheme::Parameters read_batchschedulingtotalweightedtardiness_args(
        const std::vector<char*> argv)
{
    batchschedulingtotalweightedtardiness::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        //("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
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

inline permutationflowshopschedulingmakespan::BranchingScheme::Parameters read_permutationflowshopschedulingmakespan_args(
        const std::vector<char*> argv)
{
    permutationflowshopschedulingmakespan::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("bidirectional,b", boost::program_options::value<bool>(&parameters.bidirectional), "")
        ("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
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

inline permutationflowshopschedulingtct::BranchingScheme::Parameters read_permutationflowshopschedulingtct_args(
        const std::vector<char*> argv)
{
    permutationflowshopschedulingtct::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
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

inline permutationflowshopschedulingtt::BranchingScheme::Parameters read_permutationflowshopschedulingtt_args(
        const std::vector<char*> argv)
{
    permutationflowshopschedulingtt::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("guide,g", boost::program_options::value<GuideId>(&parameters.guide_id), "")
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

inline simpleassemblylinebalancing1::BranchingScheme::Parameters read_simpleassemblylinebalancing1_args(
        const std::vector<char*> argv)
{
    simpleassemblylinebalancing1::BranchingScheme::Parameters parameters;
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

inline ushapedassemblylinebalancing1::BranchingScheme::Parameters read_ushapedassemblylinebalancing1_args(
        const std::vector<char*> argv)
{
    ushapedassemblylinebalancing1::BranchingScheme::Parameters parameters;
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

}
