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

static struct Package initpkg[] = {
	{ NULL, 0, 'r', INITREPONAME, "0.0", 1, INITREPONAME, INITREPOADDR, "",
		"builtin", "", "", "", "", 0, { 0 }, { 0 }, { 0 }, 0, 0 }
};

void
db_help() {
	APPLETUSAGE("db");
	fputs("	-i	list all installed packages\n", stderr);
	fputs("	-I	list builtin packages", stderr);
	fputs("	-p	list all packages in database\n", stderr);
	fputs("	-o	list other version of pkgs read from stdin\n", stderr);
}

int
db(int argc, char *argv[], FILE *in, FILE *out) {
	char action = 0;

	if(argc == 0)
		action = 'p';
	else if(argv[0][0] == '-')
		action = argv[0][1];
	if(argc <= 1 && strchr("iIp", action)) {
		putdb(out, action);
		return EXIT_SUCCESS;
	}
	else {
		db_help();
		return EXIT_FAILURE;
	}
}

void
putdb(FILE *out, char action) {
	int r, i;
	FILE *db;
	char *src;
	struct Package pkg;

	if(action == 'I') {
		for(i = 0; i < LENGTH(initpkg); i++)
			putpkg(&initpkg[i], out);
	}
	else {
		src = action == 'i' ? DBPREFIX "/installed" :  DBPREFIX "/packages";
		if(!(db = fopen(src, "r")))
			eprint(1, "Cannot open database `%s`: ", src);
		bzero(&pkg, sizeof(pkg));
		while((r = getpkg(&pkg, db) > 0)) {
			putpkg(&pkg, out);
		}
		freepkg(&pkg);
		if(r < 0)
			eprint(0, "You can start crying now.\n"
					"Malformed Package in Database: %s", "TODO");
		fclose(db);
	}
}

