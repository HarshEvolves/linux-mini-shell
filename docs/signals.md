# Signal Handling and Process Lifecycle

This document explains how the Linux Mini Shell uses kernel signals to manage foreground jobs, background jobs, and user interruptions safely.

## 1. Foreground vs. Background Processes
When a command is executed, it runs in one of two modes:
* **Foreground**: The shell blocks (halts) its own execution using `waitpid()` until the child process finishes.
* **Background (`&`)**: The shell prints the child's Process ID (PID) and instantly returns to the prompt. The child runs asynchronously alongside the shell.

## 2. Managing `SIGINT` (Ctrl+C)

When a user presses `Ctrl+C` in the terminal, the kernel sends a `SIGINT` (Signal Interrupt) to **all processes** attached to that terminal. A raw shell without signal handling would simply crash.

**How we protect the shell:**
1. During initialization, the shell intercepts `SIGINT` using a custom handler.
2. The handler prevents the shell from dying. Instead, it prints a clean newline.
3. Because the handler lacks the `SA_RESTART` flag, blocking functions like `getline()` or `waitpid()` are safely interrupted (returning `EINTR`). The shell catches this error, clears the input buffer, and safely reprints the `myshell$ ` prompt.
4. **Before executing a child command**, the shell calls `signals_child_reset()` to restore `SIGINT` to its default, deadly behavior for the child only.

```text
[ User presses Ctrl+C ]
        |
  Kernel sends SIGINT
        |
   +----+----+-----------------------+
   |                                 |
[ Shell ]                        [ Child Program ]
Custom Handler Caught!           Default Handler Triggered!
Action: Print \n, clear input    Action: Process is killed
Result: Shell survives!          Result: Command terminates
```

## 3. Managing `SIGCHLD` & Zombie Processes

When a child process finishes executing, it doesn't vanish immediately. It becomes a **"Zombie Process"**—a dead process that still consumes an entry in the kernel's process table until its parent reads its exit status using `waitpid()`.

* **Foreground Zombies**: Cleaned up instantly because the shell is already actively waiting for them.
* **Background Zombies**: Since the shell does not wait for background jobs, they will remain zombies forever and cause a memory leak unless explicitly handled.

**The Solution (`SIGCHLD`):**
Whenever a child process dies, the kernel alerts the parent shell by sending a `SIGCHLD` signal. 
The shell catches this signal and immediately triggers a background cleanup loop using **`waitpid(-1, NULL, WNOHANG)`**.

* `-1`: Wait for *any* child process.
* `WNOHANG`: Non-blocking flag. If there are no dead children to reap, do not freeze the shell; return instantly.

```text
[ Background Job Finishes ]
           |
   Becomes a Zombie 🧟
           |
 Kernel sends SIGCHLD to Parent
           |
[ Shell's SIGCHLD Handler ]
           |
   waitpid(WNOHANG) loop
   (Reaps the exit status)
           |
 Zombie vanishes from memory ✨
```

By heavily managing these two critical signals, the shell behaves robustly: it survives user interruptions and prevents background memory leaks.
