set title 'Real-time Data Plot';
set xlabel 'Time';
set ylabel 'Value';
set grid;
set autoscale xfixmin;
set autoscale yfixmin;

# pause 1

plot '.plot.dat' using 1:2 with lines title 'error', \
    '.plot.dat' using 1:3 with lines title 'noteError', \
    '.plot.dat' using 1:4 with lines title 'alpha', \
    '.plot.dat' using 1:5 with lines title 'division';

while (1) {
    replot;
    pause 0.001;
}
