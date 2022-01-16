load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest.git",
    commit = "703bd9caab50b139428cea1aaff9974ebee5742e",
    shallow_since = "1570114335 -0400",
)

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "9f9fb8b2f0213989247c9d5c0e814a8451d18d7f",
    remote = "https://github.com/nelhage/rules_boost",
    shallow_since = "1570056263 -0700",
)
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

http_archive(
    name = "json",
    build_file_content = """
cc_library(
        name = "json",
        hdrs = ["single_include/nlohmann/json.hpp"],
        visibility = ["//visibility:public"],
        strip_include_prefix = "single_include/"
)
""",
    urls = ["https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip"],
    sha256 = "87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014",
)

git_repository(
    name = "optimizationtools",
    remote = "https://github.com/fontanf/optimizationtools.git",
    commit = "c2df60575bed9af3cfb8652bc3c859f98b52b58c",
    shallow_since = "1630764843 +0200",
)

local_repository(
    name = "optimizationtools_",
    path = "../optimizationtools/",
)

git_repository(
    name = "orproblems",
    remote = "https://github.com/fontanf/orproblems.git",
    commit = "abcdd0f271bd86eefe1e6af00b052583e44721c0",
    shallow_since = "1642324849 +0100",
)

local_repository(
    name = "orproblems_",
    path = "../orproblems/",
)

http_archive(
    name = "interval-tree",
    build_file_content = """
cc_library(
        name = "interval-tree",
        hdrs = glob(["interval-tree-1.2/*.hpp"]),
        visibility = ["//visibility:public"],
        strip_include_prefix = "interval-tree-1.2/"
)
""",
    urls = ["https://github.com/5cript/interval-tree/archive/refs/tags/1.2.zip"],
    sha256 = "d72f917ae7d8f2540c4603bc9bf5386904ca3e40f09da0efcdb3671835b7326f"
)

