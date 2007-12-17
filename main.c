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
#include "filter.h"

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
	{ filter,	filter_help,	"filter" },
	{ main_applet,	help,		NULL },
};

int main_applet(int argc, char *argv[], FILE *in, FILE *out) {
	int i;
	int action;
	int installed = 0;
	struct Cmd cmds[2];
	char *v[1][10];

	action = 0;
	for(i = 0; i < argc && argv[i][0] == '-'; i++) 
		switch(argv[i][1]) {
		case 's':
		case 'i':
		case 'r':
			action = argv[i][1];
			break;
		case 'I':
			installed = 1;
			break;
		default:
			version();
			help();
			return EXIT_FAILURE;
		}
	if(i == argc) 
		eprint("No package do search defined\n");

	switch(action) {
		case 's':
			cmds[0].function = db;
			cmds[0].argc = installed;
			v[0][0] = "-I";
			cmds[0].argv = v[0];
			cmds[1].function = filter;
			cmds[1].argc = 0;
			cmds[0].argv = NULL;
			cmdchain(2, cmds);
			break;
	}
	return EXIT_FAILURE;
}

void help() {
	fputs("spiceman\n", stderr);
	fputs("	-I	use installed packages as db source.\n", stderr);
	fputs("	-h	help message\n", stderr);
	fputs("	-H	help message for all applets\n", stderr);
	fputs("	-v	Version\n", stderr);
	fputs("	-i	Install packages\n", stderr);
	fputs("	-r	Remove packages\n", stderr);
	fputs("	-s	search package\n", stderr);
}

void version() {
	fputs("spiceman-" VERSION " - suckless package management tools\n",stderr);
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
		case 'H':
			showhelp = 2;
			break;
		case 'R':
			if(++i < argc)
				showhelp = 1;
			else if(!(in = fopen(argv[i],"r")))
				eprint("Cannot open `%s`.",argv[i]);
		}
	}
	for(i = 0; i < LENGTH(applets) - 1; i++)
		if((strncmp(bn, APPLETPREFIX, LENGTH(APPLETPREFIX) - 1) == 0 &&
				strcmp(bn + LENGTH(APPLETPREFIX) - 1, applets[i].name) == 0) ||
				(argc > 1 && strcmp(argv[1], applets[i].name) == 0))
			break;
	if(showhelp) {
		version();
		if(showhelp == 2 && i == LENGTH(applets)-1) {
			applets[i].help();
			for(i = 0; i < LENGTH(applets) - 1;i++)
				applets[i].help();
		}
		else
			applets[i].help();
		retval = EXIT_FAILURE;
	}
	else if(applets[i].name == NULL || strcmp(argv[1], applets[i].name) != 0)
		retval = applets[i].function(argc-1, argv+1, in, stdout);
	else
		retval = applets[i].function(argc-2, argv+2, in, stdout);
	fclose(in);
	return retval;
}
