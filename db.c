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

#include "common.h"
#include "db.h"

void db_help() {
	APPLETUSAGE("db");
	fputs("	-i	list installed packages only\n", stderr);
	fputs("	-p	list packages in database\n", stderr);
}

int db(int argc, char *argv[], FILE *in, FILE *out) {
	char src = 0;
	FILE *db;
	char dbsep[2];
	struct Package pkg;

	if(argc == 0)
		src = 'p';
	else if(argv[0][0] == '-')
		src = argv[0][1];
	switch(src) {
	case 'i':
		db = fopen(DBPREFIX "/installed", "r");
		break;
	case 'p':
		db = fopen(DBPREFIX "/packages", "r");
		break;
	default:
		db_help();
		return EXIT_FAILURE;
	}
	if(!db)
		eprint(1, "Cannot open database file: ");
	SEP(db, dbsep);
	bzero(&pkg,sizeof(pkg));
	fputs(DEFAULTSEP, out);
	while(!feof(db)) {
		if(!getpkg(&pkg, db, dbsep))
			eprint(0, "You can start crying now.\nMalformed Package in Database: %s", "TODO");
		putpkg(&pkg, out, DEFAULTSEP);
		freepkg(&pkg);
	}
	fclose(db);
	return EXIT_SUCCESS;
}
