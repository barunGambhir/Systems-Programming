[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/pCdUSgOf)
# Assignment 9: Simple Shell

## Overview

In this assignment, you will develop a simple shell. A typical shell provides a command line prompt
at which user commands are entered. It executes each command in a separate process. It also runs a
command in the background if the ampersand (`&`) is used at the end of the command. You will use the
system calls you have learned so far to implement such a shell.

There are three mandatory features that you will need to implement:

* Execute user commands in separate processes, either in the foreground or in the background if
  `&` is used.
* Implement some internal commands.
* Implement a history feature.

Before you start the assignment, make sure you `record`.

## Code Structure, CMake, and the Grader

You need to structure your code nicely according to what was discussed in A4. Also, you need to use
CMake as the build system. We expect the following code structure.

```bash
  a9
  |-- CMakeLists.txt
  |-- include
  |   \-- *.h
  |-- gtest
  |   \-- (test source)
  \-- src
      \-- *.c
```

* `include/` contains a header file already. You will use the header file for this assignment. Also,
  you need to add your own header files there.
* You need to create `src/` and add all your `.c` files.
* You need to make sure that you use the correct C and C++ compilers with CMake. As you know
  already, we use `clang` in this course, and the commands below set the C and C++ compilers for
  CMake to `clang` and `clang++`. What they do is setting the environment variables `CC` and `CXX`,
  which various programs commonly use to find the C and C++ compilers.

  ```bash
  $ export CC=$(which clang)
  $ export CXX=$(which clang++)
  ```

