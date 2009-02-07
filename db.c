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
#include <unistd.h>

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
	fputs("	-I	list builtin packages\n", stderr);
	fputs("	-p	list all packages in database\n", stderr);
}

int
db(int argc, char *argv[]) {
	char action = 0;
	int r, i;
	FILE *db;
	char *src;
	struct Package pkg;

	if(argc == 0)
		action = 'p';
	else if(argv[0][0] == '-')
		action = argv[0][1];
	switch(action) {
	case 'I':
		for(i = 0; i < LENGTH(initpkg); i++)
			putpkg(&initpkg[i]);
		break;
	case 'i':
	case 'p':
		if(action == 'i')
			src = DBPREFIX "/installed";
		else
			src = DBPREFIX "/packages";
		if(!(db = fopen(src, "r")))
			die(1, "Cannot open database `%s`: ", src);
		dup2(fileno(db), STDIN_FILENO);
		bzero(&pkg, sizeof(pkg));
		while((r = getfreepkg(&pkg)) > 0)
			putpkg(&pkg);
		if(r < 0)
			die(0, "You can start crying now.\n"
					"Malformed Package in Database: ´%s´\n",
					src);
		fclose(db);
		break;
	default:
		db_help();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
