import time
import subprocess
import statistics
import os

N = 10

# https://stackoverflow.com/a/2474508
# https://stackoverflow.com/a/1557584
def time_one_program(name):
    args = (f"bin/{name}")
    start_time = time.time()
    
    subprocess.run(args, capture_output=True)
    
    end_time = time.time()
    return (end_time - start_time)


def time_multiple_runs(name):
    vals = []
    for i in range(N):
        vals.append(time_one_program(name))
    
    return statistics.mean(vals), statistics.stdev(vals)


# https://stackoverflow.com/a/3207973
def time_all():
    for fname in sorted(os.listdir("bin")):
        m, s = time_multiple_runs(fname)
        print(f"{fname}:\t\t mean {m} \t\t std {s}")

if __name__ == "__main__":
    time_all()


