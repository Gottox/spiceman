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

/* static */
static void
putbslash(const char *str, const char *chrs, FILE *out) {
	const char *p;

	for(p = str; *p; p++) {
		if(strchr(chrs, *p)) {
			fwrite(str, sizeof(char), p - str, out);
			fputc('\\', out);
			str = p;
		}
	}
	fwrite(str, sizeof(char), p - str, out);
}

static void
str2hex(char *dst, const char *src, int l) {
	unsigned int i;

	bzero(dst, sizeof(char) * l);
	for(i = 0; i < l &&
		sscanf(src + i * 2, "%2x",
				(unsigned int *)&dst[i]) > 0; i++);
}

static void
puthex(const char *src, FILE* out, int l) {
	unsigned int i;

	for(i = 0; i < l; i++)
		fprintf(out, "%02x", (unsigned int )src[i] % 256);
}

void
cmdchain(int cmdc, struct Cmd *cmd, FILE *pin, FILE *pout) {
	FILE *in, *out, *fp[2];
	int i, fd[2], pid, retval;

	in = pin;
	for(i = 0; i < cmdc; i++) {
		fpipe(fp);
		if(i + 1 == cmdc)
			out = pout;
		else if(pipe(fd))
			eprint(1, "Cannot create pipe");
		else if(!(out = fdopen(fd[1], "w")))
			eprint(1, "Cannot open pipe for writing");
		pid = fork();
		if(pid < 0)
			eprint(1, "Cannot fork");
		else if(pid == 0) {
			retval = cmd[i].function(cmd[i].argc, cmd[i].argv, in, out);
			fclose(in);
			waitchain(out);
			exit(retval);
		}
		if(out != pout)
			fclose(out);
		if(in != pin)
			fclose(in);
		if(i + 1 != cmdc && !(in = fdopen(fd[0], "r")))
			eprint(1, "Cannot open pipe for reading: ");
	}
}

void waitchain(FILE *out) {
	int status;

	fclose(out);
	while(wait(&status) != -1);
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

void
fpipe(FILE **fp) {
	int fd[2];

	if(pipe(fd))
		eprint(1, "cannot create pipe.");
	if(!(fp[1] = fdopen(fd[1], "w")))
		eprint(1, "cannot open pipe for writing.");
	if(!(fp[0] = fdopen(fd[0], "r")))
		eprint(1, "cannot open pipe for reading.");
}

int
getpkg(struct Package *pkg, FILE *in) {
	int l, n;
	char *s, *p, *q;

	l = 0;
	do {
		if(l + 1 >= pkg->blen) {
			pkg->blen = l + 1 + BUFSIZ;
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
				str2hex(pkg->md5, s, LENGTH(pkg->md5));
				break;
			case SHA1:
				str2hex(pkg->sha1, s, LENGTH(pkg->sha1));
				break;
			case KEY:
				str2hex(pkg->key, s, LENGTH(pkg->key));
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

void
putpkg(const struct Package *pkg, FILE *out) {
	char sep[3] = { FIELDSEPERATOR, '\n', 0 };
	
	if(pkg->type != '\0')
		fputc(pkg->type, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->name, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->ver, sep, out);
	fputc(FIELDSEPERATOR, out);
	fprintf(out,"%u", pkg->rel);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->desc, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->url, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->usef, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->repo, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->infourl, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->dep, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->conflict, sep, out);
	fputc(FIELDSEPERATOR, out);
	putbslash(pkg->prov, sep, out);
	fputc(FIELDSEPERATOR, out);
	fprintf(out,"%u", pkg->size);
	fputc(FIELDSEPERATOR, out);
	puthex(pkg->md5, out, LENGTH(pkg->md5));
	fputc(FIELDSEPERATOR, out);
	puthex(pkg->sha1, out, LENGTH(pkg->sha1));
	fputc(FIELDSEPERATOR, out);
	puthex(pkg->key, out, LENGTH(pkg->key));
	fputc(FIELDSEPERATOR, out);
	fprintf(out,"%lu", pkg->reltime);
	fputc(FIELDSEPERATOR, out);
	fprintf(out,"%lu",pkg->instime);
	fputc('\n', out);
	fflush(out);
}

void
freepkg(struct Package *pkg) {
	if(pkg->buf)
		free(pkg->buf);
	pkg->buf = NULL;
	pkg->blen = 0;
}

int
pkgcmp(const struct Package *p1, const struct Package *p2) {
	int cmp;

	if((cmp = strcmp(p1->name, p2->name)))
		return cmp;
	if((cmp = strcmp(p1->ver, p2->ver)))
		return cmp;
	if((cmp = (int)p1->rel - p2->rel))
		return cmp;
	return 0;
}

int
vercmp(const char *v1, const char *v2) {

	return 0;
}
void
version() {
	fputs("spiceman-" VERSION " - distributed package management tools\n", stderr);
}
