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
#include <stdarg.h>
#include <stdlib.h>

#define APPLETPREFIX		"sp-"
#define FIELDSEPERATOR		':'

/* LENGTH macro copied from dwm. */
#define MIN(a, b)		(a < b ? a : b)
#define LENGTH(x)		sizeof(x)/sizeof(x[0])
#define APPLETUSAGE(a)		fputs("sp-" a " || spiceman " a \
		"\n\t-h\thelp message\n", stderr);

/* Plan9-style Argument parsing */
/* Vars: _c -> count; _b -> break; _a -> argument */
#define ARG int _c, _b; char *_a; \
	for(_c = 0; _c < argc && argv[_c][0] == '-' && argv[_c][1] && \
			(strcmp(argv[_c], "--") != 0); _c++) \
		for(_a = &argv[_c][1], _b = 0; !_b && *_a; _a++ ) \
			switch(*_a)
#define ARGVAL()	(!_b && _a[1] && (_b = 1) ? &_a[1] : _c + 1 == argc ? \
		0 : argv[++_c])
#define ARGCHR()	(*_a)
#define ARGC()		_c

typedef int (*Cmdfunction)(int, char *[]);

enum PkgEnt	{ TYPE, NAME, VER, REL, DESC, URL, USEF, REPO, INFOURL, DEP,
	CONFLICT, PROV, SIZE, MD5, SHA, KEY, RELTIME, INSTIME, NENTRIES };

struct Cmd {
	Cmdfunction function;
	int argc;
	char *argv[16];
};

struct Package {
	char *buf;
	unsigned int blen;
	char *fields[NENTRIES];

	/* pkg information */
	char type;
	char *name;			/* name of pkg*/
	char *ver;			/* version of pkg (empty if type == REPOSITORY) */
	unsigned int rel;		/* release of pkg */
	char *desc;			/* description */
	char *url;			/* url of pkg */
	char *usef;			/* useflags */
	char *repo;			/* repository */
	char *infourl;

	/* to other packages */
	char *dep;			/* depencies */
	char *conflict;			/* pkg conflicts with */
	char *prov;			/* pkg provides */

	/* pkg integrity */
	unsigned int size;		/* size of package */
	char md5[16];
	char sha[20];
	char key[4];

	unsigned long reltime;		/* timestamp of release */
	unsigned long instime;
};


/* common.c */
int asprintf(char **str, char *format, ...);
char *astrcat(char **dest, const char *src);
char *astrcpy(char **dest, const char *src);
void cmdchain(int cmdc, struct Cmd *cmd);	/* executes a chain of commands */
void *erealloc(void *p, size_t size);		/* realloc + error testing */
void die(int pe, const char *format, ...);	/* prints message and exits */
void freepkg(struct Package *pkg);
int getfreepkg(struct Package *pkg);		/* like getpkg but frees the pkg
						   if getting fails */
int getpkg(struct Package *pkg);		/* reads a package from in */
int mkdirhier(const char *p);
void putpkg(const struct Package *pkg);		/* puts a package to out */
int pkgcmp(const char *name1, const char *ver1, const int rel1,
		const char *name2, const char *ver2, const int rel2);
					/* compares two packages */
int vercmp(const char *v1, const char *v2);
