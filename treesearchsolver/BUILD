cc_library(
        name = "treesearchsolver",
        hdrs = [
                "common.hpp",
                "algorithm_formatter.hpp",
                "greedy.hpp",
                "depth_first_search.hpp",
                "best_first_search.hpp",
                "iterative_beam_search.hpp",
                "iterative_beam_search_2.hpp",
                "iterative_memory_bounded_best_first_search.hpp",
                "anytime_column_search.hpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools/utils:utils",
                "@optimizationtools//optimizationtools/containers:containers",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "read_args",
        hdrs = ["read_args.hpp"],
        deps = [
                ":treesearchsolver",
                "@boost//:program_options",
        ],
        visibility = ["//visibility:public"],
)
