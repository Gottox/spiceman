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
#include "delete.h"
#include "depency.h"
#include "download.h"
#include "filter.h"
#include "install.h"

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
	{ main_applet, help, NULL },
};

int main_applet(int argc, char *argv[], FILE *in, FILE *out) {
	int i, action = 0, installed = 0, sync = 0;
	char *arg = NULL, *p;
	struct Cmd syncchain[] = {
		{ db,		1,	{ "-i" } },
		{ filter,	1,	{ "-r" } },
		{ depency,	1,	{ "-r" } },
		{ download,	0,	{ NULL } },
		{ install,	1,	{ "-o" } },
	};
	struct Cmd searchchain[] = {
		{ db,		1,	{ "-p" } },
		{ filter,	2,	{ "-s", NULL } },
	};
	struct Cmd installchain[] = {
		{ db,		1,	{ "-p" } },
		{ filter,	2,	{ "-e", NULL } },
		{ depency,	1,	{ "-r" } },
		{ download,	0,	{ NULL } },
		{ install,	2,	{ "-f", "-p" } },
	};
	struct Cmd rmchain[] = {
		{ db,		1,	{ "-i" } },
		{ filter,	2,	{ "-e", NULL } },
		{ delete,	0,	{ NULL } },
	};
	struct Cmd updatechain[] = {
		{ db,		1,	{ "-i" } },
		{ filter,	1,	{ "-o" } },
		{ filter,	1,	{ "-n" } },
		{ depency,	1,	{ "-r" } },
		{ install,	0,	{ NULL } },
	};

	for(i = 0; i < argc; i++) {
		if(argv[i][0] != '-' || argv[i][1] == 0) {
			help();
			return EXIT_FAILURE;
		}
		for(p = argv[i] + 1; *p; p++) {
			switch(*p) {
			case 'u':
				if(action != 0) {
					help();
					return EXIT_FAILURE;
				}
				action = 'u';
				break;
			case 'y':
				sync = 1;
				break;
			case 'r':
			case 'i':
			case 's':
				if(action != 0 || i + 1 == argc) {
					help();
					return EXIT_FAILURE;
				}
				action = *p;
				arg = argv[++i];
				break;
			case 'I':
				installed = 1;
				break;
			default:
				version();
				help();
				return EXIT_FAILURE;
			}
		}
	}

	if(sync) {
		printchain(LENGTH(syncchain), syncchain);
		cmdchain(LENGTH(syncchain), syncchain);
		if(!action)
			return EXIT_SUCCESS;
	}
	switch(action) {
		case 's':
			searchchain[1].argv[1] = arg;
			cmdchain(LENGTH(syncchain), syncchain);
			break;
		case 'i':
			installchain[1].argv[1] = arg;
			cmdchain(LENGTH(installchain), installchain);
			break;
		case 'r':
			rmchain[1].argv[1] = arg;
			cmdchain(LENGTH(rmchain), rmchain);
			break;
		case 'u':
			cmdchain(LENGTH(updatechain), updatechain);
			break;
		default:
			version();
			help();
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void printchain(int cmdc, struct Cmd *cmd) {
	int i, j;

	for(i = 0; i < cmdc; i++) {
		for(j = 0; j < LENGTH(applets)-1; j++) 
			if(applets[j].function == cmd[i].function)
				break;
		if(j == LENGTH(applets)-1)
			fputs("<unknown>", stderr);
		else
			fprintf(stderr,APPLETPREFIX "%s", applets[j].name);
		for(j = 0; j < cmd[i].argc;j++) {
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
	char *bn, *p;
	unsigned int showhelp, retval;
	FILE *in;

	showhelp = argc <= 1;
	in = stdin;
	bn = basename(argv[0]);
	for(applet = 0; applet < LENGTH(applets) - 1; applet++)
		if((strncmp(bn, APPLETPREFIX, LENGTH(APPLETPREFIX) - 1) == 0 &&
				strcmp(bn + LENGTH(APPLETPREFIX) - 1, applets[applet].name) == 0) ||
				(argc > 1 && strcmp(argv[1], applets[applet].name) == 0))
			break;
	/* global options */
	for(i = 1; i < argc && !showhelp; i++) {
		if(argv[i][0] != '-')
			continue;
		for(p = argv[i] + 1; *p; p++ )
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
			}
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
		retval = EXIT_FAILURE;
	}
	else if(applets[applet].name == NULL || strcmp(argv[1], applets[applet].name) != 0)
		retval = applets[applet].function(argc-1, argv+1, in, stdout);
	else
		retval = applets[applet].function(argc-2, argv+2, in, stdout);
	fclose(in);
	return retval;
}
