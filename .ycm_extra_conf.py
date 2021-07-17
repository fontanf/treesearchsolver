def Settings(**kwargs):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-I', '.',
                '-I', './bazel-treesearchsolver/external/json/single_include/',
                '-I', './bazel-treesearchsolver/external/googletest/googletest/include/',
                '-I', './bazel-treesearchsolver/external/boost/',
                '-I', './bazel-treesearchsolver/external/optimizationtools/',
                # '-I', './../optimizationtools/',
                '-I', './bazel-treesearchsolver/external/orproblems/',
                # '-I', './../orproblems/',
                ],
            }
