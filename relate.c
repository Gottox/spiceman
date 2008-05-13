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
#include <sys/wait.h>

#include "common.h"
#include "relate.h"

#include "filter.h"
#include "db.h"

static struct Cmd alternatechain[] = {
	{ db,		0,	{ NULL } },
	{ filter,	2,	{ "-o", NULL } },
};

int
alternate(FILE *in, FILE *out) {
	int status;
	char buf[BUFSIZ];
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
		snprintf(buf, sizeof(buf), "%s==%s", pkg.name, pkg.ver);
		puts(buf);
		alternatechain[1].argv[1] = buf;
		cmdchain(LENGTH(alternatechain), alternatechain, NULL, out);
		while(wait(&status) != -1);
	}
	freepkg(&pkg);
	return EXIT_SUCCESS;
}

void relate_help() {
	APPLETUSAGE("relate");
	fputs("	-d	show depencies\n", stderr);
	fputs("	-t	calculate recursive depencies\n", stderr);
	fputs("	-r	calculate reverse depencies\n", stderr);
	fputs("	-o	finds other versions of the same program\n", stderr);
}

int relate(int argc, char *argv[], FILE *in, FILE *out) {
	char action = 0;

	ARG {
	case 'd':
	case 't':
	case 'r':
	case 'o':
		if(!action) {
			action = ARGCHR();
			break;
		}
		/* no break, for error handling*/
	argerr:
	default:
		relate_help();
		return EXIT_FAILURE;
	}
	if(argc > 1 || argc != ARGC())
		goto argerr;
	switch(action) {
	case 'd':
	case 't':
	case 'r':
		break;
	case 'o':
		alternate(in, out);
		break;
	}
	return EXIT_SUCCESS;
}
