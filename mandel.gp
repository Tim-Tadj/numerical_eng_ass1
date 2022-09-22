set pm3d map
set format ""
set hidden3d
set terminal pngcairo size 1900,1900 enhanced
set output 'mandel.png'
set size square
set bmargin 0
set lmargin 0
set rmargin 0
set tmargin 0
unset colorbox
set palette model RGB defined (0 'black', 0.5 'blue', 1 'white')
splot "mandel.dat"


# set terminal wxt size 800,800
# set bmargin 0
# set lmargin 0
# set rmargin 0
# set tmargin 0
# unset colorbox
# replot
