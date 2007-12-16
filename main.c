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
#include "extract.h"
#include "delete.h"

#define APPLETPREFIX "sp-"
#define LENGTH(x) sizeof(x)/sizeof(x[0])

struct Applet {
	Cmdfunction function;
	char *name;
};

static struct Applet applets[] = {
	{ db,	"db" },
	{ extract,	"extract" },
	{ delete,	"delete" },
};

void help() {
	int i;
	char *argv[2] = { NULL,"-h" };

	puts("spiceman-" VERSION " - suckless package management tools");
	puts("	-h	This help message");
	puts("	-i	Install packages");
	puts("	-r	Remove packages");
	puts("	-s	search package");
	for(i = 0; i < LENGTH(applets); i++) {
		printf("\t%s\n",applets[i].name);
		argv[0] = applets[i].name;
		applets[i].function(2,argv,stdin,stdout);
	}
}

int main(int argc, char *argv[]) {
	int i;
	char *bn;

	bn = basename(argv[0]);
	for(i = 0; i < LENGTH(applets); i++) {
		if(strncmp(bn, APPLETPREFIX, LENGTH(APPLETPREFIX)-1) == 0 &&
				strcmp(bn + LENGTH(APPLETPREFIX)-1, applets[i].name) == 0)
			return applets[i].function(argc, argv, stdin, stdout);
		else if(argc > 1 && strcmp(argv[1], applets[i].name) == 0)
			return applets[i].function(argc-1, argv+1, stdin, stdout);
	}

	for(i = 1; i < argc; i++) {
		if(argv[i][0] != '-') {
			help();
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
