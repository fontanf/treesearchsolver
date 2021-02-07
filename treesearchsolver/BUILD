cc_library(
        name = "treesearchsolver",
        hdrs = [
                "common.hpp",
                "algorithms/a_star.hpp",
                "algorithms/iterative_beam_search.hpp",
                "algorithms/iterative_memory_bounded_a_star.hpp",
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
                "treesearchsolver",
                "@boost//:program_options",
        ],
        visibility = ["//visibility:public"],
)
