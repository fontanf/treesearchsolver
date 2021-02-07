# Tree Search Solver

A solver based on heuristic tree search.

![treesearch](img/treesearch.jpg?raw=true "treesearch")

[image source](https://commons.wikimedia.org/wiki/File:Saint-L%C3%A9ger-l%C3%A8s-Domart,arbre_de_la_croix_Notre-Dame_14.jpg)

## Description

The goal of this repository is to provide a simple framework to quickly implement algorithms based on heuristic tree search.

Algorithms:
* A\* `astar`
* Iterative Memory Bounded A\* `iterativememoryboundedastar`
* Iterative Beam Search `iterativebeamsearch`

## Examples

[Travelling Salesman Problem](examples/travellingsalesman.hpp)

[Sequential Ordering Problem](examples/sequentialordering.hpp)
* Literature:
  * "Tree search algorithms for the Sequential Ordering Problem" (Libralesso et al., 2019)
* State-of-the-art on the `soplib` dataset (same algorithm as `libralesso2019`)

## Usage, running examples from command line

Compile:
```shell
bazel build -- //...
```

Then, examples can be executed as follows:
```shell
./bazel-bin/examples/main -v -p travellingsalesman -a iterativebeamsearch -i data/travellingsalesman/tsplib/a280.tsp
```

## Usage, C++ library

See examples.

## Benchmarks

```
python3 ../optimizationtools/optimizationtools/bench_run.py --csv data/travellingsalesman/data.csv -l travellingsalesman --main "./bazel-bin/examples/main -p travellingsalesman" -a "iterativebeamsearch" -f "'pla85900.tsp' not in row['Path']" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv data/travellingsalesman/data.csv -l travellingsalesman -f "'pla85900.tsp' not in row['Path']" -t 62
python3 ../optimizationtools/optimizationtools/bench_run.py --csv data/sequentialordering/data.csv -l sequentialordering --main "./bazel-bin/examples/main -p sequentialordering" -a "iterativebeamsearch" -f "row['Dataset'] == 'soplib'" -t 600
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv data/sequentialordering/data.csv -l sequentialordering -f "row['Dataset'] == 'soplib'" -t 602
```

