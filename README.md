# AShell - Command Line Interface (CLI)

## Overview

AShell is a custom shell implementation for a lab assignment in Advanced Operating Systems. It supports basic command execution, pipes, input/output redirection, and background processes in interactive mode.

## Features

1. **Command Execution**: Executes commands like `ls`, `cat`, etc.
2. **Argument Handling**: Supports commands with arguments, e.g., `cat file.txt`.
3. **Input/Output Redirection**: Supports `<` for input and `>` for output redirection.
4. **Multiple Commands**: Executes multiple commands separated by `;`.
5. **Pipes**: Supports multiple pipes (`|`) for chaining commands.
6. **Background Execution**: Executes commands in the background using `&`.
7. **Error Handling**: Prints an error message when an invalid command is executed.
8. **Built-in Commands**: Supports `quit` to exit the shell.

## Files

- `AShell.c`: Main source code for the shell.
- `Makefile`: Makefile for building the project.
- `test_shell.sh`: A shell script to test the functionality of the custom shell.
- `README.md`: This README file.

## Compilation

```bash
make
```

## Usage

```bash
./AShell
```

## Specifications

- Maximum command line length: 512 characters
- Handles empty command lines and extra white spaces
- Exits on 'quit' '!q' 'exit' command or end of input (Ctrl+D)

## Error Handling

- Command does not exist or cannot be executed
- Very long command lines (over 512 characters)

## Example Usage

```bash
AShell> ls -l
AShell> cat file.txt | grep pattern | sort
AShell> echo "Background job" & sleep 5 & echo "Foreground job"
AShell> cat < input.txt > output.txt
AShell> quit
```

## Testing

A test script (`test_shell.sh`) is provided to verify the functionality of AShell. It includes tests for:

- Complex piping and redirection
- Long commands with background execution
- Multiple background jobs
- Very long command lines (over 512 characters)
- Input/Output redirection with background processes

To run the tests:

```bash
./test_shell.sh
```

## Implementation Notes

- Uses `fork()`, `execvp()`, and `wait()/waitpid()` for command execution
- Implements `dup2()`, `dup()` for input/output redirection
- Implemented `open()`, `close()` for file operations
- Uses `pipe()` for creating pipes between commands
- Handles background processes using `&` operator
- Implements signal handling for background job completion
- Used `perror()` to report errors.

## Limitations

- Does not support advanced features like command history or tab completion
- Limited error handling for complex scenarios

## Contributors

Aashay Phirke
