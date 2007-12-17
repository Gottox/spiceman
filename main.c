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
#include <string.h>
#include <libgen.h>

#include "common.h"

#include "db.h"
#include "delete.h"
#include "extract.h"
#include "depency.h"

void help();
int main_applet(int argc, char *argv[], FILE *in, FILE *out);
void version();

struct Applet {
	Cmdfunction function;
	void (*help)();
	char *name;
};

static struct Applet applets[] = {
	{ db,		db_help,	"db" },
	{ delete,	delete_help,	"delete" },
	{ extract,	extract_help,	"extract" },
	{ depency,	depency_help,	"depency" },
	{ main_applet,	help,		NULL },
};

int main_applet(int argc, char *argv[], FILE *in, FILE *out) {
	int i;

	for(i = 0; i < argc; i++) {
	
	}
	return EXIT_FAILURE;
}

void help() {
	int i;

	puts("spiceman [-I] [-h] [-v] [-i pkg|-r pkg|-s pkg]");
	puts("	-I	use installed packages as db source.");
	puts("	-h	This help message");
	puts("	-v	Version");
	puts("	-i	Install packages");
	puts("	-r	Remove packages");
	puts("	-s	search package");
	for(i = 0; i < LENGTH(applets)-1; i++) {
		applets[i].help();
	}
}

void version() {
	puts("spiceman-" VERSION " - suckless package management tools");
}

int main(int argc, char *argv[]) {
	int i;
	char *bn;
	int showhelp, retval;
	FILE *in;

	showhelp = argc <= 1;
	in = stdin;
	bn = basename(argv[0]);
	for(i = 0; i < argc && !showhelp; i++) {
		if(argv[i][0] != '-')
			continue;
		switch(argv[i][1]) {
		case 'v':
			version();
			exit(EXIT_FAILURE);
		case 'h':
			showhelp = 1;
			break;
		case 'R':
			if(++i < argc)
				showhelp = 1;
			else if(!(in = fopen(argv[i],"r")))
				eprint("Cannot open `%s`.",argv[i]);
		}
	}
	for(i = 0; i < LENGTH(applets) - 1; i++)
		if((strncmp(bn, APPLETPREFIX, LENGTH(APPLETPREFIX)-1) == 0 &&
				strcmp(bn + LENGTH(APPLETPREFIX)-1, applets[i].name) == 0) ||
				(argc > 1 && strcmp(argv[1], applets[i].name) == 0))
			break;
	if(showhelp) {
		version();
		applets[i].help();
		retval = EXIT_FAILURE;
	}
	else if(applets[i].name == NULL || strcmp(argv[1], applets[i].name) != 0)
		retval = applets[i].function(argc, argv, in, stdout);
	else
		retval = applets[i].function(argc-1, argv+1, in, stdout);
	fclose(in);
	return retval;
}
