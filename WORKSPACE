load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest.git",
    commit = "58d77fa8070e8cec2dc1ed015d66b454c8d78850",
    shallow_since = "1656350095 -0400",
)

git_repository(
    name = "com_github_nelhage_rules_boost",
    remote = "https://github.com/nelhage/rules_boost",
    commit = "e83dfef18d91a3e35c8eac9b9aeb1444473c0efd",
    shallow_since = "1671181466 +0000",
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
    commit = "efa32d3af9885e22217345059e5442477a375311",
)

local_repository(
    name = "optimizationtools_",
    path = "../optimizationtools/",
)

git_repository(
    name = "orproblems",
    remote = "https://github.com/fontanf/orproblems.git",
    commit = "d7ca80aa3ade89ace994a7db8ce4f941e8075a99",
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

