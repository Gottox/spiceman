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
	fputs("	-B	list binary packages only\n", stderr);
	fputs("	-S	list source packages only\n", stderr);
	fputs("	-L	list installed packages only\n", stderr);
	fputs("	-l	list all packages\n", stderr);
	fputs("	-s	sync with repository\n", stderr);
}

int db(int argc, char *argv[], FILE *in, FILE *out) {
	fputs("db",out);
	return 0;
}
