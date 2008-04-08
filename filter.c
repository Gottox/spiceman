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

struct Pkglst{
	struct Package pkg;
	struct Pkglst *next;
};

int exactmatch(const char *s, FILE *in, FILE *out) {
	int j;
	const char *p;
	struct Package pkg;

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
	return 0;
}

int repomatch(const char *s, FILE *in, FILE *out) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in))
		if(strcmp(pkg.repo, s) == 0)
			putpkg(&pkg, out);
	freepkg(&pkg);

	return EXIT_SUCCESS;
}

int typematch(const char *s, FILE *in, FILE *out) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in))
		if(strchr(s, pkg.type));
			putpkg(&pkg, out);
	freepkg(&pkg);

	return EXIT_SUCCESS;
}

void unique(char action, FILE *in, FILE *out) {
	int cmp;
	struct Package pkg;
	struct Pkglst *n, *l, *list = NULL, *prev = NULL;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
		for(l = list, cmp = 1; l && (cmp =
				pkgcmp(&pkg, &l->pkg, action)) > 0;
				l = l->next)
			prev = l;
		if(l && cmp == 0)
			continue;
		n = erealloc(0, sizeof(struct Pkglst));
		memcpy(&n->pkg, &pkg, sizeof(struct Package));
		if(prev == NULL) {
			n->next = list;
			list = n;
		}
		else {
			n->next = prev->next;
			prev->next = n;
		}
		pkg.buf = NULL;
		pkg.blen = 0;
	}
	freepkg(&pkg);

	while(list) {
		putpkg(&list->pkg, out);
		freepkg(&list->pkg);
		l = list;
		list = l->next;
		free(l);
	}
}

int wildcardmatch(const char *p, int fulltext, FILE *in, FILE *out) {
	unsigned int len;
	char buf[BUFSIZ], patternbuf[BUFSIZ] = "*";
	struct Package pkg;

	if(p[0] == '^')
		strncpy(patternbuf, p + 1, LENGTH(patternbuf));
	else
		strncat(patternbuf, p, LENGTH(patternbuf));
	len = strlen(patternbuf);
	if(patternbuf[len - 1] == '$')
		patternbuf[len - 1] = 0;
	else if(len + 2 < BUFSIZ) {
		patternbuf[len++] = '*';
		patternbuf[len] = 0;
	}

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
		snprintf(buf, LENGTH(buf), "%s-%s-%i", pkg.name,
				pkg.ver, pkg.rel);
		if(fnmatch(patternbuf, buf, 0) == 0)
			putpkg(&pkg, out);
		else if(fulltext && fnmatch(patternbuf, pkg.desc, 0) == 0)
			putpkg(&pkg, out);
	}
	freepkg(&pkg);
	return EXIT_SUCCESS;
}

void filter_help() {
	APPLETUSAGE("filter");
	fputs("	-n	make packages unique name and version, use newest\n", stderr);
	fputs("	-N	make packages unique name, use newest\n", stderr);
	fputs("	-t <t>	filters for types\n", stderr);
	fputs("	-R <r>	filter repository\n", stderr);
	fputs("	-s <p>	search in package-name, -version and -release\n", stderr);
	fputs("	-S <p>	search in package-name, -version, -release and "
			"-description\n", stderr);
	fputs("	-e <p>	exact match\n", stderr);
}

int filter(int argc, char *argv[], FILE *in, FILE *out) {
	char action = 0;
	char *arg = NULL;

	ARG {
	case 't':
	case 'R':
	case 's':
	case 'S':
	case 'e':
		if(!(arg = ARGVAL()))
			goto argerr;
	case 'n':
	case 'N':
		if(action)
			goto argerr;
		action = ARGCHR();
		break;
	argerr:
	default:
		filter_help();
		return EXIT_FAILURE;
	}
	if(argc <= 0 || argc != ARGC())
		goto argerr;
	if(strchr("nNv", action))
		unique(action, in, out);
	else {
		switch(action) {
		case 't':
			return typematch(arg, in, out);
			break;
		case 'R':
			return repomatch(arg, in, out);
			break;
		case 's':
		case 'S':
			return wildcardmatch(arg, action == 'S', in, out);
			break;
		case 'e':
			return exactmatch(arg, in, out);
			break;
		}
	}
	return EXIT_SUCCESS;
}
