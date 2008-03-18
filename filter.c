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

int exactmatch(const char *s, struct Package *pkg) {
	int j;
	const char *p;

	for(j = 0; s[j] && pkg->name[j] &&
		s[j] == pkg->name[j]; j++);
	if(s[j] == 0 && s[j] == pkg->name[j])
		return 1;
	else if(s[j] != '-' || pkg->name[j++] != 0)
		return 0;
	for(p = s + j, j = 0; p[j] && pkg->ver[j] && 
			p[j] == pkg->ver[j]; j++);
	if(p[j] == 0 && p[j] == pkg->ver[j])
		return 1;
	else if(p[j] != '-' || pkg->ver[j++] != 0)
		return 0;
	if(atoi(p + j) == pkg->rel)
		return 1;
	return 0;
}

int repomatch(const char *s, struct Package *pkg) {
	return strcmp(pkg->repo, s) == 0;
}

int typematch(const char *s, struct Package *pkg) {
	return strchr(s, pkg->type) != 0;
}

void unique(FILE *in, FILE *out) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
	
	}
	freepkg(&pkg);
}

int wildcardmatch(const char *s, struct Package *pkg) {
	char buf[128];

	snprintf(buf, LENGTH(buf), "%s-%s-%i",
			pkg->name, pkg->ver, pkg->rel);
	return fnmatch(s, buf, 0) == 0;
}

void filter_help() {
	APPLETUSAGE("filter");
	fputs("	-n	make packages unique, use newest\n", stderr);
	fputs("	-t <t>	filters for types\n", stderr);
	fputs("	-R <r>	filter repository\n", stderr);
	fputs("	-s <p>	search in package-name, -version and -release\n", stderr);
	fputs("	-e <p>	exact match\n", stderr);
}

int filter(int argc, char *argv[], FILE *in, FILE *out) {
	int match = 0;
	char action = 0;
	char *arg = NULL;
	struct Package pkg;

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
	if(action == 'n')
		unique(in, out);
	else {
		bzero(&pkg, sizeof(pkg));
		while(getpkg(&pkg, in) > 0) {
			switch(action) {
			case 't':
				match = typematch(arg, &pkg);
				break;
			case 'R':
				match = repomatch(arg, &pkg);
				break;
			case 's':
				match = wildcardmatch(arg, &pkg);
				break;
			case 'e':
				match = exactmatch(arg, &pkg);
				break;
			}
			if(match)
				putpkg(&pkg, out);
		}
	}
	freepkg(&pkg);
	return EXIT_SUCCESS;
}
