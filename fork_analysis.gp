set xlabel "Threads"
set ylabel "Time (s)"
set output "fork_analysis.png"
plot "fork_analysis.dat" using 1:2 title "Sockets" with lines, "" using 1:3 title "Pipes" with lines