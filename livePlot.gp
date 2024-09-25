set title "Real-time Data Plot"
set xlabel "Time"
set ylabel "Value"
set grid
set autoscale xfixmin
set autoscale yfixmin
set term x11

plot '-' using 1:2 with lines title 'Line1', \
     '-' using 1:2 with lines title 'Line2', \
     '-' using 1:2 with lines title 'Line3'

pause -1  # Keep window open after plot
