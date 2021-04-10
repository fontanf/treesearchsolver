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

### Packing

[Knapsack Problem with Conflicts](examples/knapsackwithconflicts.hpp)
* Literature:
  * "Local branching-based algorithms for the disjunctively constrained knapsack problem" (Akeb et al., 2011) [DOI](https://doi.org/10.1016/j.cie.2011.01.019)
  * "Bin Packing with Conflicts: A Generic Branch-and-Price Algorithm" (Sadykov et Vanderbeck, 2012) [DOI](https://doi.org/10.1287/ijoc.1120.0499)
  * "A Branch-and-Bound Algorithm for the Knapsack Problem with Conflict Graph" (Bettinelli et al., 2017) [DOI](https://doi.org/10.1287/ijoc.2016.0742)
  * "A new combinatorial branch-and-bound algorithm for the Knapsack Problem with Conflicts" (Coniglio et al., 2020) [DOI](https://doi.org/10.1016/j.ejor.2020.07.023)

### Routing

[Travelling Salesman Problem](examples/travellingsalesman.hpp)
* Three field classification: `1 | sᵢⱼ | Cₘₐₓ`

[Sequential Ordering Problem](examples/sequentialordering.hpp)
* Three field classification: `1 | sᵢⱼ, prec | Cₘₐₓ`
* Literature:
  * "A hybrid particle swarm optimization approach for the sequential ordering problem" (Anghinolfi et al., 2011) [DOI](https://doi.org/10.1016/j.cor.2010.10.014)
  * "A bee colony optimization with automated parameter tuning for sequential ordering problem" (Wun et al., 2014) [DOI](https://doi.org/10.1109/WICT.2014.7077286)
  * "An improved Ant Colony System for the Sequential Ordering Problem" (Skinderowicz, 2017) [DOI](https://doi.org/10.1016/j.cor.2017.04.012)
  * "An extension of the Lin-Kernighan-Helsgaun TSP solver for constrained traveling salesman and vehicle routing problems" (Helsgaun, 2017)
  * "Tree search algorithms for the Sequential Ordering Problem" (Libralesso et al., 2019)

[Thief Orienteering Problem](examples/thieforienteering.hpp)
* Description
  * Determine a subset of items to collect
  * Maximize the profit of collected items
  * Each item is in a location, a location may contain multiple items
  * Each collected item decreases the speed of the traveller
  * Time limit and capacity constraint
* Literature
  * "The Thief Orienteering Problem: Formulation and Heuristic Approaches" (Santos et Chagas, 2018) [DOI](https://doi.org/10.1109/CEC.2018.8477853)
  * "Ants can orienteer a thief in their robbery" (Chagas et Wagner, 2020) [DOI](https://doi.org/10.1016/j.orl.2020.08.011)

### Scheduling

#### Single machine scheduling

[Single machine order acceptance and scheduling problem with time windows and sequence-dependent setup times, Total weighted tardiness](examples/orderacceptanceandscheduling.hpp)
* Three field classification: `1 | rⱼ, sᵢⱼ, reject, đⱼ | ∑wⱼTⱼ - ∑vⱼ`
* Literature
  * "A tabu search algorithm for order acceptance and scheduling" (Cesaret et al., 2012) [DOI](https://doi.org/10.1016/j.cor.2010.09.018)
  * "Hybrid evolutionary approaches for the single machine order acceptance and scheduling problem" (Chaurasia et Singh, 2017) [DOI](https://doi.org/10.1016/j.asoc.2016.09.051)
  * "Exact and heuristic algorithms for order acceptance and scheduling with sequence-dependent setup times" (Silva et al., 2018) [DOI](https://doi.org/10.1016/j.cor.2017.09.006)
  * "Tabu-Based Large Neighbourhood Search for Time/Sequence-Dependent Scheduling Problems with Time Windows" (He et al., 2019)
  * "Single-machine scheduling with release times, deadlines, setup times, and rejection" (de Weerdt et al., 2020) [DOI](https://doi.org/10.1016/j.ejor.2020.09.042)

[Single machine batch scheduling problem, Total weighted tardiness](examples/batchschedulingtotalweightedtardiness.hpp)
* Three field classification: `1 | batch, rⱼ, sⱼ, compt | ∑wⱼTⱼ`
* Literature
  * "Scheduling a batch processing machine to minimize total weighted tardiness" (Chou et Wang, 2008)
  * "Minimizing total weighted tardiness on a batch-processing machine with non-agreeable release times and due dates" (Mathirajan et al., 2010) [DOI](https://doi.org/10.1007/s00170-009-2342-y)
  * "Solving single batch-processing machine problems using an iterated heuristic" (Wang, 2011) [DOI](https://doi.org/10.1080/00207543.2010.518995)
  * "Iterated local search for single machine total weighted tardiness batch scheduling" (Queiroga et al., 2020) [DOI](https://doi.org/10.1007/s10732-020-09461-x)

#### Flow shop scheduling

[Permutation flow shop scheduling problem, Makespan](examples/permutationflowshopschedulingmakespan.hpp)
* Three field classification: `Fm | prmu | Cₘₐₓ`
* Literature:
  * "An Effective Hybrid Heuristic for Flow Shop Scheduling" (Zheng et Wang, 2003) [DOI](https://doi.org/10.1007/s001700300005)
  * "A simple and effective iterated greedy algorithm for the permutation flowshop scheduling problem" (Ruiz et Stützle, 2007) [DOI](https://doi.org/10.1016/j.ejor.2005.12.009)
  * "A Variable Block Insertion Heuristic for Solving Permutation Flow Shop Scheduling Problem with Makespan Criterion" (Kizilay et al., 2019) [DOI](https://doi.org/10.3390/a12050100)
  * "A best-of-breed iterated greedy for the permutation flowshop scheduling problem with makespan objective" (Fernandez-Viagas, Framinan, 2019) [DOI](https://doi.org/10.1016/j.cor.2019.104767)
  * "A memetic algorithm with novel semi-constructive evolution operators for permutation flowshop scheduling problem" (Kurdi, 2020) [DOI](https://doi.org/10.1016/j.asoc.2020.106458)
  * "Iterative beam search algorithms for the permutation flowshop" (Libralesso et al., 2020)

[Permutation flow shop scheduling problem, Total completion time](examples/permutationflowshopschedulingtct.hpp)
* Three field classification: `Fm | prmu | ∑Cⱼ`
* Literature:
  * "Constructive and composite heuristic solutions to the P//∑Ci scheduling problem" (Liu et Reeves, 2001) [DOI](https://doi.org/10.1016/S0377-2217(00)00137-5)
  * "Local search methods for the flowshop scheduling problem with flowtime minimization" (Pan et Ruiz, 2012) [DOI](https://doi.org/10.1016/j.ejor.2012.04.034)
  * "A new set of high-performing heuristics to minimise flowtime in permutation flowshops" (Fernandez-Viagas et Framinan, 2015) [DOI](https://doi.org/10.1016/j.cor.2014.08.004)
  * "A beam-search-based constructive heuristic for the PFSP to minimise total flowtime" (Fernandez-Viagas et Framinan, 2017) [DOI](https://doi.org/10.1016/j.cor.2016.12.020)
  * "Minimizing flowtime in a flowshop scheduling problem with a biased random-key genetic algorithm" (Andrade et al., 2019) [DOI](https://doi.org/10.1016/j.eswa.2019.03.007)
  * "Iterative beam search algorithms for the permutation flowshop" (Libralesso et al., 2020)

#### Assembly Line Balancing

[Simple Assembly Line Balancing Problem of Type 1 (SALBP-1)](examples/simpleassemblylinebalancing1.hpp)
* Bin Packing Problem with precedence constraints of the form `bin(j1) <= bin(j2) `
* Literature:
  * "Simple assembly line balancing—Heuristic approaches" (Scholl et Voß, 1997) [DOI](https://doi.org/10.1007/BF00127358)
  * "State-of-the-art exact and heuristic solution procedures for simple assembly line balancing" (Scholl et Becker, 2006) [DOI](https://doi.org/10.1016/j.ejor.2004.07.022)
  * "Beam-ACO for Simple Assembly Line Balancing" (Blum, 2008) [DOI](https://doi.org/10.1287/ijoc.1080.0271)
  * "A Branch, Bound, and Remember Algorithm for the Simple Assembly Line Balancing Problem" (Sewell et Jacobson, 2011) [DOI](https://doi.org/10.1287/ijoc.1110.0462)
  * "An application of the branch, bound, and remember algorithm to a new simple assembly line balancing dataset" (Morrison et al., 2014) [DOI](https://doi.org/10.1016/j.ejor.2013.11.033)

[U-shaped Assembly Line Balancing Problem of Type 1 (UALBP-1)](examples/ushapedassemblylinebalancing1.hpp)
* Literature
  * "Branch, bound and remember algorithm for U-shaped assembly line balancing problem" (Li et al., 2018) [DOI](https://doi.org/10.1016/j.cie.2018.06.037)
  * "Enhanced beam search heuristic for U-shaped assembly line balancing problems" (Li et al., 2020) [DOI](https://doi.org/10.1080/0305215X.2020.1741569)

### Others

Pricing problems from [fontanf/columngenerationsolver](https://github.com/fontanf/columngenerationsolver)
* [Elementary Shortest Path Problem with Resource Constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprc.hpp)
* [Elementary Shortest Path Problem with Resource Constraint and Time Windows](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/espprctw.hpp)
* [Elementary Open Shortest Path Problem with Resource Constraint](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/eospprc.hpp)
* [Single machine order acceptance and scheduling problem with family setup times, Total weighted completion time](https://github.com/fontanf/columngenerationsolver/blob/master/examples/pricingsolver/oaschedulingwithfamilysetuptimestwct.hpp)

## Usage, running examples from command line

[Download data](https://github.com/fontanf/treesearchsolver/releases/download/data/treesearchdata.7z)

Compile:
```shell
bazel build -- //...
```

Then, examples can be executed as follows:
```shell
./bazel-bin/examples/main -v -p travellingsalesman -a iterativebeamsearch -i ../treesearchdata/travellingsalesman/tsplib/a280.tsp -t 5
```

## Usage, C++ library

See examples.

## Benchmarks

```
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -a "iterativebeamsearch" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/knapsackwithconflicts/data.csv -l knsapsackwithconflicts -t 62

python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/travellingsalesman/data.csv -l travellingsalesman -a "iterativebeamsearch" -f "'pla85900.tsp' not in row['Path']" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/travellingsalesman/data.csv -l travellingsalesman -f "'pla85900.tsp' not in row['Path']" -t 62
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/sequentialordering/data.csv -l sequentialordering -a "iterativebeamsearch" -f "row['Dataset'] == 'soplib'" -t 600
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/sequentialordering/data.csv -l sequentialordering -f "row['Dataset'] == 'soplib'" -t 602
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/thieforienteering/data.csv -l thieforienteering -a "iterativebeamsearch" --timelimitfield "Time limit"
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/thieforienteering/data.csv -l thieforienteering -t 1003

python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -a "iterativebeamsearch" -t 60
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/batchschedulingtotalweightedtardiness/data.csv -l batchschedulingtotalweightedtardiness -t 61
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/orderacceptanceandscheduling/data.csv -l orderacceptanceandscheduling -a "iterativebeamsearch" -f "row['Dataset'] == 'cesaret2012'" -t 10
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/orderacceptanceandscheduling/data.csv -l orderacceptanceandscheduling -f "row['Dataset'] == 'cesaret2012'" -t 11

python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/permutationflowshopscheduling/data_makespan.csv -l permutationflowshopschedulingmakespan -a "iterativebeamsearch" --timelimitfield "Time limit" -f "row['Dataset'] == 'vallada2015_large'"
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/permutationflowshopscheduling/data_makespan.csv -l permutationflowshopschedulingmakespan -t 500 -f "row['Dataset'] == 'vallada2015_large' and int(row['Job number']) <= 100"
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/permutationflowshopscheduling/data_totalcompletiontime.csv -l permutationflowshopschedulingtct -a "iterativebeamsearch" --timelimitfield "Time limit"
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/permutationflowshopscheduling/data_totalcompletiontime.csv -l permutationflowshopschedulingtct -t 500

python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -f "row['Dataset'] == 'otto2013_small'" -t 10
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -f "row['Dataset'] == 'otto2013_medium'" -t 10
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -f "row['Dataset'] == 'otto2013_large'" -t 10
python3 ../optimizationtools/optimizationtools/bench_run.py --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -a "iterativebeamsearch" -f "row['Dataset'] == 'otto2013_verylarge'" -t 10
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -f "row['Dataset'] == 'otto2013_small'" -t 11
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -f "row['Dataset'] == 'otto2013_medium'" -t 11
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -f "row['Dataset'] == 'otto2013_large'" -t 11
python3 ../optimizationtools/optimizationtools/bench_process.py -b heuristiclong --csv ../treesearchdata/simpleassemblylinebalancing1/data.csv -l simpleassemblylinebalancing1 -f "row['Dataset'] == 'otto2013_verylarge'" -t 11
```

Notes for adding new examples:
* Create `examples/problem.hpp`
* Update `examples/read_args.hpp`
* Update `examples/main.cpp`
* Update `examples/checker.hpp`
* Update `examples/BUILD`

