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
#include <stdio.h>

#define APPLETPREFIX "sp-"

#define LENGTH(x)		sizeof(x)/sizeof(x[0])
#define APPLETUSAGE(a,u)	puts("sp-" a "|spiceman " a " " u);
#define INFILEHELP		puts("	-R <f>	read packages from file (default: stdin)");

typedef int (*Cmdfunction)(int, char *[], FILE *in, FILE *out);

struct Cmd {
	Cmdfunction function;
	int argc;
	char **argv;
};

int cmdchain(int cmdc, ...);
void eprint(const char *format, ...);
int spawn(int argc, char *argv[], FILE *in, FILE *out);
