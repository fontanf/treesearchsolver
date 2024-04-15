import argparse
import sys
import os

parser = argparse.ArgumentParser(description='')
parser.add_argument('directory')
parser.add_argument(
        "-t", "--tests",
        type=str,
        nargs='*',
        help='')

args = parser.parse_args()


if args.tests is None or "knapsack-with-conflicts" in args.tests:
    print("Knapsack problem with conflicts")
    print("-------------------------------")
    print()

    data_dir = os.environ['KNAPSACK_WITH_CONFLICTS_DATA']
    data = [
            (os.path.join("hifi2006", "I1 - I10", "1I1"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "2I2"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "3I3"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "4I4"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "5I5"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "6I1"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "7I2"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "8I3"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "9I4"), "hifi2006"),
            (os.path.join("hifi2006", "I1 - I10", "10I5"), "hifi2006")]
    main = os.path.join(
            "install",
            "bin",
            "treesearchsolver_knapsack_with_conflicts")
    for instance, instance_format in data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "knapsack_with_conflicts",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + " --format \"" + instance_format + "\""
                + "  --algorithm iterative-beam-search"
                + " --minimum-size-of-the-queue 1024"
                + " --maximum-size-of-the-queue 1024"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "permutation-flowshop-scheduling-tct" in args.tests:
    print("Permutation flowshop scheduling problem, total completion time")
    print("--------------------------------------------------------------")
    print()

    data_dir = os.environ['FLOWSHOP_SCHEDULING_DATA']
    data = [
            (os.path.join("taillard1993", "tai20_5_0.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_1.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_2.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_3.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_4.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_5.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_6.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_7.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_8.txt"), "default"),
            (os.path.join("taillard1993", "tai20_5_9.txt"), "default")]
    main = os.path.join(
            "install",
            "bin",
            "treesearchsolver_permutation_flowshop_scheduling_tct")
    for instance, instance_format in data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "permutation_flowshop_scheduling_tct",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + " --format \"" + instance_format + "\""
                + "  --algorithm iterative-beam-search"
                + " --minimum-size-of-the-queue 2048"
                + " --maximum-size-of-the-queue 2048"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "permutation-flowshop-scheduling-makespan" in args.tests:
    print("Permutation flowshop scheduling problem, makespan")
    print("-------------------------------------------------")
    print()

    data_dir = os.environ['FLOWSHOP_SCHEDULING_DATA']
    data = [
            (os.path.join("vallada2015", "Small", "VFR10_5_1_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_2_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_3_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_4_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_5_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_6_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_7_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_8_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_9_Gap.txt"), "default"),
            (os.path.join("vallada2015", "Small", "VFR10_5_10_Gap.txt"), "default")]
    main = os.path.join(
            "install",
            "bin",
            "treesearchsolver_permutation_flowshop_scheduling_makespan")
    for instance, instance_format in data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "permutation_flowshop_scheduling_makespan",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + " --format \"" + instance_format + "\""
                + "  --algorithm iterative-beam-search"
                + " --minimum-size-of-the-queue 2048"
                + " --maximum-size-of-the-queue 2048"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "sequential-ordering-problem" in args.tests:
    print("Sequential ordering problem")
    print("---------------------------")
    print()

    data_dir = os.environ['SEQUENTIAL_ORDERING_DATA']
    data = [
            (os.path.join("soplib", "R.200.100.1.sop"), "soplib"),
            (os.path.join("soplib", "R.200.100.15.sop"), "soplib"),
            (os.path.join("soplib", "R.200.100.30.sop"), "soplib"),
            (os.path.join("soplib", "R.200.100.60.sop"), "soplib"),
            (os.path.join("soplib", "R.200.1000.1.sop"), "soplib"),
            (os.path.join("soplib", "R.200.1000.15.sop"), "soplib"),
            (os.path.join("soplib", "R.200.1000.30.sop"), "soplib"),
            (os.path.join("soplib", "R.200.1000.60.sop"), "soplib")]
    main = os.path.join(
            "install",
            "bin",
            "treesearchsolver_sequential_ordering")
    for instance, instance_format in data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "sequential_ordering",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + " --format \"" + instance_format + "\""
                + "  --algorithm iterative-beam-search"
                + " --minimum-size-of-the-queue 2048"
                + " --maximum-size-of-the-queue 2048"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()


if args.tests is None or "simple-assembly-line-balancing-1" in args.tests:
    print("Simple asssembly line balancing of type 1")
    print("-----------------------------------------")
    print()

    data_dir = os.environ['SIMPLE_ASSEMBLY_LINE_BALANCING_1_DATA']
    data = [
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_50.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_100.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_150.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_200.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_250.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_300.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_350.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_400.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_450.alb"), "otto2013"),
            (os.path.join("otto2013", "medium data set_n=50", "instance_n=50_500.alb"), "otto2013")]
    main = os.path.join(
            "install",
            "bin",
            "treesearchsolver_simple_assembly_line_balancing_1")
    for instance, instance_format in data:
        instance_path = os.path.join(
                data_dir,
                instance)
        json_output_path = os.path.join(
                args.directory,
                "simple_assembly_line_balancing_1",
                instance + ".json")
        if not os.path.exists(os.path.dirname(json_output_path)):
            os.makedirs(os.path.dirname(json_output_path))
        command = (
                main
                + "  --verbosity-level 1"
                + "  --input \"" + instance_path + "\""
                + " --format \"" + instance_format + "\""
                + "  --algorithm iterative-beam-search"
                + " --minimum-size-of-the-queue 4096"
                + " --maximum-size-of-the-queue 4096"
                + "  --output \"" + json_output_path + "\"")
        print(command)
        status = os.system(command)
        if status != 0:
            sys.exit(1)
        print()
    print()
    print()
