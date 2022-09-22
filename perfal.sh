# You should write a bash script called performance analysis.sh that automatically runs
# the various analyses, collates the results for each parallelisation strategy, prints a numerical
# report in the terminal, and generates plots using the gnuplot program. It should save the
# gnuplot plots into PNG formatted image files for inclusion into your Powerpoint slides.
# You are to use /usr/bin/time to do the timing and memory measurements. Also feel
# free to use any combination of bash, sed and awk to facilitate the analyses (e.g. using bash
# for loops to cycle through parameters to pass to your program, sed and awk to extract
# the relevant numbers, etc.).

# get cli argument for number of iterations if it exists if it doesn't exist, set it to 20000
if [ -z "$1" ]; then
    iterations=20000
else
    iterations=$1
fi

# run make all print to file and print error if it fails
echo "Running make all..."
make all > make_all_output.txt 2>&1 || { cat make_all_output.txt ; exit 1; }
echo "Make all successful!"


# make a list of all program .out 
# all programs compiled are in the format: <program_name>.out
program_list=$(ls mb5*)
# for each program in the list run it and plot the mandel.dat file using gnu plot
for program in $program_list
do
    # print the program name
    echo $program

    # get the time elapsed and memory used for each program and print it to the terminal
    /usr/bin/time -f "Time elapsed: %e seconds, Memory used: %M kilobytes" ./$program $iterations > /dev/null
    
    # plot the mandel.dat file using gnuplot and the mandel.gp file
    gnuplot mandel.gp > /dev/null
    # save the plot as a png file
    mv mandel.png $program.png 

    
done

# command to run this script
# ./performance_analysis.sh

# install gnuplot for wsl ubuntu
# sudo apt-get install gnuplot