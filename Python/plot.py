import csv
import matplotlib.pyplot as plt

def plot_stats():
    x = []
    y = []
    with open("losses1.txt") as f:
        plots = csv.reader(f, delimiter=",")
        for row in plots:
            x.append(float(row[0]))
            y.append(float(row[1].rstrip()))

    plt.plot(x, y)
    plt.show()

    x = []
    y = []
    with open("rewards1.txt") as f:
        plots = csv.reader(f, delimiter=",")
        for row in plots:
            x.append(float(row[0]))
            y.append(float(row[1].rstrip()))

    plt.plot(x, y)
    plt.show()

plot_stats()