* `gtest/` contains the test cases for grading. We use [Google
  Test](https://github.com/google/googletest/tree/main) to develop and run the test cases.
* `CMakeLists.txt` includes directives to compile the test cases. However, this `CMakeLists.txt` is
  not complete and `cmake` won't work. You need to add more directives to make it work. What you
  have learned from A4 should be enough to do it.
* After you make `cmake` work, you can run the executable it generates to run the test cases. At the
  end of each run, you will see how many points you get.
* The test cases expect that there is an executable named `shell` in the build directory. That is
  the executable that you need to produce for your shell. Thus, you need to edit `CMakeLists.txt` to
  generate `shell` from your code. You can use `shell` for your own manual testing during
  development as well.
* The test cases expect that regular messages/outputs go to the standard output stream and error
  messages/outputs go to the standard error stream. You need to use these streams appropriately.
* You should be able to use regular CMake commands to generate the executable for test cases as well
  as `shell`, e.g., `cmake -S . -B build && cmake --build build`.
* Remember that test cases provide an input and check the output, and they do nothing more than
  that. If a test case fails for you, go to the test case source, find out the input it uses and the
  output it expects, and start your own debugging process with the input. It is *your job* to figure
  out which test case fails and which part of the source you need to look at.
* The test cases are *not* exactly the same as what we run for our own grading. This is to prevent
  you from hard-coding or customizing your implementation tailored to our test cases. For example,
  to test an invalid command, we use a particular invalid command in the test source. This is not
  exactly the same as what we use for our own grading. Generally, you should consider the provided
  test cases as examples or specific instances of general cases.
* Still rest assured---if you do not hard-code or customize your implementation tailored to our test
  cases, what you get from running the test cases will indeed be what you get in the end.

## Task 0: Shell Prompt & Basic Command Support

The first feature you need to implement is executing programs that a user enters at the prompt. You
need to use the necessary system calls appropriately to implement this feature. Of course, you need
to handle a user command as input and pass all arguments correctly for execution. There are two
modes of execution for external programs.

* Foreground execution: In this mode, you need to execute a program in a child process and wait
  until the child process terminates.
* Background execution: In this mode, you need to execute a program in a child process but should
  not wait for the termination of the child process.
* `&` at the end of a user-entered command means that the command should run in the background.

There are a few things to keep in mind.

* For all error messages, *use the macros defined in `msgs.h` for correct formatting*.
* Your shell prompt should always show the current working directory. For example, if the user is in
  the `/home/cmpt201` folder, the prompt should be:

  ```bash
  /home/cmpt201$
  ```

  Use the `getcwd()` function to get the current directory. If `getcwd()` returns an error, display
  this error message: `shell: unable to get current directory`.
* For foreground execution, you need to wait for the exact process you fork and not for any other
  processes.
* Since we do not wait for a background process to terminate, it can become a zombie process (where
  a child process terminates but the parent process doesn't wait on it). Thus, your shell should
  still wait on each background child process *at some point* to clean it up. There are many ways to
  achieve this, but for this assignment, you can make an *additional* `waitpid()` call every time a
  user enters a command. The following are some hints.
    * This `waitpid()` call should wait on *any child process* since you're cleaning up zombie
      processes. Use an appropriate `pid` for that.
    * This `waitpid()` call should *return immediately if no child has exited*. Use an appropriate
      option for that.
    * Consult the man page of `waitpid()` for the above two points.
    * Remember that there can be multiple background processes launched previously. Thus, a single
      call will not clean up all of them. You need to use a loop to clean up all zombie processes.
* For input/output, use `read()` and `write()` since we need to handle a signal in the next task.
  Other functions like `printf()` do not play nicely with signals. To see the list of functions safe
  for use with signals, run `man signal-safety`.
* If you encounter an error when reading a user command with `read()`, you need to print out this
  error message: `shell: unable to read command`.
* If `fork()` fails, print out this error message: `shell: unable to fork`.
* If `exec` fails, print out this error message: `shell: unable to execute command`.
* If `wait` fails, print out this error message: `shell: unable to wait for child`.

## Task 1: Internal Commands and SIGINT

The second task is to implement a shell prompt and some internal commands.

* Your shell should implement a few internal commands described below. Internal commands are
  built-in commands of the shell itself, as opposed to a separate program that is executed. You
  should not fork new processes for internal commands as they are built-in. All the commands should
  be *case-sensitive*. For all error messages, use the macros defined in `msgs.h` for correct
  formatting.
    * `exit`: Exit the shell program. If the user provides any argument, abort the operation (i.e.,
      do not exit) and print out this error message: `exit: too many arguments`.
    * `pwd`: Display the current working directory. Use the `getcwd()` function. If `getcwd()`
      returns an error, display this error message: `pwd: unable to get current directory`. If the
      user provides any argument, abort the operation and print out this error message: `pwd: too
      many arguments`.
    * `cd`: Change the current working directory. Use the `chdir()` function. If `chdir()` returns
      an error, display this error message `cd: unable to change directory`. If the user passed in
      more than one argument, abort the operation and print out this error message: `cd: too many
      arguments`. Additionally, implement the following features:
        * Change to the home directory if no argument is provided.
        * Support `~` for the home folder. For example, `cd ~/cmpt201` should change to the
          `cmpt201` directory under the current user's home directory. Issuing `cd ~` should switch
          to the home directory.
        * Support `-` for changing back to the previous directory. For example, suppose that the
          current working directory is `/home` and you issued `cd /` to change to the root
          directory. Then, `cd -` should switch back to use the macros in `msgs.h` to the `/home`
          directory.
        * You may find the `getuid()` and `getpwuid()` functions useful. They allow you to gather
          useful information about the current user.
    * `help`: Display help information on internal commands.
        * If the first argument is one of the internal commands, print out the command and the help
          message. For example, if the command is `cd`, print out `cd: change the current
          directory`.
        * If the first argument is not an internal command, print out the program name and the help
          message. For example, if argument is `ls`, print out `ls: external command or
          application`.
        * If there is more than one argument, display this error message: `help: too many
          arguments`.
        * If there is no argument provided, list all the supported internal commands one by one with
          their corresponding help messages.
* Your shell should not terminate when a user presses `CTRL-C`. Thus, you need to write a signal
  handler for `SIGINT`.
    * Have the signal handler display the help information (same as the `help` command).
    * Then re-display the command-prompt before returning.
    * A user can press `CTRL-C` anytime and interrupt system calls that are being executed. Thus,
      you need to properly check the return value of a system call to check if there was an error
      and also check `errno` to check which error it was. More specifically, you need to understand
      which value `errno` is the error was because of `SIGINT`. You need to handle `SIGINT` errors
      differently from other errors.

## Task 2: History Feature

The next task is to modify your shell to provide a history feature that allows the user access up to
the 10 most recently entered commands. Start numbering the user's commands at 0 and increment for
each command entered. These numbers will grow past 9. For example, if the user has entered 35
commands, then the most recent 10 will be numbered 15 through 34. For all error messages, use the
macros defined in `msgs.h` for correct formatting.

* First, implement an internal command `history` which displays the 10 most recent commands executed
  in the shell. If there are less than 10 commands entered, display all the commands entered so far.
    * Display the commands in descending order (sorted by its command number).
    * The output should include both external application commands and internal commands.
    * Display the command number on the left, and the command (with all its arguments) on the right.
        * Use the macros in `msgs.h`.
        * If the command is run in the background using `&`, it must be added to the history with
          the `&`.
    * A sample output of the history command is shown below:

      ```bash
      /home/cmpt201$ history
      30	history
      29	cd /home/cmpt201
      28	cd /proc
      27	cat uptime &
      26	cd /usr
      25	ls
      24	man pthread_create
      23	ls
      22	echo "Hello World from my shell!"
      21	ls -la
      /home/cmpt201$
      ```

* Next, implement the `!` and related commands which allows users to run commands directly from the
  history list:
    * Command `!n` runs command number n. For example, `!22` re-runs the 23rd command in the
      history. In the above example, this will re-run the echo command.
        * If n is not a number, or an invalid value (not one of the previous 10 command numbers)
          then display this error: `history: command invalid`.
    * Command `!!` runs the last command entered.
        * If there is no previous command, display the error message: `history: no command entered`.
          The history should remain empty.
    * When running a command using `!n` or `!!`, display the command from the history to the screen
      so the user can see what command they are actually running.
    * Neither the `!!` nor the `!n` commands are to be added to the history list themselves, but
      rather the command being executed from the history must be added. Here is an example.

      ```bash
      /home/cmpt201$ echo test
      test
      /home/cmpt201$ !!
      echo test
      test
      /home/cmpt201$ history
      15	history
      14	echo test
      13	echo test
      12	ls
      11	man pthread_create
      10	cd /home/cmpt201
      9	ls
      8	ls -la
      7	echo Hello World from my shell!
      6	history
      /home/cmpt201$
      ```

## Other Requirements and Exceptions

* Your code must not have any memory leaks or memory access errors. Use appropriate sanitizers and
  test your code to make sure you do not have any memory issues.
* Your child processes may exit without freeing all their memory. You do not need to handle such
  cases.
* You do not need to support either `>` (output redirection) or `|` (pipe).

## Grading

* Code that does not compile with CMake gets a 0.
* Memory issues have a penalty of -10 pts.
* A wrong code directory structure has a penalty of -10 pts.
* Distribution
    * [29 points] Task 0
    * [39 points] Task 1
    * [32 points] Task 2

## Acknowledgment

Created by Mohamed Hefeeda, modified by Brian Fraser, Keval Vora, Tianzheng Wang, and Steve Ko.
