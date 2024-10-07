#!/bin/bash

run_test() {
    echo "Test: $1"
    echo "$2" | ./AShell > output.txt 2> error.txt
    if grep -q "$3" output.txt || grep -q "$3" error.txt ; then
        echo "PASS"
    else
        echo "FAIL"
        echo "Expected: $3"
        echo "Got:"
        cat output.txt
    fi
    echo
}

make

# Test basic command execution
run_test "Basic command" "ls" "$(ls)"

# Test command with arguments
run_test "Command with arguments" "ls -l" "$(ls -l)"

# Test multiple commands
run_test "Multiple commands" "echo Hello ; echo World" "Hello
World"

# Test input redirection
echo "Test input" > test_input.txt
run_test "Input redirection" "cat < test_input.txt" "Test input"

# Test output redirection
run_test "Output redirection" "echo Test output > test_output.txt ; cat test_output.txt" "Test output"

# Test piping
run_test "Piping" "echo Hello World | grep World" "Hello World"


# Test error handling for non-existent command
run_test "Non-existent command" "nonexistentcommand" "Error: Command not found - nonexistentcommand"

# Test quit command
run_test "Quit command" "echo Before quit ; quit ; echo After quit" "Before quit"


# Test empty command line
run_test "Empty command line" "" ""

# Test extra whitespace
run_test "Extra whitespace" "  ls   -l  " "$(ls -l)"

# Test 1: Very long command with multiple pipes and redirection
run_test "Complex piping and redirection" \
"cat /etc/passwd | grep root | sort | uniq | wc -l > root_count.txt ; cat root_count.txt" \
"3"

# Test 2: Long command with background execution
run_test "Long command with background execution" \
"ls -la /etc | grep '^d' | sort -r | head -n 5 & echo 'Background job started'; sleep 2; echo 'Finished'" \
"Background job started
Finished"

# Test 3: Multiple background jobs
run_test "Multiple background jobs" \
"sleep 2 & echo Job1 & sleep 1 & echo Job2 & wait; echo All jobs done" \
"Job1
Job2
All jobs done"

# Test 4: Complex command with pipes and background
run_test "Complex command with pipes and background" \
"find /usr/bin -type f -size +1M -exec ls -lh {} + | sort -k5 -hr | head -n 3 & echo 'Search started'; sleep 2; echo 'Finished'" \
"Search started
Finished"

# Test 5: Input/Output redirection with background
run_test "I/O redirection with background" \
"echo 'Hello, World!' > test_file.txt & cat < test_file.txt & sleep 1; cat test_file.txt; rm test_file.txt" \
"Hello, World!"

# Test 6: Very long command line (over 512 characters)
long_cmd=$(printf 'echo "%0.s-" {1..600}' | tr -d '\n')
run_test "Very long command line" \
"$long_cmd & echo 'Long command started'; sleep 1; echo 'Finished'" \
"Long command started
Finished"

# Test 7: Multiple commands with different wait times
run_test "Multiple commands with different wait times" \
"sleep 3 & echo 'Long job'; sleep 1 & echo 'Short job'; wait; echo 'All done'" \
"Long job
Short job
All done"

# Cleanup
rm -f root_count.txt test_input.txt test_output.txt output.txt error.txt

echo "All tests completed."
