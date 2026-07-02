# Design Decisions & Trade-offs

This document explains the rationale behind the major architectural decisions made during the development of the Linux Mini Shell.

## 1. Why a Modular Design?
Initially, shells often start as a massive `main.c` file containing parsing, execution, and signal logic all tangled together (a "God object" anti-pattern). 
We intentionally forced a highly modular design to achieve:
* **Separation of Concerns:** The parser has zero knowledge of `fork()`. The execution engine has zero knowledge of `strtok()`.
* **Maintainability:** Every source file remains tightly focused and under a few hundred lines.
* **Debugging:** If a pipeline hangs, the bug is guaranteed to be in `pipe.c` or `redirect.c`, not scattered across a messy `shell_loop()`.

## 2. Why introduce the `Command` abstraction?
In early iterations, the parser simply returned a `char **argv` array. However, as features like I/O redirection were added, returning just `argv` was insufficient. 

We introduced the `Command` struct to act as a **Data Transfer Object (DTO)**:
```c
typedef struct {
    char *argv[MAX_ARGS];
    char *input_file;
    char *output_file;
    int append;
} Command;
```
**Benefits:**
* **Clean API:** The parser neatly packs this struct, and the execution/pipe modules simply consume it. 
* **Scalability:** When we added unlimited pipelines, the parser simply generated an *array* of `Command` objects, requiring almost zero changes to the underlying data model.

## 3. Why separate Parser, Execute, Redirect, and Pipe?
* **Parser (`parser.c`)**: String manipulation is inherently messy in C. Confining `strtok()` and string matching here protects the rest of the codebase from memory/pointer arithmetic errors.
* **Redirect (`redirect.c`)**: Manages `open()`, `close()`, and `dup2()`. By abstracting this, `execute.c` simply calls `apply_redirection(cmd)` without caring if it's input, output, or append redirection.
* **Execute (`execute.c`)**: Focuses entirely on the `fork()` -> `execvp()` -> `waitpid()` lifecycle for a single command.
* **Pipe (`pipe.c`)**: Managing `N-1` pipes and wiring standard streams across `N` children is complex and state-heavy. Isolating this loop prevents the simple `execute.c` logic from becoming unreadable.

## 4. Trade-offs Made
* **In-Place Parsing (Mutating the Buffer):** The parser uses `strtok()`, which inserts `\0` directly into the raw input buffer, and `argv` points to these substrings.
  * *Pros:* Extremely fast, zero dynamic memory allocations (`malloc`/`free`) per argument, preventing memory leaks.
  * *Cons:* The original input string is permanently destroyed. (We work around this by `strdup`-ing a copy exclusively for the `history` module before parsing).
* **Signal Handlers vs. Process Groups:** Currently, a foreground `Ctrl+C` sends `SIGINT` to the shell *and* the child process. The shell catches it and survives, while the child dies.
  * *Trade-off:* Standard shells put children in their own process group using `setpgid()` and give them terminal control via `tcsetpgrp()`. We bypassed this to keep the OS concepts focused purely on signals (`sigaction`) without the immense overhead of managing terminal sessions.

## 5. Future Improvements
* **Advanced Parsing:** Implementing a custom state-machine parser to handle double quotes (`" "`), single quotes (`' '`), and escaped spaces (`\ `), removing the reliance on basic `strtok()`.
* **Job Control:** Expanding the `signal` logic to support stopping/resuming jobs via `SIGTSTP` (Ctrl+Z), `fg`, and `bg`.
* **Logical Operators:** Expanding the `Command` structure to form an Abstract Syntax Tree (AST) to support conditional execution (`&&` and `||`).
