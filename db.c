/* spiceman - suckless package management tools
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
#include "db.h"

void db_help() {
	APPLETUSAGE("db");
	fputs("	-i	list installed packages only\n", stderr);
	fputs("	-p	list repository packages\n", stderr);
	fputs("	-s	sync with repository\n", stderr);
	fputs("	-o	find all versions of the same packages\n", stderr);
}

int db(int argc, char *argv[], FILE *in, FILE *out) {
	struct Package pkg;
	char sep[2];
	FILE *f = fopen("pkglist","r");
	fread(sep, sizeof(sep), 1, f);
	getpkg(&pkg, f, sep);
	puts(pkg.repo);
	putpkg(&pkg, stdout, sep);
	fclose(f);
	return 0;
}
