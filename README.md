Container Memory Monitor
 Overview

A Linux Kernel Module that monitors process memory usage and enforces limits:

 Soft limit → Warning
 Hard limit → Process killed (SIGKILL)

Uses IOCTL for communication between user space and kernel.

 Files
monitor.c → Kernel module
monitor_ioctl.h → IOCTL definitions
Makefile → Build file
user.c → Test program (optional)
⚙️ Build & Run
make
sudo insmod monitor.ko
dmesg | tail

Create device:

sudo mknod /dev/container_monitor c <major_number> 0
sudo chmod 666 /dev/container_monitor
 Usage
Register process using ioctl()
Module monitors memory every second
Actions:
Soft limit exceeded → Warning log
Hard limit exceeded → Process terminated
 Remove Module
sudo rmmod monitor
 Concepts Used
Kernel Module
IOCTL
Kernel Thread
Linked List
Mutex Synchronization
 Purpose

Demonstrates kernel-level process monitoring and memory control.
