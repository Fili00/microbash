# µBash
University project implemented with another student that implements a very simplified shell. \
The shell implements the various commands via system calls and the built-in cd command.
## Purpose
The purpose of the project was to practice the use of system calls `fork`, `pipe` and `exec` since a key constraint to be met was not to use
the system function and in I/O redirection.
## Syntax
Since this is a simplified version, command parsing is also simplified by adopting a more limited syntax:
* There must be a space before and after the | symbol.
* No spaces must be present between the `>` or `<` symbol and the file where to do the I/O redirection 
<!-- end of the list -->
For more details see [micro-bash.pdf](micro-bash.pdf)

