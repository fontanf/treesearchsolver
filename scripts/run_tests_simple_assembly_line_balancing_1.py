import run_tests
import os


commands = run_tests.generate_commands(
        os.path.join(
            "bazel-bin",
            "examples",
            "simple_assembly_line_balancing_1_main")
        + " --algorithm iterative-beam-search"
        + " --minimum-size-of-the-queue 1024"
        + " --maximum-size-of-the-queue 1024",
        os.path.join(
            "..",
            "ordata",
            "scheduling",
            "simple_assembly_line_balancing_1",
            "data.csv"),
        os.path.join("test_results", "simple_assembly_line_balancing_1"),
        "row['Dataset'] in ['otto2013_small', 'otto2013_mediium', 'otto2013_large']")


if __name__ == "__main__":
    for command in commands:
        run_tests.run(command)
