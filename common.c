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
#include <strings.h>
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
	int i, l, n;
	char *s, *p, *q;

	l = 0;
	do {
		if(l + 1 >= pkg->blen) {
			pkg->blen = l + 1 + BUFSIZE;
			pkg->buf = erealloc(pkg->buf, sizeof(char) * pkg->blen);
		}
		if(fgets(pkg->buf + l, pkg->blen - l, in))
			l += strlen(&pkg->buf[l]);
	} while(l > 0 && !feof(in) && pkg->buf[l - 1] != '\n');
	if(l == 0)
		return 0;
	for(s = p = pkg->buf, n = 0; *p; p++) {
		if(*p == '\\') {
			for(q = p; *q; q++)
				q[0] = q[1];
		}
		else if(*p == FIELDSEPERATOR) {
			*p = 0;
			switch(n) {
			case TYPE:
				pkg->type = *s;
				break;
			case NAME:
				pkg->name = s;
				break;
			case VER:
				pkg->ver = s;
				break;
			case REL:
				pkg->rel = atoi(s);
				break;
			case DESC:
				pkg->desc = s;
				break;
			case URL:
				pkg->url = s;
				break;
			case USEF:
				pkg->usef = s;
				break;
			case REPO:
				pkg->repo = s;
				break;
			case INFOURL:
				pkg->infourl = s;
				break;
			case DEP:
				pkg->dep = s;
				break;
			case CONFLICT:
				pkg->conflict = s;
				break;
			case PROV:
				pkg->prov = s;
				break;
			case SIZE:
				pkg->size = atoi(s);
				break;
			case MD5:
				bzero(pkg->md5, sizeof(pkg->md5));
				for(i = 0; i < LENGTH(pkg->md5) &&
						sscanf(s + i * 2, "%2x",
							(unsigned int *)&pkg->md5[i]) > 0; i++);
				break;
			case SHA1:
				bzero(pkg->sha1, sizeof(pkg->sha1));
				for(i = 0; i < LENGTH(pkg->sha1) &&
						sscanf(s + i * 2, "%2x",
							(unsigned int *)&pkg->sha1[i]) > 0; i++);
				break;
			case KEY:
				bzero(pkg->key, sizeof(pkg->key));
				for(i = 0; i < LENGTH(pkg->sha1) &&
						sscanf(s + i * 2, "%2x",
							(unsigned int *)&pkg->key[i]) > 0; i++);
				break;
			case RELTIME:
				pkg->reltime = atol(s);
				break;
			case INSTIME:
				pkg->instime = atol(s);
				break;
			default:
				break;
			}
			s = p + 1;
			n++;
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
		case INFOURL:
			p = pkg->infourl;
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
				if(*p == '\n' || *p == FIELDSEPERATOR || *p == '\\')
					fputc('\\',out);
				fputc(*p,out);
			}
		fputc(FIELDSEPERATOR, out);
	}
	fputc('\n', out);
	fflush(out);
}

void freepkg(struct Package *pkg) {
	if(pkg->buf)
		free(pkg->buf);
	pkg->buf = NULL;
	pkg->blen = 0;
}

int pkgcmp(const struct Package *p1, const struct Package *p2, char how) {
	int cmp;

	if((cmp = strcmp(p1->name, p2->name)))
		return cmp;
	if((cmp = strcmp(p1->ver, p2->ver)))
		return cmp;
	if((cmp = (int)p1->rel - p2->rel))
		return cmp;
	return 0;
}

void version() {
	fputs("spiceman-" VERSION " - distributed package management tools\n", stderr);
}
