#!/usr/bin/awk -f
BEGIN{
    FS = " ";
}

# print the first field of each line
{
    print $0;
}



