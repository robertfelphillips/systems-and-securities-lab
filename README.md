# Mini Shell in C

A Unix-style shell written in C for systems programming practice. The project supports command execution, environment expansion, pipelines, redirection, background jobs, and a few built-in commands.

## Highlights

- Custom prompt showing `user@host:cwd`
- Tokenization for commands with `|`, `<`, `>`, and `&`
- Environment variable expansion such as `$HOME`
- Tilde expansion for `~` and `~/...`
- PATH lookup for executables
- Input and output redirection
- Multi-stage pipelines
- Background job tracking with `jobs`
- Built-in `cd` and `exit`
- Exit history for the last few valid commands

## Project Structure

```text
shell/
  include/
    lexer.h
    shell.h
  src/
    lexer.c
    main.c
    shell.c
  Makefile
```

## Build and Run

This project targets a POSIX environment such as Linux, macOS, or WSL.

```bash
cd shell
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
