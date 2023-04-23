# Tree Search Solver

A solver based on heuristic tree search.

![treesearch](img/treesearch.jpg?raw=true "treesearch")

[image source](https://commons.wikimedia.org/wiki/File:Saint-L%C3%A9ger-l%C3%A8s-Domart,arbre_de_la_croix_Notre-Dame_14.jpg)

## Description

The goal of this repository is to provide a simple framework to quickly implement algorithms based on heuristic tree search.

Solving a problem only requires a couple hundred lines of code (see examples).

Algorithms:
* Greedy `greedy`
* Best First Search `best_first_search`
* Iterative Beam Search `iterative_beam_search`
* Iterative Beam Search 2 `iterative_beam_search_2`
* Iterative Memory Bounded Best First Search `iterative_memory_bounded_best_first_search`
* Anytime Column Search `anytime_column_search`

## Examples

Data can be downloaded from [fontanf/orproblems](https://github.com/fontanf/orproblems)

### Packing

[Knapsack Problem with Conflicts](examples/knapsackwithconflicts.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/knapsackwithconflicts_main --csv ../ordata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -a iterative_beam_search -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -b heuristiclong -t 62`

</p>
</details>

### Routing

[Traveling Salesman Problem](examples/travelingsalesman.hpp)

* Branching schemes: forward, insertion

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/travelingsalesman_main --csv ../ordata/travelingsalesman/data.csv -l travelingsalesman -f "'pla85900.tsp' not in row['Path']" -a "iterative_beam_search" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/travelingsalesman/data.csv -l travelingsalesman -f "'pla85900.tsp' not in row['Path']" -b heuristiclong -t 62`

</p>
</details>

[Sequential Ordering Problem](examples/sequentialordering.hpp)

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

[Traveling Repairman Problem / Minimum Latency Problem](examples/travelingrepairman.hpp)

* Branching scheme: forward

[Thief Orienteering Problem](examples/thieforienteering.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/thieforienteering_main --csv ../ordata/thieforienteering/data.csv -l thieforienteering -a "iterative_beam_search" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/thieforienteering/data.csv -l thieforienteering -b heuristiclong -t 1003`

</p>
</details>

### Scheduling

#### Single machine scheduling

[Single machine scheduling problem with sequence-dependent setup times, Total weighted tardiness](examples/schedulingwithsdsttwt.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/schedulingwithsdsttwt_main --csv ../ordata/schedulingwithsdsttwt/data.csv -l schedulingwithsdsttwt -a "iterative_beam_search" -t 30`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/schedulingwithsdsttwt/data.csv -l schedulingwithsdsttwt -b heuristiclong -t 31`

</p>
</details>

[Single machine order acceptance and scheduling problem with time windows and sequence-dependent setup times, Total weighted tardiness](examples/orderacceptanceandscheduling.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/orderacceptanceandscheduling_main --csv ../ordata/orderacceptanceandscheduling/data.csv -l orderacceptanceandscheduling -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/orderacceptanceandscheduling/data.csv -l orderacceptanceandscheduling -b heuristiclong -t 61`

</p>
</details>

[Single machine batch scheduling problem, Total weighted tardiness](examples/batchschedulingtotalweightedtardiness.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/orderacceptanceandscheduling_main --csv ../ordata/orderacceptanceandscheduling/data.csv -f "row['Dataset'] == 'cesaret2012'" -l orderacceptanceandscheduling -a "iterative_beam_search" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/orderacceptanceandscheduling/data.csv -f "row['Dataset'] == 'cesaret2012'" -l orderacceptanceandscheduling -b heuristiclong -t 11`

</p>
</details>

#### Flow shop scheduling

[Permutation flow shop scheduling problem, Makespan](examples/permutationflowshopschedulingmakespan.hpp)

* Branching schemes: bidirectional, insertion

<details><summary>Benchmarks</summary>
<p>

```shell
DATE=$(date '+%Y-%m-%d--%H-%M-%S') && python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingmakespan_main --csv ../ordata/flowshopscheduling/data_permutationflowshopschedulingmakespan.csv -l "${DATE}_permutationflowshopschedulingmakespan" --timelimitfield "Time limit"
python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/flowshopscheduling/data_permutationflowshopschedulingmakespan.csv -l "${DATE}_permutationflowshopschedulingmakespan" -b heuristiclong -t 500
```

</p>
</details>

[Permutation flow shop scheduling problem, Total completion time](examples/permutationflowshopschedulingtct.hpp)

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

[Permutation flow shop scheduling problem, Total tardiness](examples/permutationflowshopschedulingtt.hpp)

* Branching schemes: forward, insertion

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingtt_main --csv ../ordata/permutationflowshopscheduling/data_totaltardiness.csv -l permutationflowshopschedulingtt -a "iterative_beam_search" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_totaltardiness.csv -l permutationflowshopschedulingtt -b heuristiclong -t 500`

</p>
</details>

#### Job shop scheduling

[No-wait job shop scheduling problem, Makespan](examples/nowaitjobshopschedulingmakespan.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/nowaitjobshopschedulingmakespan_main --csv ../ordata/jobshopscheduling/data_nowait.csv -l nowaitjobshopschedulingmakespan --timelimit "1000"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/jobshopscheduling/data_nowait.csv -l nowaitjobshopschedulingmakespan -b heuristiclong -t 1010`

</p>
</details>

#### Assembly Line Balancing

[Simple Assembly Line Balancing Problem of Type 1 (SALBP-1)](examples/simpleassemblylinebalancing1.hpp)

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

[U-shaped Assembly Line Balancing Problem of Type 1 (UALBP-1)](examples/ushapedassemblylinebalancing1.hpp)

* Branching scheme: forward

### Others

Pricing problems from [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver)
* [Elementary Shortest Path Problem with Resource Constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprc.hpp)
  * Branching scheme: forward
* [Elementary Shortest Path Problem with Resource Constraint and Time Windows](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprctw.hpp)
  * Branching scheme: forward
* [Elementary Open Shortest Path Problem with Resource Constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/eospprc.hpp)
  * Branching scheme: forward
* [Single machine order acceptance and scheduling problem with family setup times, Total weighted completion time](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/oaschedulingwithfamilysetuptimestwct.hpp)
  * Branching scheme: forward

## Usage, running examples from command line

Compile:
```shell
bazel build -- //...
```

Then, examples can be executed as follows:
```shell
./bazel-bin/examples/travelingsalesman_main -v 1 -a iterative_beam_search -i ../ordata/travelingsalesman/tsplib/a280.tsp -t 5 -c solution.txt
```
```
Instance
--------
Number of vertices:  280

======================================
          Tree Search Solver          
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
Time limit:                  5

       Time                           Value                         Comment
       ----                           -----                         -------
      0.003                                                             q 1
      0.003                            3285                             q 2
      0.003                            3285                             q 4
      0.004                            3285                             q 8
      0.006                            3285                            q 16
      0.010                            3285                            q 32
      0.018                            3285                            q 64
      0.036                            3285                           q 128
      0.075                            3285                           q 256
      0.162                            3285                           q 512
      0.339                            3285                          q 1024
      0.745                            3275                          q 2048
      1.581                            3275                          q 4096
      3.608                            3258                          q 8192

Final statistics
----------------
Value:                      3258
Time:                       5
Number of nodes:            4064375
Maximum size of the queue:  8192

Solution
--------
Number of vertices:   280 / 280
Distance:             3258

Checker
-------
Number of vertices:     280 / 280
Number of duplicates:   0
Feasible:               1
Total distance:         3258
```

## Usage, C++ library

See examples.


Notes for adding new examples:
* Create `examples/problem.hpp`
* Update `examples/read_args.hpp`
* Update `examples/main.cpp`
* Update `examples/checker.hpp`
* Update `examples/BUILD`

