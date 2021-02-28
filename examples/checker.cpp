#include "examples/travellingsalesman.hpp"
#include "examples/sequentialordering.hpp"
#include "examples/thieforienteering.hpp"
#include "examples/orderacceptanceandscheduling.hpp"
#include "examples/batchschedulingtotalweightedtardiness.hpp"

#include <boost/program_options.hpp>

using namespace treesearchsolver;

namespace po = boost::program_options;

int main(int argc, char *argv[])
{

    // Parse program options

    std::string problem = "travellingsalesman";
    std::string instance_path = "";
    std::string format = "";
    std::string certificate_path = "";

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("problem,p", po::value<std::string>(&problem)->required(), "set problem (required)")
        ("input,i", po::value<std::string>(&instance_path)->required(), "set input path (required)")
        ("certificate,c", po::value<std::string>(&certificate_path), "set certificate path")
        ("format,f", po::value<std::string>(&format), "set input file format (default: orlibrary)")
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

    std::ifstream file(certificate_path);
    if (!file.good()) {
        std::cerr << "\033[31m" << "ERROR, unable to open file \"" << certificate_path << "\"" << "\033[0m" << std::endl;
        return 1;
    }

    if (problem == "travellingsalesman") {
        travellingsalesman::Instance instance(instance_path, format);
    } else if (problem == "sequentialordering") {
        sequentialordering::Instance instance(instance_path, format);
    } else if (problem == "thieforienteering") {
        thieforienteering::Instance instance(instance_path, format);
        thieforienteering::Time t = 0;
        thieforienteering::Profit p = 0;
        thieforienteering::Weight w = 0;
        thieforienteering::VertexId j = 0;
        thieforienteering::ItemId i = -1;
        while (file >> i) {
            //i--;
            thieforienteering::VertexId j_next = instance.item(i).location;
            t += instance.duration(j, j_next, w);
            p += instance.item(i).profit;
            w += instance.item(i).weight;
            std::cout << "Item: " << i
                << "; Vertex: " << j_next
                << "; Duration: " << t << " / " << instance.time_limit()
                << "; Weight: " << w << " / " << instance.capacity()
                << "; Profit: " << p << std::endl;
            j = j_next;
        }
        t += instance.duration(j, instance.vertex_number() - 1, w);
        std::cout << "---" << std::endl;
        std::cout << "Duration:  " << t << " / " << instance.time_limit() << std::endl;
        std::cout << "Weight:    " << w << " / " << instance.capacity() << std::endl;
        std::cout << "Profit:    " << p << std::endl;
    } else if (problem == "orderacceptanceandscheduling") {
        orderacceptanceandscheduling::Instance instance(instance_path, format);
    } else if (problem == "batchschedulingtotalweightedtardiness") {
        batchschedulingtotalweightedtardiness::Instance instance(instance_path, format);
    } else {
        std::cerr << "\033[31m" << "ERROR, unknown problem: '" << problem << "'.\033[0m" << std::endl;
        return 1;
    }

    return 0;
}

