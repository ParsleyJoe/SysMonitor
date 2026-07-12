# Sys Monitor

Linux system monitor made in C. Calcutates data with files from /proc, calculations in reference to htop cpu usage calculation code.

## Building

Dependencies:
    C standard library(glibc on arch)
    ncurses

```
git clone https://github.com/ParsleyJoe/SysMonitor.git
cd SysMonitor
make
./monitor
```

## Usage
Press Q to quit.
Pressing CTRL+C also works fine, as it is handled gracefully
