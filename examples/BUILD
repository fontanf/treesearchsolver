cc_library(
        name = "travellingsalesman",
        hdrs = ["travellingsalesman.hpp"],
        deps = ["//treesearchsolver:treesearchsolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "sequentialordering",
        hdrs = ["sequentialordering.hpp"],
        deps = ["//treesearchsolver:treesearchsolver"],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["main.cpp"],
        deps = [
                ":travellingsalesman",
                ":sequentialordering",
                "//treesearchsolver:read_args",
                "@boost//:program_options",
        ],
        data = ["//data:data"],
)

