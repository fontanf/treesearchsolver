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

cc_library(
        name = "thieforienteering",
        hdrs = ["thieforienteering.hpp"],
        deps = ["//treesearchsolver:treesearchsolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "orderacceptanceandscheduling",
        hdrs = ["orderacceptanceandscheduling.hpp"],
        deps = ["//treesearchsolver:treesearchsolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "batchschedulingtotalweightedtardiness",
        hdrs = ["batchschedulingtotalweightedtardiness.hpp"],
        deps = ["//treesearchsolver:treesearchsolver"],
        visibility = ["//visibility:public"],
)

cc_binary(
        name = "main",
        srcs = ["read_args.hpp", "main.cpp"],
        deps = [
                ":travellingsalesman",
                ":sequentialordering",
                ":thieforienteering",
                ":orderacceptanceandscheduling",
                ":batchschedulingtotalweightedtardiness",
                "//treesearchsolver:read_args",
                "@boost//:program_options",
        ],
        data = ["//data:data"],
)

cc_binary(
        name = "checker",
        srcs = ["checker.cpp"],
        deps = [
                ":travellingsalesman",
                ":sequentialordering",
                ":thieforienteering",
                ":orderacceptanceandscheduling",
                ":batchschedulingtotalweightedtardiness",
                "//treesearchsolver:read_args",
                "@boost//:program_options",
        ],
        data = ["//data:data"],
)

