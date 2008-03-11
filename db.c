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
	fputs("	-i	list all installed packages\n", stderr);
	fputs("	-p	list all packages in database\n", stderr);
	fputs("	-o	list other version of pkgs read from stdin\n", stderr);
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
		freepkg(&pkg);
	}
	if(r < 0)
		eprint(0, "You can start crying now.\nMalformed Package in Database: %s", "TODO");
	fclose(db);
	return EXIT_SUCCESS;
}

int
alternate(FILE *in, FILE *out) {
	int i, run = 1;
	FILE *db;
	struct Package pkgs[16]/*, dbpkg*/;

	bzero(pkgs, sizeof(pkgs));
	while(run && !feof(in)) {
		for(i = 0; !feof(in) && i < LENGTH(pkgs); i++)
			if(!getpkg(&pkgs[i], in)) {
				fputs("Defect Package!\n",stderr);
				run = 0;
				i--;
				break;
			}
		if(!(db = fopen(DBPREFIX "/packages", "r")))
			eprint(1, "Cannot open database `" DBPREFIX "/packages`: ");
		/*while(!feof(db)) {
			if(!getpkg(&pkgs[i], in)) {
		}*/
		while(i--)
			freepkg(&pkgs[i]);
	}
	return EXIT_SUCCESS;
}
