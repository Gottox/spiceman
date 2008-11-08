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
#include <ctype.h>

#include "common.h"
#include "relate.h"

#include "filter.h"
#include "db.h"

static struct Cmd filterchain[] = {
	{ db,		0,	{ NULL } },
	{ filter,	2,	{ "-o", NULL } },
};

void relate_help() {
	APPLETUSAGE("relate");
	fputs("	-d	show depencies\n", stderr);
	fputs("	-t	calculate depency tree\n", stderr);
	fputs("	-r	calculate reverse depencies\n", stderr);
	fputs("	-o	finds other versions of the same program\n", stderr);
}

int relate(int argc, char *argv[]) {
	int i;
	char action = 0;
	char buf[BUFSIZ];
	char *p;
	struct Package pkg;

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
	bzero(&pkg, sizeof(pkg));
	while(getfreepkg(&pkg) > 0) {
		switch(action) {
		case 'd':
			for(p = pkg.dep; *p && !isalnum(*p); p++);
			while(*p) {
				for(i = 0; *p && isalnum(*p) && i < BUFSIZ; p++, i++)
					buf[i] = *p;
				buf[i] = 0;
				puts(buf);
				fflush(NULL);
				filterchain[1].argv[1] = buf;
				cmdchain(LENGTH(filterchain), filterchain);
				for(; *p && !isalnum(*p); p++);
			}
			break;
		case 't':
		case 'r':
			break;
		case 'o':
			snprintf(buf, sizeof(buf), "%s==%s", pkg.name, pkg.ver);
			filterchain[1].argv[1] = buf;
			cmdchain(LENGTH(filterchain), filterchain);
			break;
		}
	}
	return EXIT_SUCCESS;
}
