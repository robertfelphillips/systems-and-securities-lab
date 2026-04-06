# Custom Linux Shell

## Overview

A Unix-style shell written in C for systems programming practice. This project supports command execution, environment expansion, pipelines, redirection, background jobs, and a few built-in commands.

## Features

- Custom prompt showing `user@host:cwd`
- Tokenization for commands with `|`, `<`, `>`, and `&`
- Environment variable expansion such as `$HOME`
- Tilde expansion for `~` and `~/...`
- Executable resolution using system PATH
- Input and output redirection
- Multi-stage command pipelines
- Background job tracking with job control (`jobs`)
- Built-in `cd` and `exit`
- Exit history for the last few valid commands

## Tech Stack

- Language: C
- Environment: POSIX (Linux / macOS / WSL)
- Tools: GCC, Makefile

## Concepts Demonstrated

- Process creation and control (`fork`, `exec`, `waitpid`)
- Inter-process communication using pipes
- File descriptor manipulation (`dup2`, `open`)
- Command parsing and tokenization
- Background job management

## Files

- `src/main.c`
- `src/shell.c`
- `include/shell.h`
- `src/lexer.c`
- `include/lexer.h`
- `Makefile`

## Build and Run

This project targets a POSIX environment such as Linux, macOS, or WSL.

```bash
cd custom-linux-shell
make
./bin/shell
```

## Example Commands

```bash
ls
echo $HOME
cat < input.txt
echo hello > output.txt
ls | wc
sleep 5 &
jobs
cd ..
exit
```

## Design Notes

- The shell uses `fork`, `execv`, `waitpid`, `pipe`, `dup2`, and `open` to implement process creation and I/O control.
- Background pipelines are tracked as job groups so completed children can be reaped correctly.
- Basic syntax validation is included for malformed pipes, redirection, and background operators.

## Current Scope

This is a teaching-focused shell, not a full Bash replacement. It does not yet implement quoting, wildcard expansion, command substitution, or advanced job control.
