# Tree Search Solver

A solver based on heuristic tree search.

![treesearch](img/treesearch.jpg?raw=true "treesearch")

[image source](https://commons.wikimedia.org/wiki/File:Saint-L%C3%A9ger-l%C3%A8s-Domart,arbre_de_la_croix_Notre-Dame_14.jpg)

## Description

The goal of this repository is to provide a simple framework to quickly implement algorithms based on heuristic tree search.

Solving a problem only requires a couple hundred lines of code (see examples).

Algorithms:
* A\* `astar`
* Iterative Memory Bounded A\* `iterativememoryboundedastar`
* Iterative Beam Search `iterativebeamsearch`

## Examples

[Travelling Salesman Problem](examples/travellingsalesman.hpp)

[Sequential Ordering Problem](examples/sequentialordering.hpp)
* Literature:
  * "A hybrid particle swarm optimization approach for the sequential ordering problem" (Anghinolfi et al., 2011)
  * "A bee colony optimization with automated parameter tuning for sequential ordering problem" (Wun et al., 2014)
  * "An improved Ant Colony System for the Sequential Ordering Problem (Skinderowicz, 2017)
  * "An extension of the Lin-Kernighan-Helsgaun TSP solver for constrained traveling salesman and vehicle routing problems" (Helsgaun, 2017)
  * "Tree search algorithms for the Sequential Ordering Problem" (Libralesso et al., 2019)
* Notes:
  * State-of-the-art on the `soplib` dataset (same algorithm as `libralesso2019`)

[Thief Orienteering Problem](examples/thieforienteering.hpp)
* Description
  * Determine a subset of items to collect
  * Maximize the profit of collected items
  * Each item is in a location, a location may contain multiple items
  * Each collected item decreases the speed of the traveller
  * Time limit and capacity constraint
* Literature
  * "The Thief Orienteering Problem: Formulation and Heuristic Approaches" (Santos et Chagas, 2018)
  * "Ants can orienteer a thief in their robbery" (Chagas et Wagner, 2020)

[Single machine batch scheduling problem, Total weighted tardiness](examples/batchschedulingtotalweightedtardiness.hpp)
* Three field classification: `1 | batch, rⱼ, sⱼ, compt | ∑wⱼTⱼ`
* Literature
  * "Scheduling a batch processing machine to minimize total weighted tardiness" (Chou et Wang, 2008)
  * "Minimizing total weighted tardiness on a batch-processing machine with non-agreeable release times and due dates" (Mathirajan et al., 2010)
  * "Solving single batch-processing machine problems using an iterated heuristic" (Wang, 2011)
  * "Iterated local search for single machine total weighted tardiness batch scheduling" (Queiroga et al., 2020)

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
python3 ../optimizationtools/optimizationtools/bench_run.py --csv data/thieforienteering/data.csv -l thieforienteering --main "./bazel-bin/examples/main -p thieforienteering" -a "iterativememoryboundedastar" --timelimitfield "Time limit"
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv data/thieforienteering/data.csv -l thieforienteering -t 1003
python3 ../optimizationtools/optimizationtools/bench_run.py --csv data/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness --main "./bazel-bin/examples/main -p batchschedulingtotalweightedtardiness" -a "iterativebeamsearch" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv data/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -t 61
```

