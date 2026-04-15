
Container Memory Monitor (Linux Kernel Module)
Overview

The Container Memory Monitor is a Linux Kernel Module (LKM) that monitors the memory usage of user-specified processes. It allows users to define soft and hard memory limits for processes and enforces them in real-time.

 Soft Limit → Logs a warning when exceeded
 Hard Limit → Terminates the process (SIGKILL)

The module communicates with user-space applications using IOCTL system calls.

Features
 Real-time memory monitoring (RSS)
 Kernel thread for continuous tracking
 Soft & hard memory limit enforcement
 Linked list to manage multiple processes
 Thread-safe using mutex locks
 Character device interface (/dev/container_monitor)
 Project Structure
.
├── monitor.c              # Kernel module source code
├── monitor_ioctl.h        # IOCTL definitions and structures
├── Makefile              # Build file for kernel module
└── user.c (optional)     # User-space test program
Requirements
Linux OS (Ubuntu recommended, running in VMware)
Kernel headers installed
GCC and Make tools

Install dependencies:

sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
 Build Instructions

Compile the kernel module:

make

This generates:

monitor.ko
 Load the Module

Insert the module into the kernel:

sudo insmod monitor.ko

Check logs:

dmesg | tail

You will see:

Monitor module loaded. Major: <number>
🔌 Create Device File

Create the character device:

sudo mknod /dev/container_monitor c <major_number> 0
sudo chmod 666 /dev/container_monitor
 Usage (User-Space Program)

Example usage with ioctl():

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "monitor_ioctl.h"

int main() {
    int fd = open("/dev/container_monitor", O_RDWR);

    struct monitor_request req;
    req.pid = getpid();
    req.soft_limit = 50 * 1024 * 1024;  // 50MB
    req.hard_limit = 100 * 1024 * 1024; // 100MB

    ioctl(fd, IOCTL_REGISTER, &req);

    while(1);
    return 0;
}

Compile and run:

gcc user.c -o user
./user
 How It Works
User registers a process via ioctl()
Kernel module stores process info in a linked list
A kernel thread runs every second
It checks memory usage (RSS) of each process
Actions taken:
If memory > soft limit → Warning logged
If memory > hard limit → Process killed
 Important Concepts
IOCTL → Communication between user space and kernel
Kernel Thread → Background monitoring
RSS (Resident Set Size) → Actual memory used
Mutex Locking → Prevents race conditions
Signal Handling → SIGKILL for termination
Unload the Module

Remove the module:

sudo rmmod monitor
 Troubleshooting
Issue	Solution
invalid module format	Kernel headers mismatch
Permission denied	Use sudo
No logs	Check dmesg
ioctl not working	Ensure device file exists
 Applications
Container memory control
OS resource management
System monitoring tools
Kernel programming practice
Limitations
Uses polling (1-second interval)
Hard kill may cause abrupt termination
No advanced scheduling or prioritization
