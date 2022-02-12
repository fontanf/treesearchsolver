cc_library(
        name = "treesearchsolver",
        hdrs = [
                "common.hpp",
                "greedy.hpp",
                "best_first_search.hpp",
                "iterative_beam_search.hpp",
                "iterative_memory_bounded_best_first_search.hpp",
                "anytime_column_search.hpp",
        ],
        deps = [
                "@optimizationtools//optimizationtools:info",
                "@optimizationtools//optimizationtools:utils",
                "@optimizationtools//optimizationtools:indexed_collections",
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
