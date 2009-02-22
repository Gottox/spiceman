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
#include <ctype.h>

#include "common.h"
#include "validate.h"

static int validatepkg(struct Package *pkg) {
	int result = 1;
	char *p;

	/* TYPE */
	if(strchr("rsb", pkg->type)) {
		result = 0;
		fprintf(stderr, "Warning: %s: Type '%c' is unknown.\n",
				pkg->name, pkg->type);
	}

	/* NAME */
	if(pkg->name[0] == '\0') {
		result = 0;
		fprintf(stderr, "Warning: %s: Package name may not empty.\n",
				pkg->name);
	}
	if(strlen(pkg->name) > 255) {
		result = 0;
		fprintf(stderr, "Warning: %s: Package name is too long.\n",
				pkg->name);
	}
	for(p = pkg->name; *p && (isalnum(*p) || strchr("-_", *p)); p++);
	if(*p) {
		fprintf(stderr, "Warning %s: non-valid ascii code '%u' "
				"found.\n",
				pkg->name, *p);
	}

	/* VERSION */
	if(pkg->ver[0] == '\0') {
		result = 0;
		fprintf(stderr, "Warning: %s: Package version may not empty.\n",
				pkg->name);
	}
	if(strlen(pkg->ver) > 255) {
		result = 0;
		fprintf(stderr, "Warning: %s: Package version is too long.\n",
				pkg->name);
	}

	/* RELEASE */
	if(pkg->rel == 0) {
		result = 0;
		fprintf(stderr, "Warning: %s: Package release may not 0 or "
				"ascii.\n",
				pkg->name);
	}

	/* DESCRIPTION */
	if(strstr(pkg->desc, "\n\n")) {
		result = 0;
		fprintf(stderr, "Warning: %s: Package description may not "
				"contain empty lines.\n",
				pkg->name);
	}

	return result;
}

void validate_help() {
	APPLETUSAGE("validate");
	fputs("	-r	remove deprecated packages from output\n", stderr);
}

int validate(int argc, char *argv[]) {
	struct Package pkg;
	int error = 0;
	int printdeprec = 0;
	
	if(argc == 1 && strcmp("-w", argv[0]))
		printdeprec = 1;
	else if(argc != 0) {
		validate_help();
		return EXIT_FAILURE;
	}

	while(getfreepkg(&pkg)) {
		if(!validatepkg(&pkg)) {
			if(printdeprec)
				putpkg(&pkg);
			error = 1;
		}
		else
			putpkg(&pkg);
	}

	return error;
}
