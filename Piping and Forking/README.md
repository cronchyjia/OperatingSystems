## Goals of this assignment

1. Write a program (`fetch-info.c`) that prints out interesting statistics
about the environment in which it is run, or statistics about a single
process running in that environment.

2. Write a program (`pipe4.c`) that executes up to four shell
instructions, piping the output of one to the input of the next.

---

## Part 1: `fetch-info.c`

Every Unix filesystem includes a `/proc` directory.  This directory
contains a great deal of information, as text files, that relate to
the properties of the operating system – its memory usage, its kernel
version number, scheduling and filesystem information, and so on.
Details about processes running on the system are also available in
subdirectories of the `/proc` directory.

- If run without arguments, the program should print to the console
  the following information, in order (one item per line):
 
  1. The model name of the CPU

  2. The number of cores

  3. The version of Linux running in the environment

  4. The total memory available to the system, in kilobytes

  5. The 'uptime' (the amount of time the system has been running, expressed in terms of days, hours, minutes and seconds).

- If run with a numerical argument, the program should print to the
  console information about the corresponding process:

  1. The name of the process

  2. The console command that started the process (if any) 

  3. The number of threads running in the process

  4. The total number of context switches that have occurred during the running of the process.
   
If there is no running process that corresponds to the numerical
argument provided, the program should quit with an error message.
Regardless of which 'mode' the program runs in, it should terminate
once it has finished printing the appropriate information.


## Part 2: `pipe4.c`

Unix commands that are executed in a shell can take advantage of
'piping' – that is, the output of one command can be used as the input
of another.  For example, the command `ls -1` will list the contents
of the current directory, one entry per line.  The command `wc -l`
counts the number of lines in the input, and outputs that number as an
integer.  We can specify that the output of `ls -1` should be used as
the input of `wc -l` by joining the two commands with a pipe:

`ls -1 | wc -l`

The resulting compound command prints the number of entries in the
current directory.

Write a program (`pipe4.c`) that accepts up to four individual
commands, one per line (pressing enter or return at the end of each).
The user may indicate that they have no more commands to enter by
pressing return on a line without entering a command first.  If this
happens, or if the user enters four commands, your program must then
execute each of the four instructions, transferring the output of one
to the input of the next.  The output of the final command in the
sequence must be sent to the console.  For example, if the program is
run as follows:

`./pipe4`

and then the following is entered (where the &#9166; symbol indicates
pressing enter):

`/bin/ls -1` &#9166;

`/bin/wc -l` &#9166;

&#9166;

The program should print out the number of directory entries in the
current directory (as an integer) and then quit.

The program should only execute as many instructions as were entered
(a maximum of four) – if the enter key is pressed without first
entering a command, the program should simply quit.  If the user
enters four commands, the program should not prompt for a fifth, but
should immediately begin executing the commands.