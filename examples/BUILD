cc_library(
        name = "knapsackwithconflicts",
        hdrs = ["knapsackwithconflicts.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:knapsackwithconflicts",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "travellingsalesman",
        hdrs = ["travellingsalesman.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:travellingsalesman",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "sequentialordering",
        hdrs = ["sequentialordering.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:sequentialordering",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "travellingrepairman",
        hdrs = ["travellingrepairman.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:travellingrepairman",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "thieforienteering",
        hdrs = ["thieforienteering.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:thieforienteering",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "schedulingwithsdsttwt",
        hdrs = ["schedulingwithsdsttwt.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:schedulingwithsdsttwt",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "orderacceptanceandscheduling",
        hdrs = ["orderacceptanceandscheduling.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:orderacceptanceandscheduling",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "batchschedulingtotalweightedtardiness",
        hdrs = ["batchschedulingtotalweightedtardiness.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:batchschedulingtotalweightedtardiness",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "permutationflowshopschedulingmakespan",
        hdrs = ["permutationflowshopschedulingmakespan.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:permutationflowshopschedulingmakespan",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "permutationflowshopschedulingtct",
        hdrs = ["permutationflowshopschedulingtct.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:permutationflowshopschedulingtct",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "permutationflowshopschedulingtt",
        hdrs = ["permutationflowshopschedulingtt.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:permutationflowshopschedulingtt",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "simpleassemblylinebalancing1",
        hdrs = ["simpleassemblylinebalancing1.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:simpleassemblylinebalancing1",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "ushapedassemblylinebalancing1",
        hdrs = ["ushapedassemblylinebalancing1.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:ushapedassemblylinebalancing1",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["read_args.hpp", "main.cpp"],
        deps = [
                ":knapsackwithconflicts",
                ":travellingsalesman",
                ":sequentialordering",
                ":travellingrepairman",
                ":thieforienteering",
                ":schedulingwithsdsttwt",
                ":orderacceptanceandscheduling",
                ":batchschedulingtotalweightedtardiness",
                ":permutationflowshopschedulingmakespan",
                ":permutationflowshopschedulingtct",
                ":permutationflowshopschedulingtt",
                ":simpleassemblylinebalancing1",
                ":ushapedassemblylinebalancing1",
                "//treesearchsolver:read_args",
                "@boost//:program_options",
        ],
)

