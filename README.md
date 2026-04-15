# Multi-Container Runtime

## Team Information

* Chandana Rani S -PES2UG24AM044
* Atharvi Desurkar -PES2UG24AM034
* Team Size: 2 Students

---

## Project Summary

This project implements a lightweight Linux container runtime in C with a long-running supervisor process and a kernel-space memory monitor. The system supports multiple containers, inter-process communication, logging, and memory limit enforcement.

The project consists of two main components:

### 1. User-Space Runtime (engine.c)

* Implements a supervisor process that manages multiple containers
* Supports CLI commands like start, run, ps, logs, and stop
* Uses IPC for communication between client and supervisor
* Handles container lifecycle and metadata tracking

### 2. Kernel-Space Monitor (monitor.c)

* Implemented as a Linux Kernel Module (LKM)
* Tracks container processes using PID
* Enforces soft and hard memory limits
* Uses ioctl for communication with user-space

---

## Environment Setup

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

---

## Build Instructions

### Build all components

```bash
make
```

---

## Run Instructions

### Load kernel module

```bash
sudo insmod monitor.ko
lsmod | grep monitor
```

### Start supervisor

```bash
sudo ./engine supervisor ./rootfs-base
```

### Create container rootfs

```bash
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
```

### Start containers

```bash
sudo ./engine start alpha ./rootfs-alpha /bin/sh
sudo ./engine start beta ./rootfs-beta /bin/sh
```

### List containers

```bash
sudo ./engine ps
```

### View logs

```bash
sudo ./engine logs alpha
```

### Stop container

```bash
sudo ./engine stop alpha
```

### Unload module

```bash
sudo rmmod monitor
```

---

## Test Programs

### memory_hog.c

* Simulates memory pressure by allocating memory periodically
* Used to test soft and hard memory limits

### cpu_hog.c

* Runs an infinite loop to consume CPU
* Used for scheduling experiments

---

## Architecture Overview

* Supervisor acts as a long-running daemon
* CLI acts as a client sending commands
* IPC used:

  * File-based communication (control path)
  * Pipes for logging
* Containers created using `clone()` with namespaces

---

## Features

* Multi-container management
* Namespace isolation (PID, UTS, mount)
* IPC-based communication
* Kernel-level memory monitoring
* Soft and hard limit enforcement
* Logging system using bounded buffer

---

## Engineering Analysis

### Isolation Mechanisms

Containers use namespaces (PID, UTS, mount) and chroot to isolate filesystem and processes.

### Supervisor Design

A long-running supervisor manages lifecycle, tracks metadata, and handles signals.

### IPC and Synchronization

IPC is used for communication between client and supervisor. Proper synchronization avoids race conditions.

### Memory Management

RSS memory is monitored using kernel module. Soft limits trigger warnings, hard limits terminate processes.

### Scheduling

Different workloads demonstrate Linux scheduling behavior using CPU-bound and memory-bound processes.

---

## Design Decisions and Tradeoffs

* File-based IPC chosen for simplicity (tradeoff: slower than sockets)
* Chroot used instead of pivot_root (tradeoff: less secure but easier)
* Simple logging pipeline implemented (tradeoff: limited scalability)

---

## Conclusion

This project demonstrates key OS concepts including process isolation, IPC, kernel-user interaction, and scheduling behavior through a modular container runtime system.
