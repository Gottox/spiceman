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
#include <string.h>
#include <libgen.h>

#include "common.h"

#include "db.h"
#include "depency.h"
#include "delete.h"
#include "download.h"
#include "filter.h"
#include "install.h"
#include "ui.h"

#define APPLET(x) { x, x ## _help, #x }

void help();
int main_applet(int argc, char *argv[], FILE *in, FILE *out);
void printchain(int cmdc, struct Cmd *cmd);

struct Applet {
	Cmdfunction function;
	void (*help)();
	char *name;
};

static struct Applet applets[] = {
	APPLET(db),
	APPLET(delete),
	APPLET(depency),
	APPLET(download),
	APPLET(filter),
	APPLET(install),
	APPLET(ui),
	{ main_applet, help, NULL },
};

static struct Cmd syncchain[] = {
	{ db,		1,	{ "-i" } },
	{ filter,	1,	{ "-tr" } },
	{ download,	1,	{ "-n" } },
	{ install,	1,	{ "-o" } },
};
static struct Cmd searchchain[] = {
	{ db,		1,	{ "-p" } },
	{ filter,	2,	{ "-s", NULL } },
	{ ui,		0,	{ NULL } },
};
static struct Cmd installchain[] = {
	{ db,		1,	{ "-p" } },
	{ filter,	2,	{ "-e", NULL } },
	{ download,	0,	{ NULL } },
	{ install,	2,	{ "-fp" } },
};
static struct Cmd rmchain[] = {
	{ db,		1,	{ "-i" } },
	{ filter,	2,	{ "-e", NULL } },
	{ delete,	0,	{ NULL } },
};
static struct Cmd updatechain[] = {
	{ db,		1,	{ "-i" } },
	{ db,		1,	{ "-o" } },
	{ filter,	1,	{ "-n" } },
	{ filter,	1,	{ "-tr" } },
	{ install,	0,	{ NULL } },
};

int main_applet(int argc, char *argv[], FILE *in, FILE *out) {
	int action = 0, installed = 0, sync = 0;
	char *arg = NULL;

	ARG {
	case 'r':
	case 'i':
	case 's':
		if(!(arg = ARGVAL()))
			goto argerr;
	case 'u':
		if(action != 0) 
			goto argerr;
		action = ARGCHR();
		break;
	case 'y':
		sync = 1;
		break;
	case 'I':
		installed = 1;
		break;
	argerr:
	default:
		version();
		help();
		return EXIT_FAILURE;
	}
	if(argc < 1 || argc != ARGC())
		goto argerr;
	if(sync) {
		printchain(LENGTH(syncchain), syncchain);
		cmdchain(LENGTH(syncchain), syncchain);
	}
	switch(action) {
	case 'u':
		printchain(LENGTH(updatechain), updatechain);
		cmdchain(LENGTH(updatechain), updatechain);
		break;
	case 's':
		searchchain[0].argv[0] = installed ? "-i" : "-p";
		searchchain[1].argv[1] = arg;
		printchain(LENGTH(searchchain), searchchain);
		cmdchain(LENGTH(searchchain), searchchain);
		break;
	case 'i':
		installchain[0].argv[0] = installed ? "-i" : "-p";
		installchain[1].argv[1] = arg;
		printchain(LENGTH(installchain), installchain);
		cmdchain(LENGTH(installchain), installchain);
		break;
	case 'r':
		rmchain[1].argv[1] = arg;
		printchain(LENGTH(rmchain), rmchain);
		cmdchain(LENGTH(rmchain), rmchain);
		break;
	}
	return EXIT_SUCCESS;
}

void printchain(int cmdc, struct Cmd *cmd) {
	int i, j;

	for(i = 0; i < cmdc; i++) {
		for(j = 0; j < LENGTH(applets) - 1; j++) 
			if(applets[j].function == cmd[i].function)
				break;
		if(j == LENGTH(applets) - 1)
			fputs("<unknown>", stderr);
		else
			fprintf(stderr,APPLETPREFIX "%s", applets[j].name);
		for(j = 0; j < cmd[i].argc; j++) {
			fprintf(stderr," \"%s\"",cmd[i].argv[j]);
		}
		if(i + 1 < cmdc)
			fputs(" | ",stderr);
	}
	fputc('\n',stderr);
}

void help() {
	fputs("spiceman\n", stderr);
	fputs("	-H	help message for all applets\n", stderr);
	fputs("	-I	use installed packages as db source.\n", stderr);
	fputs("	-h	help message\n", stderr);
	fputs("	-i <p>	Install packages\n", stderr);
	fputs("	-r <p>	Remove packages\n", stderr);
	fputs("	-s <p>	search package\n", stderr);
	fputs("	-y	sync with repositories\n", stderr);
	fputs("	-u	Update system\n", stderr);
	fputs("	-v	Version\n", stderr);
}

int main(int argc, char *argv[]) {
	int applet, i;
	char *bn;
	unsigned int showhelp, retval;

	showhelp = argc < 1;
	bn = basename(argv[0]);
	for(applet = 0; applet < LENGTH(applets) - 1; applet++)
		if((strncmp(bn, APPLETPREFIX, LENGTH(APPLETPREFIX) - 1) == 0 &&
				strcmp(bn + LENGTH(APPLETPREFIX) - 1, applets[applet].name) == 0) ||
				(argc > 1 && strcmp(argv[1], applets[applet].name) == 0))
			break;
	/* shifting to second argument to make ARG* work */
	argc--;
	argv++;
	/* global options */
	ARG {
	case 'v':
		version();
		exit(EXIT_FAILURE);
	case 'h':
		showhelp = 1;
		break;
	case 'H':
		showhelp = 2;
		break;
	}
	if(showhelp) {
		version();
		if(showhelp == 2 && applet == LENGTH(applets) - 1) {
			applets[applet].help();
			for(i = 0; i < LENGTH(applets) - 1; i++)
				applets[i].help();
		}
		else
			applets[applet].help();
		return EXIT_FAILURE;
	}
	else if(argc && applets[applet].name &&
			!strcmp(applets[applet].name, argv[0]))
		return applets[applet].function(argc-1, argv+1, stdin, stdout);
	else
		return applets[applet].function(argc, argv, stdin, stdout);
	return retval;
}
