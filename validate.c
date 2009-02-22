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
#include <ctype.h>

#include "common.h"
#include "validate.h"

static int
nonempty(const char *name, const char *str, const char *label) {
	if(str[0] == '\0') {
		fprintf(stderr, "Warning: %s: %s may not empty.\n",
				name, label);
		return 0;
	}
	return 1;
}

static int
maxlength(const char *name, const char *str, int max, const char *label) {
	if(strlen(str) > max) {
		fprintf(stderr, "Warning: %s: %s is too long.\n",
				name, label);
		return 0;
	}
	return 1;
}

static int
alnumand(const char *name, const char *str, const char *allowed,
		const char *label) {
	const char *p;

	for(p = str; *p && (isalnum(*p) || strchr(allowed, *p)); p++);
	if(*p) {
		fprintf(stderr, "Warning: %s: %s may not contain "
				"ascii code '%u'.\n",
				name, label, *p);
		return 0;
	}
	return 1;

}

static int
lower(const char *name, const char *str, const char *label) {
	const char *p;

	for(p = str; *p && (islower(*p) || !isalpha(*p)); p++);
	if(*p) {
		fprintf(stderr, "Warning: %s: %s must be lower case.\n",
				name, label);
		return 0;
	}
	return 1;
}

static int
digitand(const char *name, const char *str, const char *allowed,
		const char *label) {
	const char *p;

	for(p = str; *p && (isdigit(*p) || strchr(allowed, *p)); p++);
	if(*p) {
		fprintf(stderr, "Warning: %s: %s may not contain "
				"ascii code '%u'\n",
				name, label, *p);
		return 0;
	}
	return 1;
}

static int
notcontain(const char *name, const char *str, const char *disallowed,
		const char *label) {
	if(strstr(str, disallowed)) {
		fprintf(stderr, "Warning: %s: %s contains not allowed "
				"substring\n", name, label);
		return 0;
	}
	return 1;
}

static int
validatepkg(struct Package *pkg) {
	int result = 1;
	char *label;
	char *str;

	label = "package type";
	str = pkg->fields[TYPE];
	result &= nonempty(pkg->name, str, label);
	result &= maxlength(pkg->name, str, 1, label);

	label = "package name";
	str = pkg->fields[NAME];
	result &= nonempty(pkg->name, str, label);
	result &= maxlength(pkg->name, str, 255, label);
	result &= alnumand(pkg->name, str, "_-", label);
	result &= lower(pkg->name, str, label);

	label = "package version";
	str = pkg->fields[VER];
	result &= nonempty(pkg->name, str, label);
	result &= maxlength(pkg->name, str, 255, label);
	result &= alnumand(pkg->name, str, "_.", label);

	label = "package release";
	str = pkg->fields[REL];
	result &= nonempty(pkg->name, str, label);
	result &= digitand(pkg->name, str, "", label);

	label = "package description";
	str = pkg->fields[DESC];
	result &= notcontain(pkg->name, str, "\n\n", label);

	label = "package url";
	str = pkg->fields[URL];
	result &= nonempty(pkg->name, str, label);
	result &= alnumand(pkg->name, str, "/~._=?&-:", label);

	label = "package useflags";
	str = pkg->fields[USEF];
	result &= alnumand(pkg->name, str, "_- \t", label);
	result &= lower(pkg->name, str, label);

	label = "package repository";
	str = pkg->fields[REPO];
	result &= nonempty(pkg->name, str, label);
	result &= maxlength(pkg->name, str, 255, label);
	result &= alnumand(pkg->name, str, "_-", label);
	result &= lower(pkg->name, str, label);

	label = "package info url";
	str = pkg->fields[URL];
	result &= nonempty(pkg->name, str, label);
	result &= alnumand(pkg->name, str, "/~._=?&-:", label);

	return result;
}

void validate_help() {
	APPLETUSAGE("validate");
	fputs("	-r	remove deprecated packages from output\n", stderr);
}

int validate(int argc, char *argv[]) {
	struct Package pkg;
	int error = 0;
	int printdeprec = 0;
	
	if(argc == 1 && strcmp("-w", argv[0]))
		printdeprec = 1;
	else if(argc != 0) {
		validate_help();
		return EXIT_FAILURE;
	}

	bzero(&pkg, sizeof(pkg));
	while(getfreepkg(&pkg) > 0) {
		if(!validatepkg(&pkg)) {
			if(printdeprec)
				putpkg(&pkg);
			error = 1;
		}
		else
			putpkg(&pkg);
	}

	return error;
}
