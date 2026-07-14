# Contributing to Linux Mini Shell

First off, thank you for considering contributing to Linux Mini Shell! It's people like you that make the open-source community such a great place to learn, inspire, and create.

## How Can I Contribute?

### Reporting Bugs

- Ensure the bug was not already reported by searching on GitHub under Issues.
- If you're unable to find an open issue addressing the problem, open a new one. Be sure to include a title and clear description, as much relevant information as possible, and a code sample or an executable test case demonstrating the expected behavior that is not occurring.

### Suggesting Enhancements

- Open a new issue with a clear title and description of your enhancement.
- Explain why this enhancement would be useful to most users.

### Pull Requests

1. Fork the repo and create your branch from `main`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes (`make test`).
5. Make sure your code lints and compiles with zero warnings (`make`).
6. Issue that pull request!

## Code Style

- Follow standard C99 conventions.
- Keep functions short and modular.
- Use descriptive variable and function names.
- Document any complex logic using inline comments.

## Setup for Local Development

1. Clone your fork:
   ```bash
   git clone https://github.com/YOUR-USERNAME/linux-mini-shell.git
   ```
2. Navigate to the project directory:
   ```bash
   cd linux-mini-shell
   ```
3. Build the project:
   ```bash
   make
   ```
4. Run the shell:
   ```bash
   ./build/minishell
   ```

Thanks for your contributions!
