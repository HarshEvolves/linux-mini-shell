# Systems & Embedded Interview Notes: Linux Mini Shell

This document contains 48 common interview questions focusing on OS-level systems programming in C, heavily inspired by the implementation of a custom Unix shell. Keep answers concise, technical, and accurate.

---

## Process Management (`fork`, `exec`, `wait`)

**1. What does `fork()` do?**
It creates a new child process by duplicating the calling process. The child receives an exact copy of the parent's memory space, open file descriptors, and CPU registers.

**2. What does `fork()` return?**
It returns `0` to the child process, the child's `PID` (> 0) to the parent, and `-1` on error.

**3. What is a zombie process?**
A process that has completed execution but still has an entry in the process table because its parent hasn't read its exit status via `wait()` or `waitpid()`.

**4. How do you prevent zombie processes?**
By having the parent call `wait()` or `waitpid()` on the child, or by setting the `SIGCHLD` action to `SIG_IGN` (though catching `SIGCHLD` and calling `waitpid` is best practice).

**5. What is an orphan process?**
A child process whose parent has terminated. In Linux, orphans are immediately adopted by the `init` process (PID 1), which automatically reaps them.

**6. Does `execvp()` create a new process?**
No. It replaces the current process's memory space (text, data, heap, and stack) with a brand-new program. The PID remains identical.

**7. What is the difference between `execvp()` and `execl()`?**
`execvp()` takes an array of string arguments and searches the `$PATH` for the executable. `execl()` takes a variable-length list of arguments (varargs) and requires an absolute/relative path.

**8. What happens to file descriptors across an `exec` call?**
Unless marked with the `FD_CLOEXEC` flag, all open file descriptors remain open and inherited by the new program. This is the basis of shell I/O redirection.

**9. What happens if `execvp()` returns?**
It means it failed (e.g., file not found or permission denied). A successful `execvp()` never returns because the calling program is overwritten.

**10. What is the purpose of `waitpid()`?**
It forces the parent to suspend execution until a specific child (or any child, if passed `-1`) terminates or changes state, allowing the parent to read its exit status and clear it from the process table.

---

## Inter-Process Communication & File Descriptors

**11. What is a file descriptor?**
A non-negative integer used by the kernel as an index into a per-process table of open files, sockets, and pipes.

**12. What are the standard file descriptors?**
`0`: Standard Input (`STDIN_FILENO`)  
`1`: Standard Output (`STDOUT_FILENO`)  
`2`: Standard Error (`STDERR_FILENO`)

**13. How does I/O redirection work at the system call level?**
The shell calls `open()` to get a new file descriptor for a file, then uses `dup2()` to overwrite FD 0, 1, or 2 with the new descriptor.

**14. What does `dup2(oldfd, newfd)` do?**
It duplicates the open file referenced by `oldfd` onto `newfd`. If `newfd` is already open, it is silently closed first.

**15. What is a Unix pipe?**
A unidirectional IPC channel with a read end (`fd[0]`) and a write end (`fd[1]`). Data written to the write end is buffered in the kernel until read from the read end.

**16. Why must you close unused pipe ends in the parent?**
If the parent leaves write ends open, the kernel assumes the parent might write more data. Thus, readers won't receive an `EOF` and will block indefinitely.

**17. Why must you close unused pipe ends in the child?**
Similarly, if a child leaves its copy of a pipe's write end open while another child tries to read from that pipe, the reader will never receive an `EOF` (deadlock).

**18. What happens if a process reads from an empty pipe where all write ends are closed?**
The `read()` system call returns `0`, signaling `EOF` (End of File).

**19. What happens if a process writes to a pipe where all read ends are closed?**
The kernel sends a `SIGPIPE` signal to the writing process, which typically terminates it.

**20. How do you implement `ls | wc`?**
Parent creates a pipe, forks two children. Child 1 uses `dup2` to attach `STDOUT` to the pipe write end. Child 2 uses `dup2` to attach `STDIN` to the pipe read end. Both children close all original pipe FDs and `execvp`. Parent closes all pipe FDs and `waitpid`s for both.

---

## Signals

**21. What is a signal?**
An asynchronous software interrupt delivered by the OS to a process, alerting it of events like terminal interrupts, memory faults, or child terminations.

**22. What signal is sent when the user presses `Ctrl+C`?**
`SIGINT` (Signal Interrupt).

**23. What is the difference between `signal()` and `sigaction()`?**
`signal()` is an older, less portable POSIX function. `sigaction()` is highly robust, allowing masking of other signals during execution and specific behavioral flags (like `SA_RESTART`).

