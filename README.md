# Sys Monitor
Linux system monitor made in C. Calculates data from files in /proc, with calculations referenced from htop CPU usage calculation code.

https://github.com/user-attachments/assets/c4752a8d-d39a-4e25-892d-f6baae9d8126

### Note: The transparency is from the terminal(kitty) this program will not make your terminal transparent

## Building

Dependencies:
 - C standard library(glibc on Arch)
 - ncurses

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

## Modifying
- The worker thread and the UI thread are separated in the main.c file
- The worker thread handles all /proc CPU and MEM usage Calculations(checkout worker_thread() function in main.c)
- The UI thread handles key inputs and drawing; for simplicity, the main thread is the UI thread.
