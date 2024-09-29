import matplotlib
import matplotlib.pyplot as plt
import sys
import signal

plotTypes = {};
enabledPlots = {};
class PlotType:
    def __init__(self, plotId, color, lineStyle):
        self.plotId = plotId;
        self.color = color;
        self.lineStyle = lineStyle;

        self.data = {"x": [], "y": [], "line": None};

        global plotTypes
        plotTypes[self.plotId] = self;

PlotType("error", "red", "-");
PlotType("alpha", "gray", "-");
PlotType("noteError", "green", "-");
PlotType("notePrevError", "blue", "-");
PlotType("division", "pink", "-.");

PlotType("speedScale", "blue", "-.");
PlotType("waitScale", "red", "-.");

# global quit flag
quitFlag = False

def signalHandler(sig, frame):
    global quitFlag;
    quitFlag = True;
    print("Exiting plot...");

# attach signal handler for graceful exit
signal.signal(signal.SIGINT, signalHandler);

# custom pause function to prevent window from coming to the front/focus
def mypause(interval):
    # backend = plt.rcParams['backend']
    if matplotlib.backends.backend_registry.list_builtin(matplotlib.backends.BackendFilter.INTERACTIVE):
        figManager = matplotlib._pylab_helpers.Gcf.get_active()
        if figManager is not None:
            canvas = figManager.canvas
            if canvas.figure.stale:
                canvas.draw()
            canvas.start_event_loop(interval)

def updatePlot():
    global quitFlag, plotTypes
    xOffset = 0;
    windowSize = 10;

    plt.ion();  # Interactive mode on
    fig, ax = plt.subplots();

    # set initial axis limits (adjust as needed)
    ax.set_xlim(xOffset, windowSize);   # fixed X-axis from 0 to 100
    ax.set_ylim(0, 5);                  # fixed Y-axis (adjust based on your data range)

    # lock the axis limits to prevent auto-scaling
    ax.set_autoscale_on(False);

    plt.show(block=False);  # ensure the window opens without blocking

    x = 0;
    while not quitFlag:
        line = sys.stdin.readline();  # Read a line from stdin
        # print(f"plot line {line}")
        if(line):
            # print(line, end="");
            if("PLOT" not in line):
                # print(line, end="");
                continue;
            
            if("PLOT_ENABLE" in line):
                print(line);
                return;
                continue;

            if("PLOT_UPDATE" in line):
                # print(line, end="");
                mypause(0.000000000000000001);
                ax.set_xlabel("Time");
                ax.set_ylabel("Value");
                ax.legend();
                # print("update");
                continue;

            # try:
            # parse the line, assuming format: "PLOT: [dataType] <var>"
            _PLOT, dataType, y = line.strip().split();
            # print(_PLOT, dataType, y);
            dataType = dataType.strip("[]");
            dataType = dataType.split("#")[0];

            # print(dataType, x, y);
            if(dataType == "x"):
                x = float(y);
                continue;

            y = float(y);
            # print(x, y);

            # print(dataType in plotTypes)
            if(dataType in plotTypes):
                if(0 < len(enabledPlots) and dataType not in enabledPlots):
                    continue;

                plot = plotTypes[dataType];

                # append new data points
                plot.data["x"].append(x);
                plot.data["y"].append(y);
                
                # print(plot.data["line"] is None);

                # if the line doesn't exist yet, create it
                if(plot.data["line"] is None):
                    plot.data["line"], = ax.plot(
                        plot.data["x"],
                        plot.data["y"],
                        label=dataType,
                        color=plot.color,
                        linestyle=plot.lineStyle
                    );
                else:
                    # only update the data for the existing line
                    plot.data["line"].set_xdata(plot.data["x"]);
                    plot.data["line"].set_ydata(plot.data["y"]);

                if(xOffset + windowSize < x):
                    xLim = ax.get_xlim();
                    if(xLim != xOffset):
                        continue;

                    xOffset += windowSize;
                    ax.set_xlim(xOffset, xOffset + windowSize);  # Fixed X-axis from 0 to 100

                # plot = plotTypes[dataType];
                # recalculate limits based on current data
                # ax.relim()
            else:
                raise ValueError(f"data '{dataType}' type was not defined");
            # except Exception as exception:
            #     print(exception);
            #     pass
        else:
            quitFlag = True;
            print("no line");
            # time.sleep(0.01)

    plt.close(fig);
    print("closing plot");

if __name__ == "__main__":
    updatePlot();
