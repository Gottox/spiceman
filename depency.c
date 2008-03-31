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

#include "common.h"
#include "depency.h"

#include "filter.h"
#include "db.h"

void depency_help() {
	APPLETUSAGE("depency");
	fputs("	-d	show depencies\n", stderr);
	fputs("	-t	calculate recursive depencies\n", stderr);
	fputs("	-r	calculate reverse depencies\n", stderr);
}

int depency(int argc, char *argv[], FILE *in, FILE *out) {
	char action = 0;

	ARG {
	case 'd':
	case 't':
	case 'r':
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
	return EXIT_SUCCESS;
}
