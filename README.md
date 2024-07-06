# SimpleScheduler: Minimalistic CPU Scheduler for Educational Purposes 🚀

**Overview:**
SimpleScheduler is a lightweight CPU scheduler implemented in C, designed to illustrate fundamental concepts of process scheduling. It serves as an educational tool, allowing users to explore basic CPU scheduling functionalities in a simple shell environment.

## Key Features ✨

### User-Friendly Launch 🖥️
- Initiate SimpleShell to control the execution of user-provided executables.
- Specify the total number of CPU resources (NCPU) and the time quantum (TSLICE) as command-line parameters during launch.

### Job Submission 📝
- Use the `submit` command in SimpleShell to create and execute a new process for the specified executable.
- Processes created do not immediately start execution; they wait for a signal from the SimpleScheduler.

### SimpleScheduler Daemon 👻
- SimpleScheduler operates as a daemon, ensuring minimal CPU resource utilization.
- Maintains a round-robin ready queue for processes, signaling NCPU processes to start execution and managing their time quantum.

### Priority Scheduling (Advanced) ⚡
- Demonstrate priority scheduling by allowing users to specify a priority value (1-4) when submitting a job.
- Default priority is 1 if not specified.
- Users can explore the effect of priority on job scheduling.

## User Executable (jobs) 🔧
- Seamless integration with minimal code adjustments.
- Enables users to experiment with the SimpleScheduler implementation.

**How to Use:**
1. Launch SimpleShell with specified NCPU and TSLICE values.
2. Use the `submit` command to execute user-provided executables.
