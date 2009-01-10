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

struct Pkglst {
	struct Package pkg;
	struct Pkglst *next;
};

enum Operator {
	EQUAL, NOTEQUAL, LESSER, GREATER, LESSEREQUAL, GREATEREQUAL
};

int operatormatch(const char *s) {
	int i, result;
	int operator;
	struct Package pkg;
	const char *release;
	char buf[BUFSIZ];

	bzero(&pkg, sizeof(pkg));
	while(getfreepkg(&pkg) > 0) {
		for(i = 0; pkg.name[i] && pkg.name[i] == s[i]; i++);
		if(pkg.name[i] != 0)
			continue;
		switch(s[i]) {
		case '=':
		case '-':
			operator = EQUAL;
			break;
		case '<':
			operator = LESSER;
			break;
		case '>':
			operator = GREATER;
			break;
		case '!':
			operator = NOTEQUAL;
			break;
		case 0:
			putpkg(&pkg);
			/* No break here */
		default:
			continue;
		}
		if(s[i] != '-' && s[i + 1] == '=') {
			switch(operator) {
			case LESSER:
				operator = LESSEREQUAL;
				break;
			case GREATER:
				operator = GREATEREQUAL;
				break;
			}
			i++;
		}
		i++;
		if((release = strchr(&s[i], '-'))) {
			strncpy(buf, &s[i], MIN(BUFSIZ, release - &s[i]));
			result = pkgcmp(NULL, buf, atoi(release),
					NULL, pkg.ver, pkg.rel);
			if(strchr(buf, '-'))
				continue;
		}
		else
			result = pkgcmp(NULL, &s[i], 0,
					NULL, pkg.ver, 0);
		switch(operator) {
		case EQUAL:
			result = result == 0;
			break;
		case NOTEQUAL:
			result = result != 0;
			break;
		case LESSER:
			result = result > 0;
			break;
		case GREATER:
			result = result < 0;
			break;
		case LESSEREQUAL:
			result = result >= 0;
			break;
		case GREATEREQUAL:
			result = result <= 0;
			break;
		}
		if(result)
			putpkg(&pkg);
	}
	return 0;
}

int repomatch(const char *s) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getfreepkg(&pkg))
		if(strcmp(pkg.repo, s) == 0)
			putpkg(&pkg);
	return EXIT_SUCCESS;
}

int typematch(const char *s) {
	struct Package pkg;

	bzero(&pkg, sizeof(pkg));
	while(getfreepkg(&pkg))
		if(strchr(s, pkg.type))
			putpkg(&pkg);
	return EXIT_SUCCESS;
}

int
unique(int vercmp) {
	/* BUG: Does not return the newest version. Why? */
	int cmp;
	struct Package pkg;
	struct Pkglst *n, *l, *list = NULL, *prev = NULL;

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg) > 0) {
		for(l = list, cmp = 1; l && (cmp = (vercmp &&
				pkgcmp(l->pkg.name, l->pkg.ver, l->pkg.rel,
					pkg.name, pkg.ver, pkg.rel)) != 0 ||
				strcmp(l->pkg.name, pkg.name));
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

	while(list) {
		putpkg(&list->pkg);
		freepkg(&list->pkg);
		l = list;
		list = l->next;
		free(l);
	}
	return EXIT_SUCCESS;
}

int wildcardmatch(const char *p, int fulltext) {
	unsigned int len;
	char buf[BUFSIZ], patternbuf[BUFSIZ] = "*";
	struct Package pkg;

	if(p[0] == '^')
		strncpy(patternbuf, p + 1, LENGTH(patternbuf));
	else
		strncat(patternbuf, p, LENGTH(patternbuf));
	len = strlen(patternbuf);
	if(len < LENGTH(patternbuf) - 1 && patternbuf[len - 1] == '$')
		patternbuf[len - 1] = 0;
	else if(len + 2 < BUFSIZ) {
		patternbuf[len++] = '*';
		patternbuf[len] = 0;
	}

	bzero(&pkg, sizeof(pkg));
	while(getfreepkg(&pkg) > 0) {
		snprintf(buf, LENGTH(buf), "%s-%s-%i", pkg.name,
				pkg.ver, pkg.rel);
		if(fnmatch(patternbuf, buf, 0) == 0)
			putpkg(&pkg);
		else if(fulltext && fnmatch(patternbuf, pkg.desc, 0) == 0)
			putpkg(&pkg);
	}
	return EXIT_SUCCESS;
}

void filter_help() {
	APPLETUSAGE("filter");
	fputs("	-n	make packages unique name and version, use newest\n",
			stderr);
	fputs("	-N	make packages unique name, use newest\n", stderr);
	fputs("	-t <t>	filters for types\n", stderr);
	fputs("	-R <r>	filter repository\n", stderr);
	fputs("	-s <p>	search in package-name, -version and -release\n", stderr);
	fputs("	-S <p>	search in package-name, -version, -release and "
			"-description\n", stderr);
	fputs("	-o <p>	match using operators (e.g. spiceman>" VERSION
			")\n", stderr);
}

int filter(int argc, char *argv[]) {
	char action = 0;
	char *arg = NULL;

	ARG {
	case 't':
	case 'R':
	case 's':
	case 'S':
	case 'o':
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
	switch(action) {
	case 't':
		return typematch(arg);
		break;
	case 'R':
		return repomatch(arg);
		break;
	case 's':
	case 'S':
		return wildcardmatch(arg, action == 'S');
		break;
	case 'o':
		return operatormatch(arg);
		break;
	case 'n':
	case 'N':
		return unique(action != 'N');
		break;
	}
	return EXIT_SUCCESS;
}
