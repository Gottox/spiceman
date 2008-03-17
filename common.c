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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"

int
cmdchain(int cmdc, struct Cmd *cmd) {
	FILE *in, *out;
	int i, fd[2], pid, retval, status;

	in = stdin;
	for(i = 0; i < cmdc - 1; i++) {
		if(pipe(fd))
			eprint(1, "Cannot create pipe: ");
		if(!(out = fdopen(fd[1], "w")))
			eprint(1, "Cannot open pipe for writing: ");
		pid = fork();
		if(pid < 0)
			eprint(1, "Cannot fork: ");
		else if(pid == 0) {
			retval = cmd[i].function(cmd[i].argc,cmd[i].argv,in,out);
			fclose(out);
			fclose(in);
			exit(retval);
		}
		fclose(out);
		fclose(in);
		if(!(in = fdopen(fd[0], "r")))
			eprint(1, "Cannot open pipe for reading: ");
	}
	out = stdout;
	retval = cmd[i].function(cmd[i].argc,cmd[i].argv,in,out);
	fclose(out);
	fclose(in);
	while(wait(&status) != -1);
	return retval;
}

void *
erealloc(void *p, size_t size) {
	if(!(p = realloc(p, size)))
		eprint(0, "Cannot Malloc");
	return p;
}

void
eprint(int pe, const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	if(pe)
		perror("");
	else
		fputc('\n',stderr);
	va_end(ap);
	exit(EXIT_FAILURE);
}

int
getpkg(struct Package *pkg, FILE *in) {
	int i, n, start;
	char c;
	int fields[NENTRIES + 1];

	bzero(fields, sizeof(fields));
	for(n = i = c = 0; c != seperator[1] && !feof(in) && (c = fgetc(in)); i++) {
		if(i % BUFFERSIZE == 0)
			pkg->buf = erealloc(pkg->buf, sizeof(char) * BUFFERSIZE + i);
		if((c == seperator[0] || c == seperator[1]) && n < LENGTH(fields)) {
			pkg->buf[i] = 0;
			fields[++n] = i + 1;
		}
		else
			pkg->buf[i] = c == '\\' ? fgetc(in) : c;
	}
	pkg->buf[i] = 0;
	if(i == 0) {
		return 0;
	}
	else if(n < NENTRIES) {
		freepkg(pkg);
		return -1;
	}
	for(i = 0; i < NENTRIES; i++) {
		start = fields[i];
		switch(i) {
		case TYPE:
			pkg->type = pkg->buf[start];
			break;
		case NAME:
			pkg->name = &pkg->buf[start];
			break;
		case VER:
			pkg->ver = &pkg->buf[start];
			break;
		case REL:
			pkg->rel = atoi(&pkg->buf[start]);
			break;
		case DESC:
			pkg->desc = &pkg->buf[start];
			break;
		case URL:
			pkg->url = &pkg->buf[start];
			break;
		case USEF:
			pkg->usef = &pkg->buf[start];
			break;
		case REPO:
			pkg->repo = &pkg->buf[start];
			break;
		case DEP:
			pkg->dep = &pkg->buf[start];
			break;
		case CONFLICT:
			pkg->conflict = &pkg->buf[start];
			break;
		case PROV:
			pkg->prov = &pkg->buf[start];
			break;
		case PATH:
			pkg->path = &pkg->buf[start];
			break;
		case SIZE:
			pkg->size = atoi(&pkg->buf[start]);
			break;
/*		case MD5:
			bzero(pkg->md5, sizeof(pkg->md5));
			for(i = 0; sscanf(b + start + i * 2, "%2x", (unsigned int *)&pkg->md5[i])
					&& i * 2 + 1 < l && i < LENGTH(pkg->md5); i++);
			break;
		case SHA1:
			bzero(pkg->sha1, sizeof(pkg->sha1));
			for(i = 0; sscanf(b + start + i * 2, "%2x", (unsigned int *)&pkg->sha1[i])
					&& i * 2 + 1 < l && i < LENGTH(pkg->sha1); i++);
			break;
		case KEY:
			bzero(pkg->key, sizeof(pkg->key));
			for(i = 0; sscanf(b + start + i * 2, "%2x", (unsigned int *)&pkg->key[i])
					&& i * 2 + 1 < l && i < LENGTH(pkg->key); i++);
		break;*/
		case RELTIME:
			pkg->reltime = atol(&pkg->buf[start]);
			break;
		case INSTIME:
			pkg->instime = atol(&pkg->buf[start]);
			break;
		default:
			break;
		}
	}

	return 1;
}

void putpkg(const struct Package *pkg, FILE *out) {
	unsigned int i, j;
	char *p;
	
	for(i = 0; i < NENTRIES; i++) {
		p = NULL;
		switch(i) {
		case TYPE:
			if(pkg->type != '\0')
				fputc(pkg->type, out);
			break;
		case NAME:
			p = pkg->name;
			break;
		case VER:
			p = pkg->ver;
			break;
		case REL:
			fprintf(out,"%u", pkg->rel);
			break;
		case DESC:
			p = pkg->desc;
			break;
		case URL:
			p = pkg->url;
			break;
		case USEF:
			p = pkg->usef;
			break;
		case REPO:
			p = pkg->repo;
			break;
		case DEP:
			p = pkg->dep;
			break;
		case CONFLICT:
			p = pkg->conflict;
			break;
		case PROV:
			p = pkg->prov;
			break;
		case SIZE:
			fprintf(out,"%u", pkg->size);
			break;
		case MD5:
			for(j = 0; j < LENGTH(pkg->md5); j++)
				fprintf(out, "%02x", (unsigned int )pkg->md5[j] % 256);
			break;
		case SHA1:
			for(j = 0; j < LENGTH(pkg->sha1); j++)
				fprintf(out, "%02x", (unsigned int )pkg->sha1[j] % 256);
			break;
		case KEY:
			for(j = 0; j < LENGTH(pkg->key); j++)
				fprintf(out, "%02x", (unsigned int )pkg->key[j] % 256);
			break;
		case RELTIME:
			fprintf(out,"%lu", pkg->reltime);
			break;
		case INSTIME:
			fprintf(out,"%lu",pkg->instime);
			break;
		}
		if(p)
			for(; *p != '\0'; p++) {
				if(*p == seperator[0] || *p == seperator[1] || *p == '\\')
					fputc('\\',out);
				fputc(*p,out);
			}
		fputc(seperator[0], out);
	}
	fputc(seperator[1], out);
	fflush(out);
}

void freepkg(struct Package *pkg) {
	if(pkg->buf)
		free(pkg->buf);
	pkg->buf = NULL;
}

void version() {
	fputs("spiceman-" VERSION " - distributed package management tools\n",stderr);
}
