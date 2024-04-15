# TreeSearchSolver

A solver based on heuristic tree search.

![treesearch](img/treesearch.jpg?raw=true "treesearch")

[image source](https://commons.wikimedia.org/wiki/File:Saint-L%C3%A9ger-l%C3%A8s-Domart,arbre_de_la_croix_Notre-Dame_14.jpg)

## Description

The goal of this repository is to provide a simple framework to quickly implement algorithms based on heuristic tree search.

Solving a problem only requires a couple hundred lines of code (see examples).

Algorithms:
* Greedy `greedy`
* Depth first search `depth-first-search`
* Best first search `best-first-search`
* Iterative beam search `iterative-beam-search`
* Iterative beam search 2 `iterative-beam-search-2`
* Iterative memory bounded best first search `iterative-memory-bounded-best-first-search`
* Anytime column search `anytime-column-search`

## Examples

Data can be downloaded from [fontanf/orproblems](https://github.com/fontanf/orproblems)

[Sequential ordering problem](include/treesearchsolver/examples/sequential_ordering.hpp)

* The branching scheme is taken from "Tree search for the sequential ordering problem" (Libralesso et al., 2020) [PDF](https://ecai2020.eu/papers/1126_paper.pdf).
* It is a forward branching scheme.
* The guide of a node is its bound.
* This implementation returns state-of-the-art results on the instances of the scientific literature with a dense precedence graph.

[Permutation flow shop scheduling problem, makespan](include/treesearchsolver/examples/permutation_flowshop_scheduling_makespan.hpp) and [Permutation flow shop scheduling problem, total completion time](include/treesearchsolver/examples/permutationflowshopschedulingtct.hpp)

* The branching schemes are taken from "Iterative beam search algorithms for the permutation flowshop" (Libralesso et al., 2022) [DOI](https://doi.org/10.1016/j.ejor.2021.10.015).
* For the makespan variant, it is a bidirectional branching scheme.
* For the total completion time variant, it is a forward branching scheme.
* These implementations return state-of-the-art results on the instances of the scientific literature.

[1D knapsack](https://github.com/fontanf/packingsolver/blob/master/packingsolver/onedimensional/branching_scheme.hpp), [2D rectangle knapsack](https://github.com/fontanf/packingsolver/blob/master/packingsolver/rectangle/branching_scheme.hpp) and [3D box-stacks knapsack](https://github.com/fontanf/packingsolver/blob/master/packingsolver/boxstacks/branching_scheme.hpp) problems from [fontanf/packingsolver](https://github.com/fontanf/packingsolver/)

* These are all forward branching scheme.
* These implementations have been used in the winning algorithm of the [Challenge ROADEF/EURO 2022 : Trucks Loading Problem](https://www.roadef.org/challenge/2022/en/).

[Travelling thief problem](https://github.com/fontanf/travellingthiefsolver/blob/master/travellingthiefsolver/travellingthief/algorithms/tree_search.cpp) and [thief orienteering problem](https://github.com/fontanf/travellingthiefsolver/blob/master/travellingthiefsolver/thieforienteering/algorithms/tree_search.cpp) from [fontanf/travellingthiefsolver](https://github.com/fontanf/travellingthiefsolver/)

* Here, the library is used to implement an exact dynamic programming algorithm implemented as a tree search

[Knapsack problem with conflicts](include/treesearchsolver/examples/knapsack_with_conflicts.hpp)

[Simple assembly line balancing problem of type 1 (SALBP-1)](include/treesearchsolver/examples/simple_assembly_line_balancing_1.hpp)

## Usage, running examples from command line

Compile:
```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
cmake --install build --config Release --prefix install
```

Download data:
```shell
python3 scripts/download_data.py
```

Then, examples can be executed as follows:
```shell
./install/bin/treesearchsolver_sequential_ordering --verbosity-level 1 --input "./data/sequential_ordering/soplib/R.700.1000.60.sop" --format soplib --algorithm iterative-beam-search --certificate solution.txt
```
```
======================================
           TreeSearchSolver           
======================================

Instance
--------
Number of locations:  700

Algorithm
---------
Iterative beam search

Parameters
----------
Time limit:                         inf
Messages
    Verbosity level:                1
    Standard output:                1
    File path:                      
    # streams:                      0
Logger
    Has logger:                     0
    Standard error:                 0
    File path:                      
Maximum size of the solution pool:  1
Maximum number of nodes:            -1
Growth factor:                      2
Minimum size of the queue:          1
Maximum size of the queue:          100000000

       Time                           Value                         Comment
       ----                           -----                         -------
      0.000                                                             q 1
      0.017                          277615                             q 2
      0.035                          253795                             q 4
      0.074                          246649                             q 8
      0.138                          245634                            q 16
      0.219                          245589                            q 32

Final statistics
----------------
Value:                      245589
Time:                       0.302434
Number of nodes:            17144
Maximum size of the queue:  64

Solution
--------
Length:  245589

Checker
-------
Number of Vertices:               700 / 700
Number of duplicates:             0
Number of precedence violations:  0
Feasible:                         1
Total distance:                   245589
```

## Usage, C++ library

See examples.
