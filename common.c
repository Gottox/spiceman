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

int cmdchain(int cmdc, ...) {
	va_list ap;
	struct Cmd *c;
	FILE *in, *out;
	int fd[2], pid, retval, status;

	in = stdin;
	va_start(ap, cmdc);
	while(--cmdc) {
		if(cmdc - 1 && pipe(fd) != 0)
			eprint("Cannot create pipe");
		if(!(out = fdopen(fd[0], "w")))
			eprint("Cannot open pipe for writing");
		c = va_arg(ap, struct Cmd *);
		pid = fork();
		if(pid < 0)
			eprint("Cannot fork");
		else if(pid == 0)
			exit(c->function(c->argc,c->argv,in,out));
		fclose(out);
		if(in != stdin)
			fclose(in);
		if(!(in = fdopen(fd[1], "r")))
			eprint("Cannot open pipe for reading");
	}
	out = stdout;
	c = va_arg(ap, struct Cmd *);
	retval = c->function(c->argc,c->argv,in,out);
	while(wait(&status) != -1);
	va_end(ap);
	return retval;
}

int spawn(int argc, char *argv[], FILE *in, FILE *out) {
	
	return 0;
}

void
eprint(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}
