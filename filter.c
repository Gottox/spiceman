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

void exactmatch(const char *s, FILE *in, FILE *out) {
	int j;
	struct Package pkg;
	const char *p;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
		for(j = 0; s[j] && pkg.name[j] &&
			s[j] == pkg.name[j]; j++);
		if(s[j] == 0 && s[j] == pkg.name[j]) {
			putpkg(&pkg, out);
			continue;
		}
		else if(s[j] != '-' || pkg.name[j++] != 0)
			continue;
		for(p = s + j, j = 0; p[j] && pkg.ver[j] && 
				p[j] == pkg.ver[j]; j++);
		if(p[j] == 0 && p[j] == pkg.ver[j]) {
			putpkg(&pkg, out);
			continue;
		}
		else if(p[j] != '-' || pkg.ver[j++] != 0)
			continue;
		if(atoi(p + j) == pkg.rel)
			putpkg(&pkg, out);
	}
	freepkg(&pkg);
}

void repository(const char *s, FILE *in, FILE *out) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0)
		if(!strcmp(pkg.repo, s))
			putpkg(&pkg, out);
	freepkg(&pkg);
}

void search(const char *s, FILE *in, FILE *out) {
	char buf[128];
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
		snprintf(buf, LENGTH(buf), "%s-%s-%i",
				pkg.name, pkg.ver, pkg.rel);
		if(!fnmatch(s, buf, 0))
			putpkg(&pkg, out);
	}
	freepkg(&pkg);
}

void type(const char *s, FILE *in, FILE *out) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0)
		if(strchr(s, pkg.type))
			putpkg(&pkg, out);
	freepkg(&pkg);
}

void unique(FILE *in, FILE *out) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
	
	}
	freepkg(&pkg);
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
	char action = 0;
	char *arg = NULL;

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
	switch(action) {
	case 't':
		type(arg, in, out);
		break;
	case 'R':
		repository(arg, in, out);
		break;
	case 's':
		search(arg, in, out);
		break;
	case 'e':
		exactmatch(arg, in, out);
		break;
	case 'n':
		unique(in, out);
		break;
	}
	return EXIT_SUCCESS;
}
