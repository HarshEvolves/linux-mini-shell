# Shell Execution Process Flow

This document outlines the step-by-step lifecycle of a command inside the Linux Mini Shell, from the moment a user hits "Enter" to the appearance of the next prompt.

## 🔄 Execution Flow Diagram

```text
       [ Display Prompt ] <-----------------------------------------------+
               |                                                          |
        [ getline() ] --(SIGINT caught)--> [ Print \n & Clear ] ----------+
               |                                                          |
        [ Parse Input ] (Tokenization, background '&' detection)          |
               |                                                          |
      [ Add to History ]                                                  |
               |                                                          |
       Is it a Built-in? (cd, exit, hist)                                 |
      /                 \                                                 |
  [ YES ]             [ NO ]                                              |
     |                   |                                                |
 [ Exec ]           Is it a Pipeline? (|)                                 |
     |             /                     \                                |
     |         [ YES ]                  [ NO ]                            |
     |           |                         |                              |
     |   [ Create N-1 Pipes ]              |                              |
     |   [ Loop N times:    ]              |                              |
     |   [   fork()         ]          [ fork() ]                         |
     |           |                         |                              |
     |           +-----------+-------------+                              |
     |                       |                                            |
     |                 [ In Child ]                                       |
     |                 1. Reset Signals                                   |
     |                 2. dup2() Pipes (if any)                           |
     |                 3. dup2() Redirection (<, >)                       |
     |                 4. execvp()                                        |
     |                 (Child process replaced)                           |
     |                       |                                            |
     |                 [ In Parent ]                                      |
     +<---(Background?)------+                                            |
               |                                                          |
           [ YES ]                                                        |
             |------------------------------------------------------------+
             |                                                            |
            [ NO ]                                                        |
             |                                                            |
       [ waitpid() Loop ] (Retries on EINTR if interrupted by Ctrl+C)     |
             |                                                            |
             +------------------------------------------------------------+

*(Note: Background processes that finish are reaped asynchronously via a `SIGCHLD` signal handler running `waitpid(WNOHANG)`.)*
```

## 📝 Step-by-Step Breakdown (Interview Guide)

When a user submits a command, the shell executes the following sequence:

### 1. Input Reading & Signal Resilience
The shell blocks on `getline()`. If the user presses `Ctrl+C` (`SIGINT`), the shell's custom signal handler intercepts it, prints a newline, and `getline()` returns an `EINTR` error. The shell clears this error and immediately reprints a clean prompt.

### 2. Parsing & Command Structuring
The raw string is tokenized by `parser.c`. It detects background execution (trailing `&`), splits commands by pipes (`|`), identifies I/O redirection files (`<`, `>`, `>>`), and packages this data into clean `Command` structs.

### 3. Built-in Delegation
The shell checks if the command is a built-in (e.g., `cd`, `exit`). Built-ins must execute in the **parent process** because they modify the shell's own state (like its working directory or memory). If it's a built-in, the shell executes it and loops back to the prompt.

### 4. Process Creation (`fork`)
For external commands, the shell calls `fork()` to create a child process. If there are multiple piped commands, it creates `N-1` pipes first, and calls `fork()` `N` times in a loop.

### 5. Child Environment Wiring
Inside the child process:
1. **Signal Reset**: `SIGINT` is restored to default behavior so the child can be killed normally.
2. **Pipe Wiring**: `dup2()` is used to connect `stdin`/`stdout` to the appropriate read/write ends of the pipes. All inherited pipe file descriptors are then strictly `close()`'d.
3. **I/O Redirection**: If the user specified a file (e.g., `> out.txt`), `open()` creates/opens the file, and `dup2()` maps it to standard output/input.
4. **Execution**: `execvp()` is called, completely replacing the child's memory image with the new program (e.g., `ls` or `grep`).

### 6. Parent Synchronization (`waitpid`)
Inside the parent process:
* **Background (`&`)**: The parent prints the child's PID and immediately returns to the prompt. 
* **Foreground**: The parent calls `waitpid()` to block until the child finishes. If the user presses `Ctrl+C` while waiting, the child dies, but the parent's `waitpid()` is interrupted (`EINTR`). The parent safely retries the wait loop to ensure the process is fully reaped before returning to the prompt.

### 7. Asynchronous Zombie Reaping
If a background job finishes while the shell is waiting for input, the kernel sends a `SIGCHLD` signal. The shell's background signal handler uses a non-blocking `waitpid(-1, NULL, WNOHANG)` loop to quietly reap the zombie process without interrupting the user.
