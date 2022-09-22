# Documenting commands

## to measure time elapsed 
```console
/usr/bin/time ./mb5_pthr.out 20000 2>&1 >/dev/null | tail -2 | head -1 | awk -F ' ' '{print $3}'
0:07.27elapsed
```

## removed code from mandel.gp

```c
set terminal wxt size 800,800
set bmargin 0
set lmargin 0
set rmargin 0
set tmargin 0
unset colorbox
replot
```