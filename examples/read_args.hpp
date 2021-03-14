#pragma once

#include "examples/travellingsalesman.hpp"
#include "examples/sequentialordering.hpp"
#include "examples/orderacceptanceandscheduling.hpp"
#include "examples/thieforienteering.hpp"
#include "examples/batchschedulingtotalweightedtardiness.hpp"
#include "examples/simpleassemblylinebalancing1.hpp"

#include <boost/program_options.hpp>

namespace treesearchsolver
{

typedef int64_t GuideId;
typedef int64_t Counter;

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

inline orderacceptanceandscheduling::BranchingScheme::Parameters read_orderacceptanceandscheduling_args(
        const std::vector<char*> argv)
{
    orderacceptanceandscheduling::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        //("bound,b", boost::program_options::value<GuideId>(&parameters.bound_id), "")
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

inline simpleassemblylinebalancing1::BranchingScheme::Parameters read_simpleassemblylinebalancing1_args(
        const std::vector<char*> argv)
{
    simpleassemblylinebalancing1::BranchingScheme::Parameters parameters;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        //("bound,b", boost::program_options::value<GuideId>(&parameters.bound_id), "")
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
