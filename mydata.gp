set key font ', 40'
set xtics font ', 45'
set ytics font ', 45'
set xlabel font ', 45' offset 0,-1
set ylabel font ', 45' offset -16,0
# Set the legend in front of the plot
# set key opaque
set border back
set lmargin 29
set rmargin 10
set tmargin 6
set bmargin 6

set xlabel 'Threads'
set ylabel 'Time (s)'
set terminal pngcairo size 1900,1900 enhanced
set output 'images/mydatatime.png'

# plot "mydata.dat" using 1:2 with lines title "Time", "mydata.dat" using 1:4 with lines title "Time2"
plot "mydata.dat" using 1:2 title "ForkSocket" with lines lw 10, "" using 1:5 title "ForkPipe" with lines lw 10, "" using 1:8 title "OMP" with lines lw 10, "" using 1:11 title "Pthreads" with lines lw 10

set xlabel 'Threads'
set ylabel 'Memory (kB)'
set terminal pngcairo size 1900,1900 enhanced
set output 'images/mydatamemory.png'
plot "mydata.dat" using 1:3 title "ForkSocket" with lines lw 10, "" using 1:6 title "ForkPipe" with lines lw 10, "" using 1:9 title "OMP" with lines lw 10, "" using 1:12 title "Pthreads" with lines lw 10

set xlabel 'Threads'
set ylabel 'Speedup'
set terminal pngcairo size 1900,1900 enhanced
set output 'images/mydataamdahl.png'
plot "mydata.dat" using 1:4 title "ForkSocket" with lines lw 10, "" using 1:7 title "ForkPipe" with lines lw 10, "" using 1:10 title "OMP" with lines lw 10, "" using 1:13 title "Pthreads" with lines lw 10