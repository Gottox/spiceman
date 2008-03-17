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
#include <strings.h>
#include <string.h>

#include "common.h"
#include "ui.h"

void
wordwrap(char *str, int width) {
	int i;
	char *lastblank = NULL, *p;

	for(p = str, i = 0; *p; p++, i++) {
		if(*p == '\t' || *p == ' ')
			lastblank = p;
		if(i > width && lastblank) {
			p = lastblank;
			lastblank = NULL;
			i = 0;
			*p = '\n';
		}
	}
}

void
ui_help() {
	APPLETUSAGE("db");
	fputs("	-s	short output\n", stderr);
	fputs("	-m	medium output (default)\n", stderr);
	fputs("	-f	full output\n", stderr);
	fputs("	-w <w>	maximal text width (0 = no line wrapping)\n", stderr);
}

int
ui(int argc, char *argv[], FILE *in, FILE *out) {
	int width = 80;
	char *arg;
	char action = 0;
	struct Package pkg;

	ARGBEGIN {
	case 'w':
		if(!(arg = ARGVAL()))
			goto argerr;
		width = atoi(arg);
		break;
	case 's':
	case 'f':
	case 'm':
		if(action)
			goto argerr;
		action = ARGCHR();
		break;
	argerr:
	default:
		ui_help();
		return EXIT_FAILURE;
	} ARGEND

	bzero(&pkg, sizeof(pkg));
	while(getpkg(&pkg, in) > 0) {
		fprintf(stderr, "[%c] %s-%s-%i (%s)\n", pkg.type, pkg.name, pkg.ver, pkg.rel, pkg.repo);
		if(action != 's') {
			if(width)
				wordwrap(pkg.desc, width);
			fputs(pkg.desc, stderr);
			fputc('\n', stderr);
			if(action == 'f') {
				fputs("Depencies: ", stderr);
				fputs(pkg.dep, stderr);
				fputs("\nConflicts: ", stderr);
				fputs(pkg.conflict, stderr);
				fputs("\nProvides: ", stderr);
				fputs(pkg.prov, stderr);
				fputc('\n', stderr);
			}
		}
		fputc('\n', stderr);
	}
	freepkg(&pkg);
	return EXIT_SUCCESS;
}
