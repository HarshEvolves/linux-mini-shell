# Linux Mini Shell

A minimal Unix shell written in C. Supports executing external commands via `fork`/`execvp` and includes built-in commands for `cd` and `exit`.

## Features

- **Command execution** — runs any program found in `$PATH`
- **Built-in `cd`** — change directory (defaults to `$HOME`)
- **Built-in `exit`** — quit the shell
- **Ctrl+D** — graceful EOF handling

## Build & Run

```bash
make        # compile
make run    # compile and launch the shell
make clean  # remove build artifacts
```

## Project Structure

```
linux-mini-shell/
├── src/
│   └── main.c        # Shell implementation
├── Makefile
├── README.md
└── .gitignore
```

## Usage

```
minishell$ ls -la
minishell$ cd /tmp
minishell$ pwd
minishell$ echo hello world
minishell$ exit
```

## License

MIT
