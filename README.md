# Sys Monitor
Linux system monitor made in C. Calcutates data with files from /proc, calculations in reference to htop cpu usage calculation code.

https://github.com/user-attachments/assets/c4752a8d-d39a-4e25-892d-f6baae9d8126

### Note: The transparency is from the terminal(kitty) this program will not make your terminal transparent

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
- Press Q to quit.
- Pressing CTRL+C also works fine, as it is handled gracefully
- Up and Down arrows to scroll processes
- Left and right arrow to change selected column (processes are sorted in descending order based on selected column)
