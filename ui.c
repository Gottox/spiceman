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
		else if(*p == '\n')
			i = 0;

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
	fputs("	-N	seperate pkg by 0-byte\n", stderr);
}

int
ui(int argc, char *argv[], FILE *in, FILE *out) {
	unsigned int i, width = 80;
	char *arg, seperator = '\n';
	char action = 0;
	struct Package pkg;

	ARG {
	case 'N':
		seperator = '\0';
		break;
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
	}

	bzero(&pkg, sizeof(pkg));
	for(i = 1; getpkg(&pkg, in) > 0; i++) {
		fprintf(out, "%i. [%c] %s-%s-%i (%s)", i, pkg.type, pkg.name,
				pkg.ver, pkg.rel, pkg.repo);
		if(action != 's') {
			if(width)
				wordwrap(pkg.desc, width);
			fputc('\n', out);
			fputs(pkg.desc, out);
			fputs("\nURL: ", out);
			fputs(pkg.infourl, out);
			fputc('\n', out);
			if(action == 'f') {
				fputs("Depencies: ", out);
				fputs(pkg.dep, out);
				fputs("\nConflicts: ", out);
				fputs(pkg.conflict, out);
				fputs("\nProvides: ", out);
				fputs(pkg.prov, out);
				fputc('\n', out);
			}
		}
		fputc(seperator, out);
	}
	freepkg(&pkg);

	fprintf(stderr, "%i packages\n", i - 1);

	return EXIT_SUCCESS;
}
