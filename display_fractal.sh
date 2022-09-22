
make clean > /dev/null
make all > /dev/null

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

if [ -z $5 ]; then
    threads=2
else
    threads=$5
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    openFunc=wslview
elif [[ "$OSTYPE" == "darwin"* ]]; then
    openFunc=open
fi

./mb5_omp.out $iterations $x $y $size $threads 
# Display the fractal

gnuplot mandel.gp
mv mandel.png mandel_omp.png

$openFunc mandel_omp.png