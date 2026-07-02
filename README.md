# Linux Mini Shell

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![POSIX](https://img.shields.io/badge/POSIX-000000?style=for-the-badge&logo=linux&logoColor=white)
![Makefile](https://img.shields.io/badge/Makefile-000000?style=for-the-badge&logo=gnu&logoColor=white)
![GitHub](https://img.shields.io/badge/github-%23121011.svg?style=for-the-badge&logo=github&logoColor=white)

## 📌 Project Overview
A lightweight, POSIX-compliant UNIX shell written in C from scratch. This project demonstrates core operating system concepts such as process creation, inter-process communication (IPC), file descriptor manipulation, and robust signal handling. It closely mirrors the behavior of standard shells like `bash` or `sh` while remaining modular and heavily documented.

## ✨ Features
- **Execution:** Runs standard foreground and background (`&`) processes.
- **Pipelining:** Supports unlimited chained pipelines (e.g., `ls -l | grep src | wc -l`).
- **I/O Redirection:** Handles input (`<`), output (`>`), and append (`>>`) file redirection.
- **Built-in Commands:** `cd`, `exit`, and `history`.
- **Signal Handling:** Gracefully handles `SIGINT` (Ctrl+C) without crashing, and automatically reaps background zombie processes via `SIGCHLD`.

## 🏗️ Architecture Overview
The shell is built with a clean, modular architecture separating concerns across dedicated C modules:
- **`shell.c`**: The main REPL (Read-Eval-Print Loop).
- **`parser.c`**: Parses raw string input into structured `Command` data objects.
- **`execute.c`**: Handles single-command `fork()` and `execvp()` logic.
- **`pipe.c`**: Orchestrates `N-1` pipes for `N` commands, chaining standard I/O streams.
- **`redirect.c`**: Manages file opens and `dup2()` descriptor mapping.
- **`signals.c`**: Installs isolated signal handlers and manages state restoration for child processes.
- **`builtin.c` & `history.c`**: Implements internal commands and memory-managed command history.

## 📂 Folder Structure
```text
linux-mini-shell/
├── Makefile
├── README.md
└── src/
    ├── builtin.c / builtin.h
    ├── command.c / command.h
    ├── execute.c / execute.h
    ├── history.c / history.h
    ├── main.c
    ├── parser.c / parser.h
    ├── pipe.c / pipe.h
    ├── redirect.c / redirect.h
    ├── shell.c / shell.h
    └── signals.c / signals.h
```

## 🛠️ System Calls Used
This project extensively utilizes UNIX system calls to interface directly with the kernel:
- **Process Management**: `fork()`, `execvp()`, `waitpid()`
- **IPC & Pipes**: `pipe()`, `dup2()`
- **File I/O**: `open()`, `close()`, `read()`, `write()`
- **Signals**: `signal()`, `sigaction()`, `kill()`
- **Environment**: `chdir()`

## 🚀 Build Instructions
To build and run the project locally, ensure you have `gcc` and `make` installed.

```bash
# Clone the repository
git clone https://github.com/HarshEvolves/linux-mini-shell.git
cd linux-mini-shell

# Build the project
make

# Run the shell
make run
# OR manually run: ./build/minishell
```

## 💡 Usage Examples

**Standard Execution & Backgrounding**
```bash
myshell$ ls -la
myshell$ sleep 10 &
[12345]
```

**Unlimited Pipelining**
```bash
myshell$ cat README.md | grep "C" | wc -l
```

**I/O Redirection**
```bash
myshell$ echo "Hello, World!" > output.txt
myshell$ cat < output.txt >> history.log
```

**Signal Resilience**
```bash
myshell$ sleep 10
^C  # Kills the sleeping child, but returns cleanly to the shell prompt
myshell$ 
```

## 🧠 Key OS Concepts Demonstrated
1. **Process Forking & Execution:** Replacing a child process image with a new executable while preserving parent state.
2. **Zombie Process Reaping:** Using non-blocking `waitpid(WNOHANG)` inside a `SIGCHLD` handler to prevent resource leaks.
3. **File Descriptor Chaining:** Routing `stdout` of one process into the `stdin` of another using kernel pipes and `dup2()`.
4. **Interruption Safety:** Safely handling `EINTR` errors from blocking calls like `getline()` and `waitpid()` when signals are caught.

## 🚀 Future Improvements
- **Logical Operators:** Support for `&&` and `||`.
- **Job Control:** Implementation of `fg`, `bg`, and `jobs` commands using `SIGTSTP`.
- **Quoting & Escaping:** Support for parsing strings containing spaces inside double/single quotes.
- **Environment Variables:** Support for expanding `$VAR` variables and `export`.

## 🎓 Learning Outcomes
Building this shell provided deep, hands-on experience with systems programming in C, kernel-level process management, memory safety (avoiding leaks across forks), and designing robust architectures that can safely handle asynchronous OS signals.