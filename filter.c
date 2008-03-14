/* spiceman - distributed package management tools
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <fnmatch.h>

#include "common.h"
#include "filter.h"

#define ARRAYSIZE 1

void filter_help() {
	APPLETUSAGE("filter");
	fputs("	-n	make packages unique, use newest\n", stderr);
	fputs("	-t <t>	filters for types\n", stderr);
	fputs("	-R <r>	filter repository\n", stderr);
	fputs("	-s <p>	search in package-name, -version and -release\n", stderr);
	fputs("	-e <p>	exact match\n", stderr);
}

int filter(int argc, char *argv[], FILE *in, FILE *out) {
	struct Package pkg;
	char action = 0;
	char *arg = NULL;
	bzero(&pkg, sizeof(pkg));

	ARGBEGIN {
	case 't':
	case 'R':
	case 's':
	case 'e':
		if(!(arg = ARGVAL()))
			goto argerr;
	case 'n':
		if(action)
			goto argerr;
		action = ARGCHR();
		break;
	argerr:
	default:
		filter_help();
		return EXIT_FAILURE;
	} ARGEND;
	if(argc > 1 && argc != ARGC())
		goto argerr;

	switch(action) {
	case 't':
		while(getpkg(&pkg, in, seperator))
			if(strchr(arg, pkg.type))
				putpkg(&pkg, out, seperator);
		break;
	case 's':
		break;
	case 'R':
	case 'e':
		break;
	case 'n':
		break;
	}
	return EXIT_SUCCESS;
}
