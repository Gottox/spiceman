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
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "common.h"

/* static */
static void
putbslash(const char *str, const char *chrs, FILE *out) {
	const char *p;

	for(p = str; *p; p++) {
		if(strchr(chrs, *p)) {
			fwrite(str, sizeof(char), p - str, out);
			fputc('\\', out);
			str = p + 1;
			switch(*p) {
				case '\n': fputc('n', out); break;
				case '\t': fputc('t', out); break;
				default: str--;
			}
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
cmdchain(int cmdc, struct Cmd *cmd) {
	int fd[2], in, out , pid, i, status;

	in = STDIN_FILENO;
	for(i = 0; i < cmdc; i++) {
		if(i + 1 == cmdc)
			out = STDOUT_FILENO;
		else {
			pipe(fd);
			out = i + 1 == cmdc ? STDOUT_FILENO : fd[1];
		}
		pid = fork();
		if(pid < 0)
			die(1, "Cannot fork");
		else if(pid == 0) {
			if(out != STDOUT_FILENO)
				dup2(out, STDOUT_FILENO);
			if(in != STDIN_FILENO)
				dup2(in, STDIN_FILENO);
			exit(cmd[i].function(cmd[i].argc, cmd[i].argv));
		}
		close(in);
		close(out);
		in = fd[0];
	}
	while(wait(&status) != -1);
}

void *
erealloc(void *p, size_t size) {
	if(!(p = realloc(p, size)))
		die(0, "Cannot Malloc");
	return p;
}

void
die(int pe, const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	if(pe)
		perror("");
	else
		fputc('\n',stderr);
	va_end(ap);
	exit(EXIT_SUCCESS);
}

void
freepkg(struct Package *pkg) {
	if(pkg->buf)
		free(pkg->buf);
	pkg->buf = NULL;
	pkg->blen = 0;
}

int
getfreepkg(struct Package *pkg) {
	int rc;
	if(!(rc = getpkg(pkg)) > 0)
		freepkg(pkg);
	return rc;
}

int
getpkg(struct Package *pkg) {
	int l, n;
	char *s, *p;

	l = 0;
	do {
		if(l + 1 >= pkg->blen) {
			pkg->blen = l + 1 + BUFSIZ;
			pkg->buf = erealloc(pkg->buf, sizeof(char) * pkg->blen);
		}
		if(fgets(&pkg->buf[l], pkg->blen - l, stdin))
			l += strlen(&pkg->buf[l]);
	} while(l > 0 && !feof(stdin) && pkg->buf[l - 1] != '\n');
	if(l == 0)
		return 0;
	for(s = p = pkg->buf, n = 0; *p; p++) {
		if(*p == '\\') {
			memmove(p, p + 1, l - (p - pkg->buf));
			switch(*p) {
			case 'n':
				*p = '\n';
				break;
			case 't':
				*p = '\t';
				break;
			}
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

int
mkdirhier(char *path) {
	int i, rc;
	char tmp;
	struct stat st = { .st_mode = 0700 };
	
	for(i = 0; path[i] != 0; i++)
		if((tmp = path[i]) == '/' && stat(path, &st) != 0) {
			path[i] = 0;
			mkdir(path, st.st_mode);
			path[i] = tmp;
		}
	rc = mkdir(path, st.st_mode);
	return errno == EEXIST ? 0 : rc;
}

void
putpkg(const struct Package *pkg) {
	char sep[] = { FIELDSEPERATOR, '\n', '\t', 0 };
	
	if(pkg->type != '\0')
		fputc(pkg->type, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->name, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->ver, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	fprintf(stdout,"%u", pkg->rel);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->desc, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->url, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->usef, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->repo, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->infourl, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->dep, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->conflict, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	putbslash(pkg->prov, sep, stdout);
	fputc(FIELDSEPERATOR, stdout);
	fprintf(stdout,"%u", pkg->size);
	fputc(FIELDSEPERATOR, stdout);
	puthex(pkg->md5, stdout, LENGTH(pkg->md5));
	fputc(FIELDSEPERATOR, stdout);
	puthex(pkg->sha1, stdout, LENGTH(pkg->sha1));
	fputc(FIELDSEPERATOR, stdout);
	puthex(pkg->key, stdout, LENGTH(pkg->key));
	fputc(FIELDSEPERATOR, stdout);
	fprintf(stdout,"%lu", pkg->reltime);
	fputc(FIELDSEPERATOR, stdout);
	fprintf(stdout,"%lu",pkg->instime);
	fputc('\n', stdout);
	fflush(stdout);
}

int
pkgcmp(const char *name1, const char *ver1, const int rel1,
		const char *name2, const char *ver2, const int rel2) {
	int cmp = 0;

	if(name1 != name2 && (cmp = strcmp(name1, name2)))
		return cmp;
	if(ver1 != ver2 && (cmp = vercmp(ver1, ver2)))
		return cmp;
	if((cmp = (int)rel1 - rel2))
		return cmp;
	return 0;
}

int
vercmp(const char *v1, const char *v2) {
	const char *p;
	int isdig1, isdig2, dig1, dig2, retval = 0;

	for(; *v1 && *v2 && retval == 0; v1++, v2++) {
		if(*v1 != *v2) {
			isdig1 = isdigit(*v1);
			isdig2 = isdigit(*v2);
			if(isdig1 && isdig2) {
				dig1 = atoi(v1);
				dig2 = atoi(v2);
				if (dig1 == dig2) {
					for(; v1[1] && isdigit(v1[1]); v1++);
					for(; v2[1] && isdigit(v2[1]); v2++);
				}
				else if(dig1 > dig2)
					retval = 1;
				else
					retval = -1;
			}
			else if(isdig1)
				retval = 1;
			else if(isdig2)
				retval = -1;
			else if((*v1 == '-' || *v1 == '.') && (*v2 == '-' || *v2 == '.'))
				return *v1 == '.' ? 1 : -1;
			else
				retval = strcmp(v1, v2);
		}
	}
	for(p = *v1 ? v1 : v2; *p && retval == 0; p++) {
		if(isalpha(*p))
			retval = *v1 ? -1 : 1;
		else if(!strchr("-.0", *p))
			retval = *v1 ? 1 : -1;
	}
	return retval;
}

void
version() {
	fputs("spiceman-" VERSION " - distributed package management tools\n",
			stderr);
}
