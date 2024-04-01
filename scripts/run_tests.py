import sys
import os

test_results_directory = sys.argv[1]

knapsack_with_conflicts_data = [
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
knapsack_with_conflicts_main = os.path.join(
        "bazel-bin",
        "examples",
        "knapsack_with_conflicts_main")
for instance, instance_format in knapsack_with_conflicts_data:
    instance_path = os.path.join(
            "data",
            "knapsack_with_conflicts",
            instance)
    json_output_path = os.path.join(
            test_results_directory,
            "knapsack_with_conflicts",
            instance + ".json")
    if not os.path.exists(os.path.dirname(json_output_path)):
        os.makedirs(os.path.dirname(json_output_path))
    command = (
            knapsack_with_conflicts_main
            + "  --verbosity-level 1"
            + "  --input \"" + instance_path + "\""
            + " --format \"" + instance_format + "\""
            + "  --algorithm iterative-beam-search"
            + " --minimum-size-of-the-queue 1024"
            + " --maximum-size-of-the-queue 1024"
            + "  --output \"" + json_output_path + "\"")
    print(command)
    os.system(command)
    print()

permutation_flowshop_scheduling_tct_data = [
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
permutation_flowshop_scheduling_tct_main = os.path.join(
        "bazel-bin",
        "examples",
        "permutation_flowshop_scheduling_tct_main")
for instance, instance_format in permutation_flowshop_scheduling_tct_data:
    instance_path = os.path.join(
            "data",
            "flowshop_scheduling",
            instance)
    json_output_path = os.path.join(
            test_results_directory,
            "permutation_flowshop_scheduling_tct",
            instance + ".json")
    if not os.path.exists(os.path.dirname(json_output_path)):
        os.makedirs(os.path.dirname(json_output_path))
    command = (
            permutation_flowshop_scheduling_tct_main
            + "  --verbosity-level 1"
            + "  --input \"" + instance_path + "\""
            + " --format \"" + instance_format + "\""
            + "  --algorithm iterative-beam-search"
            + " --minimum-size-of-the-queue 2048"
            + " --maximum-size-of-the-queue 2048"
            + "  --output \"" + json_output_path + "\"")
    print(command)
    os.system(command)
    print()

permutation_flowshop_scheduling_makespan_data = [
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
permutation_flowshop_scheduling_makespan_main = os.path.join(
        "bazel-bin",
        "examples",
        "permutation_flowshop_scheduling_makespan_main")
for instance, instance_format in permutation_flowshop_scheduling_makespan_data:
    instance_path = os.path.join(
            "data",
            "flowshop_scheduling",
            instance)
    json_output_path = os.path.join(
            test_results_directory,
            "permutation_flowshop_scheduling_makespan",
            instance + ".json")
    if not os.path.exists(os.path.dirname(json_output_path)):
        os.makedirs(os.path.dirname(json_output_path))
    command = (
            permutation_flowshop_scheduling_makespan_main
            + "  --verbosity-level 1"
            + "  --input \"" + instance_path + "\""
            + " --format \"" + instance_format + "\""
            + "  --algorithm iterative-beam-search"
            + " --minimum-size-of-the-queue 2048"
            + " --maximum-size-of-the-queue 2048"
            + "  --output \"" + json_output_path + "\"")
    print(command)
    os.system(command)
    print()

sequential_ordering_data = [
        (os.path.join("soplib", "R.200.100.1.sop"), "soplib"),
        (os.path.join("soplib", "R.200.100.15.sop"), "soplib"),
        (os.path.join("soplib", "R.200.100.30.sop"), "soplib"),
        (os.path.join("soplib", "R.200.100.60.sop"), "soplib"),
        (os.path.join("soplib", "R.200.1000.1.sop"), "soplib"),
        (os.path.join("soplib", "R.200.1000.15.sop"), "soplib"),
        (os.path.join("soplib", "R.200.1000.30.sop"), "soplib"),
        (os.path.join("soplib", "R.200.1000.60.sop"), "soplib")]
sequential_ordering_main = os.path.join(
        "bazel-bin",
        "examples",
        "sequential_ordering_main")
for instance, instance_format in sequential_ordering_data:
    instance_path = os.path.join(
            "data",
            "sequential_ordering",
            instance)
    json_output_path = os.path.join(
            test_results_directory,
            "sequential_ordering",
            instance + ".json")
    if not os.path.exists(os.path.dirname(json_output_path)):
        os.makedirs(os.path.dirname(json_output_path))
    command = (
            sequential_ordering_main
            + "  --verbosity-level 1"
            + "  --input \"" + instance_path + "\""
            + " --format \"" + instance_format + "\""
            + "  --algorithm iterative-beam-search"
            + " --minimum-size-of-the-queue 2048"
            + " --maximum-size-of-the-queue 2048"
            + "  --output \"" + json_output_path + "\"")
    print(command)
    os.system(command)
    print()

simple_assembly_line_balancing_1_data = [
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
simple_assembly_line_balancing_1_main = os.path.join(
        "bazel-bin",
        "examples",
        "simple_assembly_line_balancing_1_main")
for instance, instance_format in simple_assembly_line_balancing_1_data:
    instance_path = os.path.join(
            "data",
            "simple_assembly_line_balancing_1",
            instance)
    json_output_path = os.path.join(
            test_results_directory,
            "simple_assembly_line_balancing_1",
            instance + ".json")
    if not os.path.exists(os.path.dirname(json_output_path)):
        os.makedirs(os.path.dirname(json_output_path))
    command = (
            simple_assembly_line_balancing_1_main
            + "  --verbosity-level 1"
            + "  --input \"" + instance_path + "\""
            + " --format \"" + instance_format + "\""
            + "  --algorithm iterative-beam-search"
            + " --minimum-size-of-the-queue 4096"
            + " --maximum-size-of-the-queue 4096"
            + "  --output \"" + json_output_path + "\"")
    print(command)
    os.system(command)
    print()
