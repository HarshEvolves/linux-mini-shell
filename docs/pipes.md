# Unix Pipes and IPC in the Shell

This document explains the theory and implementation of Unix pipelines within the Linux Mini Shell.

## 1. What is a Unix Pipe?
A Unix pipe is a unidirectional, inter-process communication (IPC) channel provided by the kernel. It has two ends:
* **Read End (`fd[0]`)**: Where data is pulled out.
* **Write End (`fd[1]`)**: Where data is pushed in.

When multiple commands are chained together (e.g., `cmd1 | cmd2`), the standard output (`stdout`) of `cmd1` is attached to the write end of a pipe, and the standard input (`stdin`) of `cmd2` is attached to the read end of that same pipe.

---

## 2. System Calls Used

The shell orchestrates pipelines using the following kernel system calls:

* **`pipe(int pipefd[2])`**: Asks the kernel to create a pipe. It populates the array with two file descriptors: `pipefd[0]` (read) and `pipefd[1]` (write).
* **`fork()`**: Creates an identical child process. The child inherits a copy of all the parent's open file descriptors, giving both processes access to the pipe.
* **`dup2(int oldfd, int newfd)`**: Duplicates `oldfd` onto `newfd`. To wire a pipe to standard output, we use `dup2(pipefd[1], STDOUT_FILENO)`.
* **`close(int fd)`**: Closes a file descriptor.
* **`execvp(const char *file, char *const argv[])`**: Replaces the child process with a new program (like `ls` or `wc`). Because file descriptors are preserved across `execvp()`, the new program automatically reads from/writes to the pipe.

---

## 3. The "Unused File Descriptor" Trap

**Why must we diligently close unused pipe ends?**
Programs like `cat` or `wc` read from standard input until they detect an **EOF (End of File)**. The kernel only sends an EOF to the read end of a pipe when **all copies** of the write end are closed across all processes (parent and children).

If the shell (the parent) forgets to `close(pipefd[1])`, the kernel thinks the shell might still write to the pipe. The child reading from the pipe will hang forever waiting for data, causing the pipeline to deadlock.

**Rule:** After `fork()` and `dup2()`, close every single pipe file descriptor in both the parent and the children.

---

## 4. Pipeline Implementation & Diagrams

### Single Pipeline (2 Commands)
For a command like `ls | wc`:
1. The shell creates **1 pipe**.
2. It forks **Child 1** (`ls`). Child 1 uses `dup2` to point `stdout` to the pipe's write end.
3. It forks **Child 2** (`wc`). Child 2 uses `dup2` to point `stdin` to the pipe's read end.
4. The parent closes both pipe ends and waits for both children to exit.

```text
    [ Parent Shell ] 
   (Closes both ends)
           |
      +----+----+
      |         |
[ Child 1 ]   [ Child 2 ]
   (ls)          (wc)
    |             ^
    v             |
 [ Write ]     [ Read ]
 +--------------------+
 |    Kernel Pipe     |
 +--------------------+
```

### Unlimited Pipelines (N Commands)
For a command like `cat file.txt | grep "error" | wc -l`:
The shell must scale dynamically. For **N commands**, it creates **N-1 pipes**.

**Algorithm:**
1. Generate `N-1` pipes in an array.
2. Loop `N` times, forking one child per command.
3. Inside the child (Command `i`):
   * If `i > 0` (Not the first command): `dup2` the previous pipe's read end to `stdin`.
   * If `i < N - 1` (Not the last command): `dup2` the current pipe's write end to `stdout`.
   * **Close all N-1 pipes in the child**.
   * Execute the program.
4. Inside the parent:
   * **Close all N-1 pipes in the parent**.
   * Wait for all `N` children to finish.

```text
[ cat ]           [ grep ]           [ wc ]
   |                 ^  |               ^
   |                 |  |               |
   v                 |  v               |
 +---+-------------+---+ +---+-------------+---+
 |Write           Read | |Write           Read |
 |       Pipe 0        | |       Pipe 1        |
 +---------------------+ +---------------------+
```

By strictly managing loops and closing all inherited pipe file descriptors in every process, the mini shell safely and efficiently handles any length of pipeline without leaking resources or deadlocking.
