/* spiceman - suckless package manager tools
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

#include "spiceman.h"

#include "db.h"
#include "install.h"
#include "remove.h"

#define APPLETPREFIX "sp-"
#define LENGTH(x) sizeof(x)/sizeof(x[0])

struct Applet {
	int (*function)(int, char *[]);
	char *name;
};

static struct Applet applets[] = {
	{ db, "db" },
	{ ins, "ins" },
	{ db, "db" },
};

void help() {
	int i;
	char *argv[2] = { NULL,"-h" }

	puts("Applets:");
	for(i = 0; i < LENGTH(applets); i++) {
		printf("%s\n",applets[i].name);
		applets[i].function(2.)
	}
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	int i;
	char *bn;

	bn = basename(argv[0]);
	for(i = 0; i < LENGTH(applets); i++) {
		if(strncmp(bn, APPLETPREFIX, LENGTH(APPLETPREFIX)-1) == 0 &&
				strcmp(bn + LENGTH(APPLETPREFIX)-1,applets[i].name) == 0)
			return applets[i].function(argc,argv);
		else if(argc > 1 && strcmp(argv[1],applets[i].name) == 0)
			return applets[i].function(argc-1,argv+1);
	}

	help();

	for(i = 1; i < argc; i++) {
		
	}
	return 0;
}
