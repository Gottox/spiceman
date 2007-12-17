/* spiceman - suckless package management tools
 * Copyright (C) Enno Boland
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"

int checksyntax(char *line) {
	return 1;
}

int cmdchain(int cmdc, struct Cmd *cmd) {
	FILE *in, *out;
	int i, fd[2], pid, retval, status;

	in = stdin;
	for(i = 0; i < cmdc-1; i++) {
		if(pipe(fd) != 0)
			eprint("Cannot create pipe: ");
		if(!(out = fdopen(fd[1], "w")))
			eprint("Cannot open pipe for writing: ");
		pid = fork();
		if(pid < 0)
			eprint("Cannot fork: ");
		else if(pid == 0)
			exit(cmd[i].function(cmd[i].argc,cmd[i].argv,in,out));
		fclose(out);
		if(in != stdin)
			fclose(in);
		if(!(in = fdopen(fd[0], "r")))
			eprint("Cannot open pipe for reading: ");
	}
	out = stdout;
	retval = cmd[i].function(cmd[i].argc,cmd[i].argv,in,out);
	while(wait(&status) != -1);
	return retval;
}

void
eprint(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	perror(NULL);
	va_end(ap);
	exit(EXIT_FAILURE);
}
