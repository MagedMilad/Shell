SHELL
===
##Overview
Implementing the UNIX shell in C using multi-processes, support the interactive and batch mode ,
foreground and background processes , log file with ids of terminated child processes ,
history file of all executed commands of the user

##Objectives

• To get familiar with Linux and its programming environment.

• To understand the relationship between OS command interpreters (shells), system calls, and the kernel.

• To learn how processes are handled (i.e., starting and waiting for their termination).

• To learn robust programming and modular programming.

##Overview

You are required to implement a command line interpreter (i.e., shell). The shell should display a user
prompt, for example: Shell>, at which a user can enter for example, `ls -l` command, as follows:
`Shell> ls -l`. Next, your shell creates a child process to execute this command. Finally, when its
execution is finished, it prompts for the next command from the user.

A Unix shell is a command-line interpreter that provides a traditional user interface for the Unix
operating system and for Unix-like systems. The shell can run in two modes: interactive and batch.
In the shell interactive mode, you start the shell program, which displays a prompt (e.g. Shell>)
and the user of the shell types commands at the prompt. Your shell program executes each of these
commands and terminates when the user enters the exit command at the prompt. In the shell batch
mode, you start the shell by calling your shell program and specifying a batch file to execute the
commands included in it. This batch file contains the list of commands (on separate lines) that the
user wants to execute. In batch mode, you should not display a prompt, however, you will need to
echo each line you read from the batch file (print it) before executing it. This feature in your program
is to help debugging and testing your code. Your shell terminates when the end of the batch file is
reached or the user types Ctrl-D.

Commands submitted by the user may be executed in either the foreground or the background.
User appends an & character at the end of the command to indicate that your shell should execute
it in the background. For example, if the user of your shell program enters Shell> myCommand,
your shell program should execute ”myCommand” in the foreground, which means that your shell
program waits until the execution of this command completes before it proceeds and displays the
next prompt. However, if the user of your shell program enters Shell> myCommand &, your shell program
should execute ”myCommand” in background, which means that your shell program starts
executing the command, and then immediately returns and displays the next prompt while the com-
mand is still running in the background.

##Program Details

Your C program must be invoked exactly as follows:

`Shell [batchFile]`

where the batchFile is an optional argument to your shell program. If it is present, your shell will
read each line of the batchFile for commands to be executed. If not present, your shell will run in
interactive mode by printing a prompt to the user at stdout and reading the command from stdin.

For example, if you type the following `Shell /home/cs333/testCase1.txt`
your program will read commands from `/home/cs333/testCase1.txt` until the exit command
is read or end of file is reached.

Your shell should handle errors in a decent way. Your C program should not core dump, hang indefinitely,
or prematurely terminate. Your program should check for errors and handle them by printing an un-
derstandable error message and either continue processing or exit, depending upon the situation.

The following cases are considered errors and you need to handle them in your program:

  •    An incorrect number of command line arguments to your shell program.

  • The batch file does not exist or cannot be opened.

In the following cases, you should print a message to the user (stderr) and continue reading the
following commands:

  • A command does not exist or cannot be executed.

  • A very long command line (over 512 characters).

These cases are not errors, however, you still need to handle them in your shell program:

  • An empty command line.

  • Multiple white spaces on a command line.

  • White space before or after the & character.

  • The input batch file ends without and exit command or the user types Ctrl-D as command
in the interactive mode.

  • If the & character appears in the middle of a line, then the job should not be placed in the
background; instead, the & character is treated as one of the job arguments.