**24. What does the `SA_RESTART` flag do?**
If a blocking system call (like `wait` or `read`) is interrupted by a signal, `SA_RESTART` tells the OS to automatically restart the system call rather than returning `-1` and setting `errno` to `EINTR`.

**25. Why might `getline()` or `waitpid()` return `-1` with `errno == EINTR`?**
Because a signal (without `SA_RESTART`) arrived and interrupted the blocking system call. The shell must handle this gracefully (e.g., reprinting the prompt or retrying `waitpid`).

**26. How does a shell safely handle `Ctrl+C` while waiting for input?**
It catches `SIGINT` with a custom handler. The interruption causes `getline()` to return `EINTR`. The shell clears the error and loops back to print a fresh prompt, refusing to exit.

**27. How does the shell ensure a foreground child is killed by `Ctrl+C` but the shell survives?**
The shell intercepts `SIGINT`. However, after `fork()` and before `execvp()`, the child resets its `SIGINT` handler back to `SIG_DFL` (default behavior). Thus, the child dies, but the shell lives.

**28. What signal alerts a parent that a child has terminated?**
`SIGCHLD`.

**29. How do you reap background processes without blocking the shell?**
Set a `SIGCHLD` handler. Inside the handler, run `waitpid(-1, &status, WNOHANG)` in a `while` loop until it returns `<= 0`.

**30. Why use `WNOHANG` with `waitpid()`?**
It makes `waitpid` non-blocking. If no children have exited, it returns `0` immediately instead of freezing the shell.

---

## Architecture & Built-ins

**31. Why are built-in commands necessary?**
Because some commands must alter the internal state of the shell process itself. A forked child cannot alter its parent's state.

**32. Name three commands that MUST be built-ins.**
`cd`, `exit`, `export`.

**33. Why can't `cd` be executed as an external command?**
A child process inherits the parent's working directory. If `cd` were an external binary, the child would change its directory and immediately exit, leaving the parent's (shell's) working directory entirely unchanged.

**34. Why can't `exit` be executed as an external command?**
An external `exit` binary would just kill itself. To exit the shell, the shell process itself must break its main loop or call `exit()`.

**35. What is a REPL?**
Read-Eval-Print Loop. The core infinite loop of any shell: read user input, evaluate/parse it, print results/execute it, and repeat.

**36. Why split tokenization (parsing) from execution?**
Separation of concerns. Modularity makes the codebase extensible. Execution logic shouldn't care about spaces or quotes, only an array of strings (`argv`).

**37. How do you implement a background job in a shell?**
Detect a trailing `&`. Call `fork()`, but in the parent, simply print the child's PID and skip the `waitpid()` call.

**38. What is the difference between foreground and background execution?**
Foreground execution forces the parent shell to block via `waitpid` until completion. Background execution skips the wait, allowing the shell to immediately prompt for new input.

---

## Advanced Systems & Edge Cases

**39. What is the `errno` variable?**
A thread-local integer set by system calls and library functions to indicate what went wrong during an error (e.g., `ENOENT` for file not found).

**40. How does `perror()` work?**
It maps the current integer value of `errno` to a human-readable string and prints it to `stderr`, prefixed by an optional custom string.

**41. How does `open()` handle file creation for `>` redirection?**
It uses flags: `O_WRONLY | O_CREAT | O_TRUNC`. This opens the file for writing, creates it if it's missing, and truncates (empties) it if it already exists.

**42. What is the difference between `>` and `>>` redirection?**
`>` truncates (`O_TRUNC`), while `>>` appends (`O_APPEND`), adding data to the end without overwriting.

**43. What permissions should be used when a shell creates a new file via `open()`?**
Typically `0644` (`S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH`), granting read/write to the owner and read-only to everyone else.

**44. What happens if you call `waitpid(-1, NULL, 0)` in a loop indefinitely?**
It will block and wait for children. Once the process has zero children remaining, it returns `-1` and sets `errno` to `ECHILD`.

**45. Can `execvp()` fail? Give an example.**
Yes. It fails if the binary does not exist (returns `ENOENT`), lacks execution permissions (`EACCES`), or isn't a valid executable format (`ENOEXEC`).

**46. What happens to allocated heap memory (malloc) after `execvp()`?**
It is completely wiped out and replaced by the new program's memory. You don't need to `free()` memory right before calling `execvp()`.

**47. What happens to heap memory after `fork()`?**
The child gets an exact, independent copy. Modifying a variable in the child does not affect the parent.

**48. Why is it dangerous to use `printf()` inside a signal handler?**
`printf()` is not async-signal-safe. It uses internal locks for `stdout`. If a signal interrupts a `printf()` call in the main program and the handler calls `printf()`, it can cause a deadlock. You should use `write()` instead.
