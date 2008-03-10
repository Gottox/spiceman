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
#include <stdio.h>

#define APPLETPREFIX "sp-"

#define LENGTH(x)		sizeof(x)/sizeof(x[0])
#define APPLETUSAGE(a)		fputs("sp-" a "\nspiceman " a "\n", stderr);
#define INFILEHELP		fputs("	-R <f>	read packages from file (default: stdin)\n", stderr);
#define BUFFERSIZE		1024
#define DEFAULTSEP		":\n"


typedef int (*Cmdfunction)(int, char *[], FILE *in, FILE *out);

enum PkgEnt	{ TYPE, NAME, VER, REL, RELTIME, DESC, URL, USEF, REPO, DEP,
	CONFLICT, PROV, SIZE, MD5, SHA1, KEY, INSTIME, NENTRIES };

struct Cmd {
	Cmdfunction function;
	int argc;
	char **argv;
};

struct Package {
	/* pkg information */
	char type;
	char *name;			/* name of pkg*/
	char *ver;			/* version of pkg (empty if type == REPOSITORY) */
	unsigned int rel;		/* release of pkg */
	unsigned int reltime;		/* timestamp of release */
	char *desc;			/* description */
	char *url;			/* url of pkg */
	char *usef;			/* useflags */
	char *repo;			/* repository */

	/* to other packages */
	char *dep;			/* depencies */
	char *conflict;			/* pkg conflicts with */
	char *prov;			/* pkg provides */

	/* pkg integrity */
	unsigned int size;		/* time of release */
	char md5[16];
	char sha1[20];
	char key[100];			/* TODO: Don't know how many bytes to read. */

	/* information for installed packages */
	unsigned int instime;
};

/* common.c */
void *amemcpy(void *d, void *s, size_t n);	/* cpy memory to an allocated buffer */
int cmdchain(int cmdc, struct Cmd *cmd);	/* executes a chain of commands */
void *erealloc(void *p, size_t size);		/* remalloc + error testing */
void eprint(const char *format, ...);		/* prints message and exits */
void freepkg(struct Package *pkg);
int getpkg(struct Package *pkg, FILE *in, char *sep);
						/* reads a package from in */
void putpkg(struct Package *pkg, FILE *out, char *sep);
						/* puts a package to out */
void version();
