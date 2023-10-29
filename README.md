# TreeSearchSolver

A solver based on heuristic tree search.

![treesearch](img/treesearch.jpg?raw=true "treesearch")

[image source](https://commons.wikimedia.org/wiki/File:Saint-L%C3%A9ger-l%C3%A8s-Domart,arbre_de_la_croix_Notre-Dame_14.jpg)

## Description

The goal of this repository is to provide a simple framework to quickly implement algorithms based on heuristic tree search.

Solving a problem only requires a couple hundred lines of code (see examples).

Algorithms:
* Greedy `greedy`
* Best first search `best-first-search`
* Iterative beam search `iterative-beam-search`
* Iterative beam search 2 `iterative-beam-search-2`
* Iterative memory bounded best first search `iterative-memory-bounded-best-first-search`
* Anytime column search `anytime-column-search`

## Examples

Data can be downloaded from [fontanf/orproblems](https://github.com/fontanf/orproblems)

### Packing

[Knapsack problem with conflicts](examples/knapsackwithconflicts.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/knapsackwithconflicts_main --csv ../ordata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -a iterative-beam-search -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -b heuristiclong -t 62`

</p>
</details>

[1D knapsack](https://github.com/fontanf/packingsolver/blob/master/packingsolver/onedimensional/branching_scheme.hpp), [2D rectangle knapsack](https://github.com/fontanf/packingsolver/blob/master/packingsolver/rectangle/branching_scheme.hpp) and [3D box-stacks knapsack](https://github.com/fontanf/packingsolver/blob/master/packingsolver/boxstacks/branching_scheme.hpp) problems from [fontanf/packingsolver](https://github.com/fontanf/packingsolver/)

### Routing

[Traveling salesman problem](examples/travelingsalesman.hpp)

* Branching schemes: forward, insertion

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/travelingsalesman_main --csv ../ordata/travelingsalesman/data.csv -l travelingsalesman -f "'pla85900.tsp' not in row['Path']" -a "iterative-beam-search" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/travelingsalesman/data.csv -l travelingsalesman -f "'pla85900.tsp' not in row['Path']" -b heuristiclong -t 62`

</p>
</details>

[Sequential ordering problem](examples/sequentialordering.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

```shell
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/sequentialordering_main --csv ../ordata/sequentialordering/data.csv -l "${DATE}_sequentialordering" -f "row['Dataset'] == 'soplib'" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/sequentialordering/data.csv -l "${DATE}_sequentialordering" -f "row['Dataset'] == 'soplib'" -b heuristiclong -t 61
```

![sequentialordering](img/sequentialordering.png?raw=true "sequentialordering")

</p>
</details>

[Traveling repairman problem / minimum latency problem](examples/travelingrepairman.hpp)

* Branching scheme: forward

[Travelling thief problem](https://github.com/fontanf/travellingthiefsolver/blob/master/travellingthiefsolver/algorithms/tree_search.cpp) and [thief orienteering problem](https://github.com/fontanf/travellingthiefsolver/blob/master/thieforienteeringsolver/algorithms/tree_search.cpp) from [fontanf/travellingthiefsolver](https://github.com/fontanf/travellingthiefsolver/)

* Here, the library is used to implement an exact dynamic programming algorithm implemented as a tree search

### Scheduling

#### Single machine scheduling

[Single machine scheduling problem with sequence-dependent setup times, total weighted tardiness](examples/schedulingwithsdsttwt.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/schedulingwithsdsttwt_main --csv ../ordata/schedulingwithsdsttwt/data.csv -l schedulingwithsdsttwt -a "iterative-beam-search" -t 30`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/schedulingwithsdsttwt/data.csv -l schedulingwithsdsttwt -b heuristiclong -t 31`

</p>
</details>

[Single machine order acceptance and scheduling problem with time windows and sequence-dependent setup times, total weighted tardiness](examples/orderacceptanceandscheduling.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/orderacceptanceandscheduling_main --csv ../ordata/orderacceptanceandscheduling/data.csv -l orderacceptanceandscheduling -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/orderacceptanceandscheduling/data.csv -l orderacceptanceandscheduling -b heuristiclong -t 61`

</p>
</details>

[Single machine batch scheduling problem, total weighted tardiness](examples/batchschedulingtotalweightedtardiness.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/orderacceptanceandscheduling_main --csv ../ordata/orderacceptanceandscheduling/data.csv -f "row['Dataset'] == 'cesaret2012'" -l orderacceptanceandscheduling -a "iterative-beam-search" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/orderacceptanceandscheduling/data.csv -f "row['Dataset'] == 'cesaret2012'" -l orderacceptanceandscheduling -b heuristiclong -t 11`

</p>
</details>

#### Flow shop scheduling

[Permutation flow shop scheduling problem, makespan](examples/permutationflowshopschedulingmakespan.hpp)

* Branching schemes: bidirectional, insertion

<details><summary>Benchmarks</summary>
<p>

```shell
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingmakespan_main --csv ../ordata/flowshopscheduling/data_permutationflowshopschedulingmakespan.csv -l "${DATE}_permutationflowshopschedulingmakespan" --timelimitfield "Time limit"
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/flowshopscheduling/data_permutationflowshopschedulingmakespan.csv -l "${DATE}_permutationflowshopschedulingmakespan" -b heuristiclong -t 500
```

</p>
</details>

[Permutation flow shop scheduling problem, total completion time](examples/permutationflowshopschedulingtct.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

```shell
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingtct_main --csv ../ordata/flowshopscheduling/data_permutationflowshopschedulingtct.csv -l "${DATE}_permutationflowshopschedulingtct" --timelimitfield "Time limit"
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/flowshopscheduling/data_permutationflowshopschedulingtct.csv -l "${DATE}_permutationflowshopschedulingtct" -b heuristiclong -t 500
```

![permutationflowshopschedulingtct](img/permutationflowshopschedulingtct.png?raw=true "permutationflowshopschedulingtct")

</p>
</details>

[Permutation flow shop scheduling problem, total tardiness](examples/permutationflowshopschedulingtt.hpp)

* Branching schemes: forward, insertion

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingtt_main --csv ../ordata/permutationflowshopscheduling/data_totaltardiness.csv -l permutationflowshopschedulingtt -a "iterative-beam-search" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_totaltardiness.csv -l permutationflowshopschedulingtt -b heuristiclong -t 500`

</p>
</details>

#### Job shop scheduling

[No-wait job shop scheduling problem, makespan](examples/nowaitjobshopschedulingmakespan.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/nowaitjobshopschedulingmakespan_main --csv ../ordata/jobshopscheduling/data_nowait.csv -l nowaitjobshopschedulingmakespan --timelimit "1000"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/jobshopscheduling/data_nowait.csv -l nowaitjobshopschedulingmakespan -b heuristiclong -t 1010`

</p>
</details>

#### Assembly line balancing

[Simple assembly line balancing problem of type 1 (SALBP-1)](examples/simpleassemblylinebalancing1.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

```shell
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_small'" -l "${DATE}_simpleassemblylinebalancing1_small" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_small'" -l "${DATE}_simpleassemblylinebalancing1_small" -b heuristiclong -t 61
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_medium'" -l "${DATE}_simpleassemblylinebalancing1_medium" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_medium'" -l "${DATE}_simpleassemblylinebalancing1_medium" -b heuristiclong -t 61
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_large'" -l "${DATE}_simpleassemblylinebalancing1_large" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_large'" -l "${DATE}_simpleassemblylinebalancing1_large" -b heuristiclong -t 61
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_verylarge'" -l "${DATE}_simpleassemblylinebalancing1_verylarge" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_verylarge'" -l "${DATE}_simpleassemblylinebalancing1_verylarge" -b heuristiclong -t 61
```

![simpleassemblylinebalancing1](img/simpleassemblylinebalancing1_small.png?raw=true "simpleassemblylinebalancing1")
![simpleassemblylinebalancing1](img/simpleassemblylinebalancing1_medium.png?raw=true "simpleassemblylinebalancing1")
![simpleassemblylinebalancing1](img/simpleassemblylinebalancing1_large.png?raw=true "simpleassemblylinebalancing1")
![simpleassemblylinebalancing1](img/simpleassemblylinebalancing1_verylarge.png?raw=true "simpleassemblylinebalancing1")

</p>
</details>

[U-shaped assembly line balancing problem of type 1 (UALBP-1)](examples/ushapedassemblylinebalancing1.hpp)

* Branching scheme: forward

### Others

Pricing problems from [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver)
* [Elementary shortest path problem with resource constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprc.hpp)
  * Branching scheme: forward
* [Elementary shortest path problem with resource constraint and time windows](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprctw.hpp)
  * Branching scheme: forward
* [Elementary open shortest path problem with resource constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/eospprc.hpp)
  * Branching scheme: forward
* [Single machine order acceptance and scheduling problem with family setup times, total weighted completion time](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/oaschedulingwithfamilysetuptimestwct.hpp)
  * Branching scheme: forward

## Usage, running examples from command line

Compile:
```shell
bazel build -- //...
```

Then, examples can be executed as follows:
```shell
./bazel-bin/examples/sequentialordering_main --verbosity-level 1 --input "../ordata/sequentialordering/soplib/R.700.1000.60.sop" --format soplib --algorithm iterative-beam-search --certificate solution.txt
```
```
Instance
--------
Number of locations:  700

======================================
           TreeSearchSolver           
======================================

Algorithm
---------
Iterative Beam Search

Parameters
----------
Minimum size of the queue:   1
Maximum size of the queue:   100000000
Maximum number of nodes:     -1
Growth factor:               2
Maximum size of the pool:    1
Time limit:                  inf

       Time                           Value                         Comment
       ----                           -----                         -------
      0.069                                                             q 1
      0.086                          277615                             q 2
      0.105                          253795                             q 4
      0.144                          246649                             q 8
      0.209                          245634                            q 16
      0.291                          245589                            q 32

Final statistics
----------------
Value:                      245589
Time:                       0.375687
Number of nodes:            17144
Maximum size of the queue:  64

Solution
--------
Length:            245589

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


Notes for adding new examples:
* Create `examples/problem.hpp`
* Update `examples/read_args.hpp`
* Update `examples/main.cpp`
* Update `examples/checker.hpp`
* Update `examples/BUILD`

