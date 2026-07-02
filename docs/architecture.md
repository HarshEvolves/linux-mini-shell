# Linux Mini Shell Architecture

This document outlines the high-level architecture and design philosophy of the Linux Mini Shell project.

## Architecture Diagram

The shell uses a pipeline-driven, modular architecture where responsibilities are strictly separated across dedicated C modules.

```text
                             +-------------------+
                             |      main.c       |
                             +---------+---------+
                                       |
                             +---------v---------+
                             |      shell.c      |
                             | (REPL & Signals)  |
                             +---------+---------+
                                       |
               +-----------------------+-----------------------+
               |                       |                       |
     +---------v---------+   +---------v---------+   +---------v---------+
     |     parser.c      |   |     builtin.c     |   |      pipe.c       |
     | (Tokenization)    |   | (cd, exit, hist)  |   | (N-1 Pipeline)    |
     +---------+---------+   +---------+---------+   +---------+---------+
               |                       |                       |
     +---------v---------+   +---------v---------+   +---------v---------+
     |     command.c     |   |     history.c     |   |     execute.c     |
     | (Data Structure)  |   | (Ring Buffer)     |   | (fork, execvp)    |
     +-------------------+   +-------------------+   +---------+---------+
                                                               |
                                                     +---------v---------+
                                                     |    redirect.c     |
                                                     | (dup2, open, I/O) |
                                                     +-------------------+
```

## Module Descriptions

### Core Loop
* **`main`**: The program entry point. It simply invokes the shell initialization and kicks off the main loop.
* **`shell`**: The core Read-Eval-Print Loop (REPL). It manages user input via `getline()`, detects EOF (Ctrl+D), checks for background execution (`&`), and routes the execution flow to either `builtin` or the `execute`/`pipe` modules. 

### Parsing and Data
* **`parser`**: Responsible for converting raw string input into structured data. It handles string tokenization, detects pipeline characters (`|`), and extracts I/O redirection tokens (`<`, `>`, `>>`).
* **`command`**: Defines the central Data Transfer Object (DTO) of the shell—the `Command` struct. It holds the argument vector (`argv`) and pointers to input/output redirection files, providing a unified structure passed between the parser and execution layers.

### Execution Engine
* **`execute`**: Handles the execution of a single external command. It uses `fork()` to create a child process, invokes `redirect` to wire up file descriptors, resets signals, and calls `execvp()`. The parent process blocks via `waitpid()` unless it is a background job.
* **`pipe`**: Orchestrates multi-command pipelines. For `N` commands, it creates `N-1` pipes, forks `N` child processes, wires standard I/O streams across processes sequentially, and ensures all file descriptors are strictly managed and closed to prevent leaks.
* **`redirect`**: Encapsulates file descriptor manipulation. It safely uses `open()`, `close()`, and `dup2()` to map standard input and standard output to specified files.

### Shell Internal Features
* **`builtin`**: Identifies and executes commands that must modify the shell's own process state (e.g., `cd` to change directories, `exit` to terminate).
* **`history`**: Maintains an in-memory history of the user's previously executed commands using an array-based structure, safely managing string allocation and cleanup.

### System & Lifecycle Management
* **`signal`**: Centralizes kernel signal handling. It handles `SIGINT` (Ctrl+C) by catching `EINTR` to safely interrupt input and present a fresh prompt. It also handles `SIGCHLD` to asynchronously reap background "zombie" processes without blocking the main loop.
* **`jobs`**: *(Note: Historical background job tracking logic has been tightly integrated and folded into the `signal` module for cohesive lifecycle management).*
* **`utils`**: *(Note: Utility functions are kept static and local to their respective modules rather than dumped in a global util file, ensuring strict encapsulation).*

## Why Modular Architecture?

The project is heavily modularized for several critical reasons:

1. **Separation of Concerns**: Each C file does exactly one thing. The parser doesn't know about `fork()`, and the execution engine doesn't care about string parsing.
2. **Maintainability**: Files remain small, focused, and easy to read. Bugs relating to file descriptors are isolated to `redirect.c` and `pipe.c`.
3. **Extensibility**: Adding a new feature, such as unlimited pipelines, only required updates to `pipe.c` and a minor routing check in `shell.c`. The `parser.c` and `command.c` modules scaled effortlessly to support arrays of commands.
4. **Safety**: Signal handling and process lifecycle management are notoriously tricky in C. Isolating this logic inside `signals.c` and `execute.c` ensures that `waitpid` retries and `dup2` close sequences are executed correctly without tangling with the REPL loop.
