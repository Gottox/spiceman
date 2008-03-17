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

char *
astrcpy(void *d, void *s, int l) {
	if(!d || strlen(d) <= l)
		d = erealloc(d, (l + 1) * sizeof(char));
	return strcpy(d, s);
}

int
cmdchain(int cmdc, struct Cmd *cmd) {
	FILE *in, *out;
	int i, fd[2], pid, retval, status;

	in = stdin;
	for(i = 0; i < cmdc-1; i++) {
		if(pipe(fd) != 0)
			eprint(1, "Cannot create pipe: ");
		if(!(out = fdopen(fd[1], "w")))
			eprint(1, "Cannot open pipe for writing: ");
		pid = fork();
		if(pid < 0)
			eprint(1, "Cannot fork: ");
		else if(pid == 0)
			exit(cmd[i].function(cmd[i].argc,cmd[i].argv,in,out));
		fclose(out);
		if(in != stdin)
			fclose(in);
		if(!(in = fdopen(fd[0], "r")))
			eprint(1, "Cannot open pipe for reading: ");
	}
	out = stdout;
	retval = cmd[i].function(cmd[i].argc,cmd[i].argv,in,out);
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
	char c;
	char *b;
	int l, ent, i;
	 
	b = NULL;
	for(c = ent = l = 0; c != seperator[1] && !feof(in) && (c = fgetc(in));) {
		if(l % BUFFERSIZE == 0)
			b = erealloc(b, sizeof(char) * BUFFERSIZE + l);
		b[l] = c;
		if(b[l] == '\\') {
			b[l] = fgetc(in);
			l++;
		}
		else if(b[l] == seperator[0] || b[l] == seperator[1]) {
			b[l] = '\0';
			switch(ent) {
			case TYPE:
				pkg->type = b[0];
				break;
			case NAME:
				pkg->name = astrcpy(pkg->name, b, l);
				break;
			case VER:
				pkg->ver = astrcpy(pkg->ver, b, l);
				break;
			case REL:
				pkg->rel = atoi(b);
				break;
			case DESC:
				pkg->desc = astrcpy(pkg->desc, b, l);
				break;
			case URL:
				pkg->url = astrcpy(pkg->url, b, l);
				break;
			case USEF:
				pkg->usef = astrcpy(pkg->usef, b, l);
				break;
			case REPO:
				pkg->repo = astrcpy(pkg->repo, b, l);
				break;
			case DEP:
				pkg->dep = astrcpy(pkg->dep, b, l);
				break;
			case CONFLICT:
				pkg->conflict = astrcpy(pkg->conflict, b, l);
				break;
			case PROV:
				pkg->prov = astrcpy(pkg->prov, b, l);
				break;
			case PATH:
				pkg->path = astrcpy(pkg->path,b, l);
				break;
			case SIZE:
				pkg->size = atoi(b);
				break;
			case MD5:
				bzero(pkg->md5, sizeof(pkg->md5));
				for(i = 0; sscanf(b + i * 2, "%2x", (unsigned int *)&pkg->md5[i])
						&& i * 2 + 1 < l && i < LENGTH(pkg->md5); i++);
				break;
			case SHA1:
				bzero(pkg->sha1, sizeof(pkg->sha1));
				for(i = 0; sscanf(b + i * 2, "%2x", (unsigned int *)&pkg->sha1[i])
						&& i * 2 + 1 < l && i < LENGTH(pkg->sha1); i++);
				break;
			case KEY:
				bzero(pkg->key, sizeof(pkg->key));
				for(i = 0; sscanf(b + i * 2, "%2x", (unsigned int *)&pkg->key[i])
						&& i * 2 + 1 < l && i < LENGTH(pkg->key); i++);
				break;
			case RELTIME:
				pkg->reltime = atol(b);
				break;
			case INSTIME:
				pkg->instime = atol(b);
				break;
			default:
				break;
			}
			l = 0;
			ent++;
		}
		else
			l++;
	}
	if(b)
		free(b);
	if(ent == 0) {
		return 0;
	}
	else if(ent < NENTRIES) {
		freepkg(pkg);
		return -1;
	}
	else {
		return 1;
	}
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
	int i;
	char **l[] = {
		&pkg->name,
		&pkg->ver,
		&pkg->desc,
		&pkg->url,
		&pkg->usef,
		&pkg->repo,
		&pkg->dep,
		&pkg->conflict,
		&pkg->prov,
		&pkg->path,
	};
	for(i = 0; i < LENGTH(l); i++)
		if(*l[i] != NULL) {
			free(*l[i]);
			*l[i] = NULL;
		}
}

void version() {
	fputs("spiceman-" VERSION " - distributed package management tools\n",stderr);
}
