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
* Iterative Memory Bounded Best First Search `iterative_memory_bounded_best_first_search`

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

[Travelling Salesman Problem](examples/travellingsalesman.hpp)

* Branching schemes: forward, insertion

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/travellingsalesman_main --csv ../ordata/travellingsalesman/data.csv -l travellingsalesman -f "'pla85900.tsp' not in row['Path']" -a "iterative_beam_search" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/travellingsalesman/data.csv -l travellingsalesman -f "'pla85900.tsp' not in row['Path']" -b heuristiclong -t 62`

</p>
</details>

[Sequential Ordering Problem](examples/sequentialordering.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/sequentialordering_main --csv ../ordata/sequentialordering/data.csv -l sequentialordering -f "row['Dataset'] == 'soplib'" -a "iterative_beam_search" -t 600`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/sequentialordering/data.csv -l sequentialordering -f "row['Dataset'] == 'soplib'" -b heuristiclong -t 602`

</p>
</details>

[Travelling Repairman Problem / Minimum Latency Problem](examples/travellingrepairman.hpp)

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
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/orderacceptanceandscheduling_main --csv ../ordata/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -a "iterative_beam_search" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -b heuristiclong -t 61`

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

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingmakespan_main --csv ../ordata/permutationflowshopscheduling/data_makespan.csv -f "row['Dataset'] == 'vallada2015_large'" -l permutationflowshopschedulingmakespan -a "iterative_beam_search" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_makespan.csv -f "row['Dataset'] == 'vallada2015_large' and int(row['Job number']) <= 100" -l permutationflowshopschedulingmakespan -b heuristiclong -t 500`

</p>
</details>

[Permutation flow shop scheduling problem, Total completion time](examples/permutationflowshopschedulingtct.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/permutationflowshopschedulingtct_main --csv ../ordata/permutationflowshopscheduling/data_totalcompletiontime.csv -l permutationflowshopschedulingtct -a "iterative_beam_search" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_totalcompletiontime.csv -l permutationflowshopschedulingtct -b heuristiclong -t 500`

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

#### Assembly Line Balancing

[Simple Assembly Line Balancing Problem of Type 1 (SALBP-1)](examples/simpleassemblylinebalancing1.hpp)

* Branching scheme: forward

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_small'" -l simpleassemblylinebalancing1 -a "iterative_beam_search" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_medium'" -l simpleassemblylinebalancing1 -a "iterative_beam_search" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_large'" -l simpleassemblylinebalancing1 -a "iterative_beam_search" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --main ./bazel-bin/examples/simpleassemblylinebalancing1_main --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_verylarge'" -l simpleassemblylinebalancing1 -a "iterative_beam_search" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_small'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_medium'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_large'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_verylarge'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`

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
./bazel-bin/examples/travellingsalesman_main -v -a iterative_beam_search -i ../ordata/travellingsalesman/tsplib/a280.tsp -t 5
```

## Usage, C++ library

See examples.


Notes for adding new examples:
* Create `examples/problem.hpp`
* Update `examples/read_args.hpp`
* Update `examples/main.cpp`
* Update `examples/checker.hpp`
* Update `examples/BUILD`

