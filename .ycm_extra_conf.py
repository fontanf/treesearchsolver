def Settings(**kwargs):
    return {
            'flags': [
                '-x', 'c++',
                '-Wall', '-Wextra', '-Werror',
                '-I', '.',
                '-I', './bazel-treesearchsolver/external/'
                'json/single_include/',
                '-I', './bazel-treesearchsolver/external/'
                'interval-tree/interval-tree-1.2/',
                '-I', './bazel-treesearchsolver/external/'
                'googletest/googletest/include/',
                '-I', './bazel-treesearchsolver/external/boost/',
                '-I', './bazel-treesearchsolver/external/optimizationtools/',

                # Optimization tools
                # '-I', '../'
                '-I', './bazel-treesearchsolver/external/'
                'optimizationtools/',

                # OR Problems
                # '-I', '../'
                '-I', './bazel-treesearchsolver/external/'
                'orproblems/',
                ],
            }
