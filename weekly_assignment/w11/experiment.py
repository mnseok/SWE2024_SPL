import subprocess
import time
import matplotlib.pyplot as plt
import re

# Compile the C program
def compile_program(n_data, n_threads):
    with open("monte_carlo_multi_thread.c", "r") as file:
        source = file.read()

    # Update N_DATA and N_THREAD in the source code using regular expressions
    source = re.sub(r"#define\s+N_DATA\s+\d+", f"#define N_DATA {n_data}", source)
    source = re.sub(r"#define\s+N_THREAD\s+\d+", f"#define N_THREAD {n_threads}", source)
    source = re.sub(
        r"#define\s+N_DATA_PER_THREAD\s+\(.*\)",
        "#define N_DATA_PER_THREAD (N_DATA / N_THREAD)",
        source,
    )

    with open("monte_carlo_multi_thread.c", "w") as file:
        file.write(source)

    # Compile the program
    subprocess.run(["gcc", "-o", "multi", "monte_carlo_multi_thread.c", "-lpthread"])

# Run the compiled program and measure execution time
def run_program():
    start_time = time.time()
    subprocess.run(["./multi"], stdout=subprocess.PIPE)
    end_time = time.time()
    return (end_time - start_time) * 1000  # Execution time in milliseconds

# Experiment configurations
n_data_list = [100000, 500000, 1000000, 5000000]
n_threads_list = [1, 2, 4, 8]

results = {}

# Number of times each experiment is run
num_runs = 100

# Run experiments
for n_data in n_data_list:
    results[n_data] = []
    for n_threads in n_threads_list:
        print(f"Running for N_DATA={n_data}, N_THREAD={n_threads}...")
        compile_program(n_data, n_threads)
        exec_times = []
        for _ in range(num_runs):
            exec_time = run_program()
            exec_times.append(exec_time)
        avg_time = sum(exec_times) / num_runs
        results[n_data].append(avg_time)
        print(f"Average Execution Time: {avg_time:.2f} ms")

# Plot all results in a single figure
plt.figure(figsize=(12, 8))
rows = len(n_data_list) // 2 + len(n_data_list) % 2  # 2 columns per row

for idx, (n_data, times) in enumerate(results.items()):
    plt.subplot(rows, 2, idx + 1)
    plt.plot(n_threads_list, times, marker="o")
    plt.title(f"N_DATA={n_data}")
    plt.xlabel("Number of Threads")
    plt.ylabel("Execution Time (ms)")
    plt.xticks(n_threads_list)
    plt.grid()

plt.tight_layout()
plt.savefig("average_execution_time_subplots.png")
plt.show()
