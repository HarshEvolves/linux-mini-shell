# I/O Redirection

This document explains how the Linux Mini Shell handles file redirection to route data streams dynamically.

## 1. File Descriptors & The Standard Streams
In Unix, everything is treated as a file. When a program runs, the operating system automatically opens three standard streams, represented by integer **File Descriptors (FDs)**:
* **`0` (`STDIN_FILENO`)**: Standard Input (default: Keyboard)
* **`1` (`STDOUT_FILENO`)**: Standard Output (default: Terminal Screen)
* **`2` (`STDERR_FILENO`)**: Standard Error (default: Terminal Screen)

When a program like `grep` reads data, it reads from FD 0. When it prints results, it writes to FD 1. I/O redirection simply intercepts and replaces these default file descriptors before the program starts.

## 2. Supported Redirection Types
* **`<` (Input)**: Replaces `stdin`. The command reads its input from a specified file rather than the keyboard.
* **`>` (Output)**: Replaces `stdout`. The command writes its output to a specified file. If the file exists, it is **truncated** (overwritten).
* **`>>` (Append)**: Replaces `stdout`. The command writes its output to a file, **appending** to whatever is already there.

## 3. System Calls Used

The shell performs redirection in the child process using three crucial system calls:

1. **`open(const char *pathname, int flags, mode_t mode)`**: Opens the target file and returns a new, unused file descriptor (e.g., FD `3`).
   * For `>`, we use flags: `O_WRONLY | O_CREAT | O_TRUNC`.
   * For `>>`, we use flags: `O_WRONLY | O_CREAT | O_APPEND`.
   * For `<`, we use flags: `O_RDONLY`.
2. **`dup2(int oldfd, int newfd)`**: The workhorse of redirection. It forces the `newfd` (e.g., FD 1) to point to the same open file as `oldfd` (e.g., FD 3). If `newfd` was already open (like the terminal), the kernel closes it automatically before duplicating.
3. **`close(int fd)`**: Closes the original `oldfd` (e.g., FD 3) because it is no longer needed once it has been duplicated onto FD 0 or 1.

## 4. Execution Flow Diagram

Here is an example of what happens under the hood when a user types:  
`echo "Hello" > output.txt`

```text
[ Step 1: Default State ]
FD 0 -> Keyboard
FD 1 -> Terminal Screen
FD 2 -> Terminal Screen

[ Step 2: open("output.txt") ]
FD 0 -> Keyboard
FD 1 -> Terminal Screen
FD 2 -> Terminal Screen
FD 3 -> output.txt (New!)

[ Step 3: dup2(3, STDOUT_FILENO) ]
FD 0 -> Keyboard
FD 1 -> output.txt <--- (Terminal screen closed, now points to file)
FD 2 -> Terminal Screen
FD 3 -> output.txt

[ Step 4: close(3) ]
FD 0 -> Keyboard
FD 1 -> output.txt
FD 2 -> Terminal Screen
(FD 3 is closed and cleaned up)

[ Step 5: execvp("echo") ]
The "echo" program starts. It knows nothing about redirection. 
It simply writes "Hello" to FD 1, which the kernel silently routes 
directly into output.txt!
```

## 5. Security & Permissions
When `open()` creates a new file, it requires permissions. The shell uses the mode `0644` (`S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH`), which grants the user read/write access, while giving groups and others read-only access.
