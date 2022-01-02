# Linux Terminal
This is a shell-like application for Linux written in C, as a project from second bachelor year.
It has two commands implemented by the author: cd and tail (which is neither optimal or complete).
The shell supports: history feature, pipeline handling and redirection of input and output.

For compiling the source code, while having the gcc installed, one should use the following command in the terminal:

  `gcc -o shell.out shell.c -lreadline`
