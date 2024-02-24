cc_library(
        name = "knapsack_with_conflicts",
        hdrs = ["knapsack_with_conflicts.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:knapsack_with_conflicts",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "knapsack_with_conflicts_main",
        srcs = ["knapsack_with_conflicts_main.cpp"],
        deps = [
                ":knapsack_with_conflicts",
                "//treesearchsolver:read_args",
        ],
)


cc_library(
        name = "sequential_ordering",
        hdrs = ["sequential_ordering.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:sequential_ordering",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "sequential_ordering_main",
        srcs = ["sequential_ordering_main.cpp"],
        deps = [
                ":sequential_ordering",
                "//treesearchsolver:read_args",
        ],
)


cc_library(
        name = "permutation_flowshop_scheduling_makespan",
        hdrs = ["permutation_flowshop_scheduling_makespan.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:permutation_flowshop_scheduling_makespan",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "permutation_flowshop_scheduling_makespan_main",
        srcs = ["permutation_flowshop_scheduling_makespan_main.cpp"],
        deps = [
                ":permutation_flowshop_scheduling_makespan",
                "//treesearchsolver:read_args",
        ],
)


cc_library(
        name = "permutation_flowshop_scheduling_tct",
        hdrs = ["permutation_flowshop_scheduling_tct.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:permutation_flowshop_scheduling_tct",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "permutation_flowshop_scheduling_tct_main",
        srcs = ["permutation_flowshop_scheduling_tct_main.cpp"],
        deps = [
                ":permutation_flowshop_scheduling_tct",
                "//treesearchsolver:read_args",
        ],
)


cc_library(
        name = "simple_assembly_line_balancing_1",
        hdrs = ["simple_assembly_line_balancing_1.hpp"],
        deps = [
                "//treesearchsolver:treesearchsolver",
                "@orproblems//orproblems:simple_assembly_line_balancing_1",
        ],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "simple_assembly_line_balancing_1_main",
        srcs = ["simple_assembly_line_balancing_1_main.cpp"],
        deps = [
                ":simple_assembly_line_balancing_1",
                "//treesearchsolver:read_args",
        ],
)
