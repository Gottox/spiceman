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
	int len = 0, j;
	struct Package pkg;
	char action = 0;
	char *arg = NULL, *p;
	char needle[512];
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
	if(argc <= 0 || argc != ARGC())
		goto argerr;
	if(arg)
		len = strlen(arg);
	while(getpkg(&pkg, in) > 0)
		switch(action) {
		case 't':
			if(strchr(arg, pkg.type))
				putpkg(&pkg, out);
			break;
		case 'R':
			if(!strcmp(pkg.repo, arg))
				putpkg(&pkg, out);
			break;
		case 's':
			snprintf(needle, sizeof(needle), "%s-%s-%i",
					pkg.name, pkg.ver, pkg.rel);
			if(!fnmatch(arg, needle, 0))
				putpkg(&pkg, out);
			break;
		case 'e':
			for(p = arg, j = 0; p[j] && pkg.name[j] &&
				p[j] == pkg.name[j]; j++);
			if(p[j] == 0 && p[j] == pkg.name[j]) {
				putpkg(&pkg, out);
				break;
			}
			else if(p[j] != '-' || pkg.name[j++] != 0)
				break;
			for(p += j, j = 0; p[j] && pkg.ver[j] && 
					p[j] == pkg.ver[j]; j++);
			if(p[j] == 0 && p[j] == pkg.ver[j]) {
				putpkg(&pkg, out);
				break;
			}
			else if(p[j++] != '-' || pkg.ver[j++] != 0)
				break;
			if(atoi(p + j) == pkg.rel)
				putpkg(&pkg, out);
			break;
		case 'n':
			break;
		}
	freepkg(&pkg);
	return EXIT_SUCCESS;
}
