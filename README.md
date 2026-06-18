# Linux Mini Shell

A Unix-like command-line shell developed in C++ using Linux system calls.

## Features

* Execute Linux commands
* Built-in `cd` command
* Built-in `history` command
* Built-in `exit` command
* Process creation using `fork()`
* Command execution using `execvp()`
* Child process management using `wait()`

## Concepts Used

* Operating Systems
* Linux System Programming
* Process Management
* System Calls
* Parent and Child Processes
* Command Interpretation

## System Calls Used

### fork()

Creates a child process from the parent process.

### execvp()

Replaces the child process image with the requested command.

### wait()

Waits for child process termination and prevents zombie processes.

## Example

```bash
myshell> pwd
/home/harsh

myshell> ls
main.cpp
README.md

myshell> history
1 pwd
2 ls

myshell> exit
```

## How to Build

```bash
g++ main.cpp -o myshell
```

## Run

```bash
./myshell
```

## Learning Outcomes

* Understanding process creation and execution
* Working with Linux system calls
* Implementing command interpreters
* Managing parent and child processes
* Applying Operating Systems concepts in practice
