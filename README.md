# Linux Mini Shell

A minimal Unix shell written in C. Supports executing external commands via `fork()` and `execvp()`, along with built-in commands like `cd` and `exit`.

## Features

- Execute external commands
- Built-in `cd`
- Built-in `exit`
- Graceful EOF handling (`Ctrl+D`)

## Build

```bash
make