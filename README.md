# Multi-Container Runtime

## Team Information

* Chandana Rani S - PES2UG24AM044
* Atharvi Desurkar - PES2UG24AM034
* Team Size: 2 Students

---

# Project Summary

This project implements a lightweight Linux container runtime in C with a long-running supervisor process and a kernel-space memory monitor. The system supports multiple containers, inter-process communication, logging, and memory limit enforcement.

The project consists of two main components:

---

## 1. User-Space Runtime (engine.c)

* Implements a supervisor process that manages multiple containers
* Supports CLI commands like start, run, ps, logs, and stop
* Uses IPC for communication between client and supervisor
* Handles container lifecycle and metadata tracking

---

## 2. Kernel-Space Monitor (monitor.c)

* Implemented as a Linux Kernel Module (LKM)
* Tracks container processes using PID
* Enforces soft and hard memory limits
* Uses ioctl for communication with user-space

---

# Environment Setup

```bash id="kxnbel"
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

---

# Build Instructions

```bash id="xk9f2u"
make
```

---

# Run Instructions

### Load kernel module

```bash id="ksg9bp"
sudo insmod monitor.ko
lsmod | grep monitor
```

### Start supervisor

```bash id="o8kncc"
sudo ./engine supervisor ./rootfs-base
```

### Create container rootfs

```bash id="kribie"
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
```

### Start containers

```bash id="b3xqcv"
sudo ./engine start alpha ./rootfs-alpha /bin/sh
sudo ./engine start beta ./rootfs-beta /bin/sh
```

### List containers

```bash id="e5vdf9"
sudo ./engine ps
```

### View logs

```bash id="yqj1kq"
sudo ./engine logs alpha
```

### Stop container

```bash id="k3qkpo"
sudo ./engine stop alpha
```

### Unload module

```bash id="d2u4ra"
sudo rmmod monitor
```

---

# Test Programs

### memory_hog.c

* Simulates memory pressure by allocating memory periodically
* Used to test soft and hard memory limits

### cpu_hog.c

* Runs an infinite loop to consume CPU
* Used for scheduling experiments

---

# Architecture Overview

* Supervisor acts as a long-running daemon
* CLI acts as a client sending commands
* IPC used:

  * File-based communication (control path)
  * Pipes for logging
* Containers created using `clone()` with namespaces

---

# Features

* Multi-container management
* Namespace isolation (PID, UTS, mount)
* IPC-based communication
* Kernel-level memory monitoring
* Soft and hard limit enforcement
* Logging system using bounded buffer

---

# IPC, Synchronization, and Bounded Buffer

## Synchronization Mechanisms Used

* **Mutex locks** are used to protect shared data structures such as:

  * Container metadata list
  * Logging buffer

* **Condition variables** (or signaling mechanism) are used in the logging system to coordinate:

  * Producer threads (writing logs)
  * Consumer threads (writing logs to files)

---

## Why Mutex is Used

* Prevents multiple threads from modifying shared data simultaneously
* Ensures **thread-safe access** to shared memory
* Avoids data corruption in logging and metadata tracking

---

## Race Conditions Without Synchronization

* Two threads writing logs at the same time → **corrupted output**
* Buffer overflow when multiple producers write → **data loss**
* Consumer reading incomplete data → **inconsistent logs**
* Metadata updates overlapping → **incorrect container state**

---

## Bounded Buffer Design

* A **fixed-size buffer** is used between producer and consumer threads

* Producers:

  * Read container output (stdout/stderr)
  * Insert data into buffer

* Consumers:

  * Remove data from buffer
  * Write to log files

---

## How Bounded Buffer Prevents Issues

* **No data loss**: producers wait when buffer is full
* **No corruption**: mutex ensures only one thread modifies buffer at a time
* **No deadlock**: proper signaling ensures threads wake correctly
* **Ordered logging**: maintains correct sequence of logs
* **Graceful shutdown**: consumer flushes remaining data before exit

---

# Engineering Analysis

## Isolation Mechanisms

Containers use namespaces (PID, UTS, mount) and chroot to isolate filesystem and processes. Each container has its own environment but shares the host kernel.

---

## Supervisor Design

A long-running supervisor manages lifecycle, tracks metadata, and handles signals. It ensures proper process creation, monitoring, and cleanup.

---

## IPC and Synchronization

IPC is used for communication between client and supervisor, while pipes are used for logging. Mutex locks and synchronization primitives prevent race conditions and ensure consistency.

---

## Memory Management

RSS memory is monitored using kernel module. Soft limits trigger warnings, while hard limits enforce termination. Kernel-level enforcement ensures accurate and reliable control.

---

## Scheduling

Different workloads demonstrate Linux scheduling behavior using CPU-bound and memory-bound processes. The scheduler allocates CPU time based on priority and workload characteristics.

---

# Design Decisions and Tradeoffs

* File-based IPC chosen for simplicity (tradeoff: slower than sockets)
* Chroot used instead of pivot_root (tradeoff: less secure but easier)
* Bounded buffer logging implemented (tradeoff: increased complexity but ensures correctness)

---

# Conclusion

This project demonstrates key OS concepts including process isolation, IPC, synchronization, kernel-user interaction, memory enforcement, and scheduling behavior through a modular container runtime system.

---

# Final Statement

> This implementation acts as a lightweight version of Docker, combining user-space container management with kernel-level resource control.
