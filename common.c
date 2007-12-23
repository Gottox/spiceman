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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"



/* common.c */
void *
amemcpy(void *d, void *s, size_t n) {
	d = erealloc(0,n);
	return memcpy(d, s, n);
}

int cmdchain(int cmdc, struct Cmd *cmd) {
	FILE *in, *out;
	int i, fd[2], pid, retval, status;

	in = stdin;
	for(i = 0; i < cmdc-1; i++) {
		if(pipe(fd) != 0)
			eprint("Cannot create pipe: ");
		if(!(out = fdopen(fd[1], "w")))
			eprint("Cannot open pipe for writing: ");
		pid = fork();
		if(pid < 0)
			eprint("Cannot fork: ");
		else if(pid == 0)
			exit(cmd[i].function(cmd[i].argc,cmd[i].argv,in,out));
		fclose(out);
		if(in != stdin)
			fclose(in);
		if(!(in = fdopen(fd[0], "r")))
			eprint("Cannot open pipe for reading: ");
	}
	out = stdout;
	retval = cmd[i].function(cmd[i].argc,cmd[i].argv,in,out);
	while(wait(&status) != -1);
	return retval;
}

void *
erealloc(void *p, size_t size) {
	if(!(p = realloc(p, size))) {
		perror("Cannot Malloc");
		exit(EXIT_FAILURE);
	}
	return p;
}

void
eprint(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	perror(NULL);
	va_end(ap);
	exit(EXIT_FAILURE);
}

int
getpkg(struct Package *pkg, FILE *in, char *sep) {
	char *b;
	int l, ent, i;

	if(strlen(sep) != 2)
		return 0;
	ent = l = 0;
	b = erealloc(NULL, sizeof(char) * BUFFERSIZE);
	while((b[l] = fgetc(in)) && b[l] != sep[1]) {
		if(l && l % BUFFERSIZE == 0)
			b = erealloc(b, sizeof(char) * BUFFERSIZE + l);
		if(b[l] == sep[0]) {
			b[l] = '\0';
			switch(ent) {
			case TYPE:
				pkg->type = b[0];
				break;
			case NAME:
				amemcpy(pkg->name,b, sizeof(char) * l);
				break;
			case VER:
				amemcpy(pkg->name,b, sizeof(char) * l);
				break;
			case REL:
				pkg->rel = atoi(b);
				break;
			case RELTIME:
				pkg->reltime = atoi(b);
				break;
			case DESC:
				amemcpy(pkg->desc,b, sizeof(char) * l);
				break;
			case URL:
				amemcpy(pkg->url,b, sizeof(char) * l);
				break;
			case USEF:
				amemcpy(pkg->usef,b, sizeof(char) * l);
				break;
			case REPO:
				amemcpy(pkg->repo,b, sizeof(char) * l);
				break;
			case DEP:
				amemcpy(pkg->dep,b, sizeof(char) * l);
				break;
			case CONFLICT:
				amemcpy(pkg->conflict,b, sizeof(char) * l);
				break;
			case PROV:
				amemcpy(pkg->prov,b, sizeof(char) * l);
				break;
			case SIZE:
				pkg->size = atoi(b);
				break;
			case MD5:
				for(i = 0; sscanf(b+i*2, "%2x", (unsigned int *)&pkg->key[i]) && i < LENGTH(pkg->md5);i++);
			case SHA1:
				for(i = 0; sscanf(b+i*2, "%2x", (unsigned int *)&pkg->key[i]) && i < LENGTH(pkg->sha1);i++);
			case KEY:
				for(i = 0; sscanf(b+i*2, "%2x", (unsigned int *)&pkg->key[i]) && i < LENGTH(pkg->key);i++);
			case INSTIME:
				pkg->reltime = atoi(b);
				break;
			default:
				break;
			}
			l = 0;
			ent++;
		}
		l++;
	}
	
	return 0;
}

void putpkg(struct Package *pkg, FILE *out, char *sep) {
	int i = 0;
	
	for(i = 0; i < NENTRIES; i++) {
		
	}
}

void version() {
	fputs("spiceman-" VERSION " - suckless package management tools\n",stderr);
}
