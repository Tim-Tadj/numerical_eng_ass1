BEGIN{
    FS = " ";
    time = 0;
    n_comparisons = 4;

    i=0;
    #get number of lines in file
    while (getline < "fork_comparison.txt") {
        i++;
    }
    n_lines = i/n_comparisons;
    for (i = 0; i < n_lines; i++) {
        time_arr[i] = 0;
    }
    # init threads array
    for (i = 0; i < n_lines; i++) {
        threads_arr[i] = 0;
    }
    i=0;
}

# print the first field of each line
/threads/{
    split($3, a, ":");
    threads_arr[i] = a[2];
}

/time/{
    split($4, a, ":");
    time_arr[i] = a[2];
    i++;
}

END{
    for (i = 0; i < n_lines/n_comparisons; i++) {
        print threads_arr[i] " " time_arr[i] " " time_arr[i+n_lines/n_comparisons] " " time_arr[i+2*n_lines/n_comparisons] " " time_arr[i+3*n_lines/n_comparisons];
    }
}