# Project Specification: SchedSim

## 1. Introduction and Scope
This document defines the functional and technical specifications for `schedsim`, a discrete-event CPU scheduling simulator developed for CMSC 125. The software models operating system task allocation, providing empirical data on the performance of various scheduling algorithms under defined workloads. The tool is designed to operate as a standalone POSIX-compliant binary, seamlessly integrable with custom Unix shells via standard system calls.

## 2. Functional Requirements

### Command-Line Interface
The application must accept execution arguments to define the scheduling algorithm and the workload source. Supported flags include the algorithm selector, the target workload file, an inline processes string, a time quantum definer for applicable algorithms, and a comparison mode toggle. 

### Workload Parsing
The system must parse workload specifications from either a designated text file or a comma-separated command-line string. File inputs will contain one process per line with comment lines ignored. String inputs will utilize colon-separated attributes for each process. All inputs will define a process identifier, an arrival time, and a total burst time.

### Core Simulation Engine
A time-stepped simulation loop must drive the application. At every discrete time unit, the engine must evaluate process arrivals, update the remaining execution time of the active process, handle task completions, and defer to the selected algorithm module to determine the subsequent active process.

## 3. Algorithm Specifications

### First-Come First-Serve (FCFS)
The system will implement a non-preemptive queue where processes execute strictly in the order of their arrival. Once a process acquires the CPU, it retains control until its remaining time reaches zero.

### Shortest Job First (SJF)
This non-preemptive policy requires the system to evaluate the ready queue and select the process with the shortest total burst time. Similar to FCFS, execution runs to completion without interruption.

### Shortest Time-to-Completion First (STCF)
As the preemptive variant of SJF, the engine must evaluate the ready queue at every time step. If a newly arrived process possesses a shorter remaining execution time than the currently active process, the system must trigger a context switch and reallocate the CPU.

### Round Robin (RR)
This preemptive policy requires a fixed time quantum. The system will allocate CPU time to processes in a first-in, first-out manner. Upon the expiration of a process's quantum, it must be preempted and relegated to the back of the ready queue.

### Multi-Level Feedback Queue (MLFQ)
The MLFQ implementation must operate entirely independent of the burst time variable, deriving process behavior solely from runtime observation. The system will maintain a minimum of three priority queues, each with distinct time quantums and allotment thresholds. Processes exhausting their allotment at a specific priority tier will be demoted. To mitigate starvation, the system must execute a global priority boost at a predefined interval, returning all active processes to the highest priority queue.

## 4. Output and Reporting

### Execution Timeline
The simulator must generate an ASCII Gantt chart visualizing the execution sequence. Each character in the output string will represent a specific time unit or a scaled block of time, denoting which process held the CPU.

### Performance Metrics
Upon completion of a simulation run, the system must calculate and output key performance indicators for every process. Required metrics include turnaround time, waiting time, and response time. The application must also compute the average values for these metrics across the entire workload.

### Comparative Analysis Mode
When initiated with the comparison flag, the simulator will execute the provided workload against all five implemented algorithms sequentially. The output will be a consolidated table displaying the average turnaround time, average waiting time, average response time, and total context switches for each algorithm.

## 5. Technical Constraints

### Execution Environment
The compiled binary must function as an independent child process. It must manage its own standard input and output streams and return standard exit codes to the parent process.

### Memory Management
The application must dynamically allocate memory for process structures and scheduling queues. It is strictly required that all allocated memory is properly freed prior to application termination, ensuring zero memory leaks during operation.

