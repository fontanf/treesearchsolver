# Tree Search Solver

A solver based on heuristic tree search.

![treesearch](img/treesearch.jpg?raw=true "treesearch")

[image source](https://commons.wikimedia.org/wiki/File:Saint-L%C3%A9ger-l%C3%A8s-Domart,arbre_de_la_croix_Notre-Dame_14.jpg)

## Description

The goal of this repository is to provide a simple framework to quickly implement algorithms based on heuristic tree search.

Solving a problem only requires a couple hundred lines of code (see examples).

Algorithms:
* A\* `astar`
* Iterative Beam Search `iterativebeamsearch`

## Examples

Data can be downloaded from [fontanf/orproblems](https://github.com/fontanf/orproblems)

### Packing

[Knapsack Problem with Conflicts](examples/knapsackwithconflicts.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -a "iterativebeamsearch" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -b heuristiclong -t 62`

</p>
</details>

### Routing

[Travelling Salesman Problem](examples/travellingsalesman.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/travellingsalesman/data.csv -l travellingsalesman -f "'pla85900.tsp' not in row['Path']" -a "iterativebeamsearch" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/travellingsalesman/data.csv -l travellingsalesman -f "'pla85900.tsp' not in row['Path']" -b heuristiclong -t 62`

</p>
</details>

[Sequential Ordering Problem](examples/sequentialordering.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/sequentialordering/data.csv -l sequentialordering -f "row['Dataset'] == 'soplib'" -a "iterativebeamsearch" -t 600`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/sequentialordering/data.csv -l sequentialordering -f "row['Dataset'] == 'soplib'" -b heuristiclong -t 602`

</p>
</details>

[Travelling Repairman Problem / Minimum Latency Problem](examples/travellingrepairman.hpp)

[Thief Orienteering Problem](examples/thieforienteering.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/thieforienteering/data.csv -l thieforienteering -a "iterativebeamsearch" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/thieforienteering/data.csv -l thieforienteering -b heuristiclong -t 1003`

</p>
</details>

### Scheduling

#### Single machine scheduling

[Single machine scheduling problem with sequence-dependent setup times, Total weighted tardiness](examples/schedulingwithsdsttwt.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/schedulingwithsdsttwt/data.csv -l schedulingwithsdsttwt -a "iterativebeamsearch" -t 30`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/schedulingwithsdsttwt/data.csv -l schedulingwithsdsttwt -b heuristiclong -t 31`

</p>
</details>

[Single machine order acceptance and scheduling problem with time windows and sequence-dependent setup times, Total weighted tardiness](examples/orderacceptanceandscheduling.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -a "iterativebeamsearch" -t 60`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -b heuristiclong -t 61`

</p>
</details>

[Single machine batch scheduling problem, Total weighted tardiness](examples/batchschedulingtotalweightedtardiness.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/orderacceptanceandscheduling/data.csv -f "row['Dataset'] == 'cesaret2012'" -l orderacceptanceandscheduling -a "iterativebeamsearch" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/orderacceptanceandscheduling/data.csv -f "row['Dataset'] == 'cesaret2012'" -l orderacceptanceandscheduling -b heuristiclong -t 11`

</p>
</details>

#### Flow shop scheduling

[Permutation flow shop scheduling problem, Makespan](examples/permutationflowshopschedulingmakespan.hpp)
* Bidirectional branching scheme

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/permutationflowshopscheduling/data_makespan.csv -f "row['Dataset'] == 'vallada2015_large'" -l permutationflowshopschedulingmakespan -a "iterativebeamsearch" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_makespan.csv -f "row['Dataset'] == 'vallada2015_large' and int(row['Job number']) <= 100" -l permutationflowshopschedulingmakespan -b heuristiclong -t 500`

</p>
</details>

[Permutation flow shop scheduling problem, Total completion time](examples/permutationflowshopschedulingtct.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/permutationflowshopscheduling/data_totalcompletiontime.csv -l permutationflowshopschedulingtct -a "iterativebeamsearch" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_totalcompletiontime.csv -l permutationflowshopschedulingtct -b heuristiclong -t 500`

</p>
</details>

[Permutation flow shop scheduling problem, Total tardiness](examples/permutationflowshopschedulingtt.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/permutationflowshopscheduling/data_totaltardiness.csv -l permutationflowshopschedulingtt -a "iterativebeamsearch" --timelimitfield "Time limit"`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/permutationflowshopscheduling/data_totaltardiness.csv -l permutationflowshopschedulingtt -b heuristiclong -t 500`

</p>
</details>

#### Assembly Line Balancing

[Simple Assembly Line Balancing Problem of Type 1 (SALBP-1)](examples/simpleassemblylinebalancing1.hpp)

<details><summary>Benchmarks</summary>
<p>

* Benchmarks:
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_small'" -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_medium'" -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_large'" -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_verylarge'" -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -t 10`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_small'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_medium'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_large'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`
  * `python3 ../optimizationtools/optimizationtools/bench_process.py --csv ../ordata/simpleassemblylinebalancing1/data.csv -f "row['Dataset'] == 'otto2013_verylarge'" -l simpleassemblylinebalancing1 -b heuristiclong -t 11`

</p>
</details>

[U-shaped Assembly Line Balancing Problem of Type 1 (UALBP-1)](examples/ushapedassemblylinebalancing1.hpp)

### Others

Pricing problems from [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver)
* [Elementary Shortest Path Problem with Resource Constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprc.hpp)
* [Elementary Shortest Path Problem with Resource Constraint and Time Windows](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprctw.hpp)
* [Elementary Open Shortest Path Problem with Resource Constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/eospprc.hpp)
* [Single machine order acceptance and scheduling problem with family setup times, Total weighted completion time](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/oaschedulingwithfamilysetuptimestwct.hpp)

## Usage, running examples from command line

Compile:
```shell
bazel build -- //...
```

Then, examples can be executed as follows:
```shell
./bazel-bin/examples/main -v -p travellingsalesman -a iterativebeamsearch -i ../ordata/travellingsalesman/tsplib/a280.tsp -t 5
```

## Usage, C++ library

See examples.


Notes for adding new examples:
* Create `examples/problem.hpp`
* Update `examples/read_args.hpp`
* Update `examples/main.cpp`
* Update `examples/checker.hpp`
* Update `examples/BUILD`

