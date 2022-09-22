BEGIN{
    FS = " ";
    # FNR>3;
    time = 0;
    n_comparisons = 4; #how many thread runs to compare
    n_programs = 4; #how many programs to compare

    i=0;
    #get number of lines in file
    while (getline < "output.txt") {
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
    # init memory array
    for (i = 0; i < n_lines; i++) {
        memory_arr[i] = 0;
    }
    i=0;
    default_time = 0;
    default_elapsed = 0;
}

/Default/{
    str =$2
    # split and stor time time:151.415167
    split(str, arr, ":");
    default_time = arr[2];
    # print "default_time: " default_time;
}

# print the first field of each line
/threads/{
    if (NR>3)
    {
        str = $2;
        split(str, a, ":");
        threads_arr[i] = a[2];
    }
        
        # print threads_arr[i];
}

/elapsed/{
    if (NR>3)
    {
        str = $3
        #extract the numbers from the string using match 2:34.44elapsed
        match(str, /([0-9]+):([0-9]+).([0-9]+)/, a);
        # print a[1], a[2], a[3];
        time = a[1]*60 + a[2] + a[3]/100;
        time_arr[i] = time;
        # print time_arr[i];

        str = $6
        #extract the numbers from the string using match 2:34.44elapsed
        match(str, /([0-9]+)/, a);
        memory_arr[i] = a[1];
        # print memory_arr[i];

        i++;
    }

    if(NR==2)
    {
        str = $3
        #extract the numbers from the string using match 2:34.44elapsed
        match(str, /([0-9]+):([0-9]+).([0-9]+)/, a);
        # print a[1], a[2], a[3];
        default_elapsed = a[1]*60 + a[2] + a[3]/100;
        # print "default_elapsed: " default_elapsed;
    }
    

}

END{
    #thr_num forksocket forkpipe omp pthreads
    # j=0;
    for (i = 0; i < n_comparisons; i++) {
        printf "%i ", threads_arr[i*n_comparisons];
        for(j = 0; j < n_programs; j++) {
            printf "%f ", time_arr[i*n_comparisons + j];
            printf "%i ", memory_arr[i*n_comparisons + j];
            Fenhanced = default_elapsed/default_time;
            Senhanced = default_time/time_arr[i*n_comparisons + j];
            printf "%f ", (1/((1-Fenhanced) + (Fenhanced/Senhanced)));
        }
        printf "\n";
    }
}