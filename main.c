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
#include "extract.h"
#include "filter.h"

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
	APPLET(extract),
	APPLET(filter),
	{ main_applet, help, NULL },
};

int main_applet(int argc, char *argv[], FILE *in, FILE *out) {
	int i;
	int action;
	int showcommand;

	showcommand = 0;
	int installed = 0;
	struct Cmd cmds[2];
	char *v[2][10], *arg;
	action = 0;
	arg = NULL;
	for(i = 0; i < argc; i++) {
		if(argv[i][0] != '-') {
			help();
			return EXIT_FAILURE;
		}
		switch(argv[i][1]) {
		case 's':
		case 'i':
		case 'r':
			action = argv[i][1];
			if(++i == argc) {
				help();
				return EXIT_FAILURE;
			}
			arg = argv[i];
			break;
		case 'I':
			installed = 1;
			break;
		case 'c':
			showcommand = 1;
			break;
		default:
			version();
			help();
			return EXIT_FAILURE;
		}
	}
	switch(action) {
		case 's':
			cmds[0].function = db;
			cmds[0].argc = installed;
			v[0][0] = "-i";
			cmds[0].argv = v[0];
			cmds[1].function = filter;
			cmds[1].argc = 2;
			v[1][0] = "-s";
			v[1][1] = arg;
			cmds[1].argv = v[1];
			if(showcommand)
				printchain(2, cmds);
			else
				return cmdchain(2, cmds);
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
	fputs("\n",stderr);
}

void help() {
	fputs("spiceman\n", stderr);
	fputs("	-H	help message for all applets\n", stderr);
	fputs("	-I	use installed packages as db source.\n", stderr);
	fputs("	-h	help message\n", stderr);
	fputs("	-c	show command and exit\n", stderr);
	fputs("	-i <p>	Install packages\n", stderr);
	fputs("	-r <p>	Remove packages\n", stderr);
	fputs("	-s <p>	search package\n", stderr);
	fputs("	-u	Update system\n", stderr);
	fputs("	-v	Version\n", stderr);
}

int main(int argc, char *argv[]) {
	int applet, i;
	char *bn;
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
	for(i = 1; i < argc && !showhelp; i++)
		if(argv[i][0] == '-')
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
				if(++i == argc || i > applet)
					showhelp = 1;
				else if(!(in = fopen(argv[i],"r")))
					eprint(1, "Cannot open `%s`: ", argv[i]);
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
		retval = EXIT_FAILURE;
	}
	else if(applets[applet].name == NULL || strcmp(argv[1], applets[applet].name) != 0)
		retval = applets[applet].function(argc-1, argv+1, in, stdout);
	else
		retval = applets[applet].function(argc-2, argv+2, in, stdout);
	fclose(in);
	return retval;
}
