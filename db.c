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
#include "db.h"

#define BUFSIZE 128

void db_help() {
	APPLETUSAGE("db");
	fputs("	-i	list all installed packages\n", stderr);
	fputs("	-p	list all packages in database\n", stderr);
	fputs("	-o	list other version of pkgs read from stdin\n", stderr);
	fputs("	-d	show depencies\n", stderr);
	fputs("	-t	calculate recursive depencies\n", stderr);
	fputs("	-r	calculate reverse depencies\n", stderr);
}

int db(int argc, char *argv[], FILE *in, FILE *out) {
	char action = 0;

	if(argc == 0)
		action = 'p';
	else if(argv[0][0] == '-')
		action = argv[0][1];
	switch(action) {
	case 'i':
	case 'p':
		return putdb(out, action);
	case 'o':
		return alternate(in, out);
	default:
		db_help();
		return EXIT_FAILURE;
	}
}

int
putdb(FILE *out, char action) {
	int r;
	FILE *db;
	char *src;
	struct Package pkg;

	src = action == 'i' ? DBPREFIX "/installed" :  DBPREFIX "/packages";
	if(!(db = fopen(src, "r")))
		eprint(1, "Cannot open database `%s`: ", src);
	bzero(&pkg, sizeof(pkg));
	while((r = getpkg(&pkg, db) > 0)) {
		putpkg(&pkg, out);
	}
	freepkg(&pkg);
	if(r < 0)
		eprint(0, "You can start crying now.\nMalformed Package in Database: %s", "TODO");
	fclose(db);
	return EXIT_SUCCESS;
}

int
alternate(FILE *in, FILE *out) {
	FILE *db;
	struct Package pkg;
	struct Package dbpkg;
	long *found = 0, pos;
	int fsize = 0, i;

	bzero(&pkg, sizeof(pkg));
	bzero(&dbpkg, sizeof(pkg));
	if(!(db = fopen(DBPREFIX "/packages", "r")))
		eprint(1, "Cannot open database `" DBPREFIX "/packages`: ");
	while(getpkg(&pkg, in) > 0) {
		while(getpkg(&dbpkg, db) > 0) {
			pos = ftell(db);
			for(i = 0; i < fsize && found[i] != pos; i++);
			if(fsize && i != fsize)
				continue;
			if(strcmp(pkg.name, dbpkg.name) ||
					strcmp(pkg.ver, dbpkg.ver))
				continue;
			if(fsize % BUFSIZE == 0) {
				found = erealloc(found, fsize + BUFSIZE);
			}
			found[fsize++] = pos;
			putpkg(&dbpkg, out);
		}
		rewind(db);
	}
	return EXIT_SUCCESS;
}
