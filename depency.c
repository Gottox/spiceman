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

#include "common.h"
#include "depency.h"
#include "db.h"

void depency_help() {
	APPLETUSAGE("depency");
	fputs("	-b	backward depencies\n", stderr);
	fputs("	-n	only print not installed depencies\n", stderr);
	fputs("	-r	recursive walk to the depencies\n", stderr);
}

int depency(int argc, char *argv[], FILE *in, FILE *out) {
	return 0;
}
