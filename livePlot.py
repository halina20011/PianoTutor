# python-matplotlib
# import matplotlib.pylot as plt
# import sys
#
# def update():
#     plt.ion();
#     fig, ax = plt.subplots();
#
#     xData, yData = [], [];
#
#     while True:
#         line = sys.stdin.readline();
#         if(line):
#             _, x, y = line.strip().split();
#
#             ax.clear();
#             ax.plot(xData, yData, label="plot");

import matplotlib
import matplotlib.pyplot as plt
import sys
import time
import signal

ERROR_TYPE = "error";
ALPHA_TYPE = "alpha";
NOTE_ERROR_TYPE = "noteError";
DIVISION_TYPE = "division";

# Define colors and line styles for different data types
colors = {
    ERROR_TYPE: "red",
    ALPHA_TYPE: "gray",
    NOTE_ERROR_TYPE: "green",
    DIVISION_TYPE: "pink"
}

lineStyles = {
    ERROR_TYPE: "-",
    # "dataType2": "--",
    ALPHA_TYPE: "-.",
    NOTE_ERROR_TYPE: "-",
    DIVISION_TYPE: "-."
}

# Global quit flag
quit_flag = False

def signalHandler(sig, frame):
    global quit_flag
    quit_flag = True
    print("Exiting plot...")

# attach signal handler for graceful exit
signal.signal(signal.SIGINT, signalHandler)

# custom pause function to prevent window from coming to the front
def mypause(interval):
    # backend = plt.rcParams['backend']
    if matplotlib.backends.backend_registry.list_builtin(matplotlib.backends.BackendFilter.INTERACTIVE):
        figManager = matplotlib._pylab_helpers.Gcf.get_active()
        if figManager is not None:
            canvas = figManager.canvas
            if canvas.figure.stale:
                canvas.draw()
            canvas.start_event_loop(interval)

def update_plot():
    global quit_flag
    xOffset = 0;
    windowSize = 10

    plt.ion()  # Interactive mode on
    fig, ax = plt.subplots()

    # Set initial axis limits (adjust as needed)
    ax.set_xlim(xOffset, windowSize)  # Fixed X-axis from 0 to 100
    ax.set_ylim(0, 5)   # Fixed Y-axis (adjust based on your data range)

    # Lock the axis limits to prevent auto-scaling
    ax.set_autoscale_on(False)

    # Data containers for different types
    data = {
        ERROR_TYPE: {"x": [], "y": [], "line": None},
        ALPHA_TYPE: {"x": [], "y": [], "line": None},
        NOTE_ERROR_TYPE: {"x": [], "y": [], "line": None},
        DIVISION_TYPE: {"x": [], "y": [], "line": None},
        # "dataType2": {"x": [], "y": [], "line": None},
        # "dataType3": {"x": [], "y": [], "line": None}
    }

    plt.show(block=False)  # Ensure the window opens without blocking

    x = 0;

    while not quit_flag:
        line = sys.stdin.readline()  # Read a line from stdin
        # print(f"plot line {line}")
        if(line):
            if("PLOT" not in line):
                # print(line, end="");
                continue;

            try:
                # Parse the line, assuming format: "PLOT: [dataType] <var>"
                _PLOT, dataType, y = line.strip().split()
                # print(_PLOT, dataType, y);
                dataType = dataType.strip("[]");
                dataType = dataType.split("#")[0];

                # print(dataType, x, y);
                if(dataType == "x"):
                    x = float(y);
                    continue;

                # x = float(y);
                y = float(y)
                # print(x, y);

                if(dataType in data):
                    # Append new data points
                    data[dataType]["x"].append(x)
                    data[dataType]["y"].append(y)

                    # If the line doesn't exist yet, create it
                    if data[dataType]["line"] is None:
                        data[dataType]["line"], = ax.plot(
                            data[dataType]["x"],
                            data[dataType]["y"],
                            label=dataType,
                            color=colors[dataType],
                            linestyle=lineStyles[dataType]
                        )
                    else:
                        # Only update the data for the existing line
                        data[dataType]["line"].set_xdata(data[dataType]["x"])
                        data[dataType]["line"].set_ydata(data[dataType]["y"])

                    # ax.set_xlabel("Time")
                    # ax.set_ylabel("Value")
                    # ax.legend()

                    if x > xOffset + windowSize:
                        xOffset += windowSize;
                        ax.set_xlim(xOffset, xOffset + windowSize)  # Fixed X-axis from 0 to 100

                    # recalculate limits based on current data
                    # ax.relim()
                    mypause(0.000000000000000001)

            except Exception as exception:
                print(exception);
                pass
        else:
            quit_flag = True;
            print("no line");
            time.sleep(0.01)

    plt.close(fig)
    print("Plot closed.")

if __name__ == "__main__":
    update_plot()
