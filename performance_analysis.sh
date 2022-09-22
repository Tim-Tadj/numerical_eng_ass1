#!/bin/bash
# You should write a bash script called performance analysis.sh that automatically runs
# the various analyses, collates the results for each parallelisation strategy, prints a numerical
# report in the terminal, and generates plots using the gnuplot program. It should save the
# gnuplot plots into PNG formatted image files for inclusion into your Powerpoint slides.
# You are to use /usr/bin/time to do the timing and memory measurements. Also feel
# free to use any combination of bash, sed and awk to facilitate the analyses (e.g. using bash
# for loops to cycle through parameters to pass to your program, sed and awk to extract
# the relevant numbers, etc.).

# get cli argument for number of iterations if it exists if it doesn't exist, set it to 20000

echo "new script"
if [ -z $1 ]; then
    iterations=20000
else
    iterations=$1
fi

if [ -z $2 ]; then
    x=0
else
    x=$2
fi

if [ -z $3 ]; then
    y=0
else
    y=$3
fi

if [ -z $4 ]; then
    size=4
else
    size=$4
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    timeProgram=/usr/bin/time
    awkProgram=awk
else
    timeProgram=gtime
    awkProgram=gawk
fi

echo $timeProgram

# if [ -z $5 ]; then
#     threads=2
# else
#     threads=$5
# fi

save_plot=false

# run make all print to file and print error if it fails
make clean
echo "Running make all..."
make all > make_all_output.txt 2>&1 || { cat make_all_output.txt ; exit 1; }
echo "Make all successful!"

#make empty file for output

# remove old output file
rm output.txt
echo "Creating output file..."
touch output.txt

# make a list of all program .out 
# all programs compiled are in the format: <program_name>.out
program_list=$(ls *.out)
# for each program in the list run it and plot the mandel.dat file using gnu plot
base_program=mb5.out
echo "Running $base_program with $iterations iterations, x origin = $x, y origin = $y, window size = $size"
$timeProgram ./$base_program $iterations $x $y $size 2>&1 >>  output.txt | cat >> output.txt

gnuplot mandel.gp > /dev/null
# save the plot as a png file
mv mandel.png images/$program.png  

# run parellel version of program
for threads in 2 4 6 8
do
    for program in $program_list
    do
        # get the time elapsed and memory used for each program and print it to the terminal
        # the default mandelbrot does not require a thread count
        if [ $program == "mb5.out" ]; then
        continue
        # given as 's' and 'p' respectively
        elif [ $program == "mb5_fork.out" ]; then
            echo "Running $program with sockets, $iterations iterations and $threads threads, x origin = $x, y origin = $y, window size = $size"
            $timeProgram ./$program $iterations $x $y $size $threads 's' 2>&1 >>  output.txt | cat >> output.txt
            echo "Running $program with pipes, $iterations iterations and $threads threads, x origin = $x, y origin = $y, window size = $size"
            $timeProgram ./$program $iterations $x $y $size $threads 'p' 2>&1 >>  output.txt | cat >> output.txt
        else
            echo "Running $program with $iterations iterations and $threads threads, x origin = $x, y origin = $y, window size = $size"
            $timeProgram ./$program $iterations $x $y $size $threads 2>&1 >>  output.txt | cat >> output.txt
        fi
        
        # plot the mandel.dat file using gnuplot and the mandel.gp file
        if $save_plot == true; then
            gnuplot mandel.gp > /dev/null
            # save the plot as a png file
            mv mandel.png images/$program.png
        fi
    done
done

$awkProgram -f comparison.awk 'output.txt' > mydata.dat
gnuplot mydata.gp

# command to run this script
# sh performance_analysis.sh

# install gnuplot for wsl ubuntu
# sudo apt-get install gnuplot