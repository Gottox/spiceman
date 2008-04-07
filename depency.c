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

#include "common.h"
#include "depency.h"

#include "filter.h"
#include "db.h"

static struct Cmd dbchain[] = {
	{ db,		0,	{ NULL } },
};

int
alternate(FILE *in, FILE *out) {
	struct Package pkg;
	struct Package dbpkg;
	FILE *db[2];

	bzero(&pkg, sizeof(pkg));
	bzero(&dbpkg, sizeof(dbpkg));

	while(getpkg(&pkg, in) > 0) {
		fpipe(db);
		cmdchain(LENGTH(dbchain), dbchain, NULL, db[1]);
		fclose(db[1]);
		while(getpkg(&dbpkg, db[0]) > 0)
			if(!strcmp(pkg.name, dbpkg.name) && !vercmp(pkg.ver, dbpkg.ver))
				putpkg(&dbpkg, out);
		waitchain(db[0]);
	}
	freepkg(&pkg);
	freepkg(&dbpkg);
	return EXIT_SUCCESS;
}

void depency_help() {
	APPLETUSAGE("depency");
	fputs("	-d	show depencies\n", stderr);
	fputs("	-t	calculate recursive depencies\n", stderr);
	fputs("	-r	calculate reverse depencies\n", stderr);
	fputs("	-o	finds other versions of the same program\n", stderr);
}

int depency(int argc, char *argv[], FILE *in, FILE *out) {
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
		depency_help();
		return EXIT_FAILURE;
	}
	if(argc > 1 || argc != ARGC())
		goto argerr;
	switch(action) {
	case 'o':
		alternate(in, out);
		break;
	}
	return EXIT_SUCCESS;
}
