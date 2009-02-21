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
#include <strings.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"
#include "download.h"

#define HTTPHEADER "GET /%s HTTP/1.0\r\n" \
	"User-Agent: spiceman/" VERSION "\r\n" \
	"Accept: */*\r\n" \
	"Host: %s:%i\r\n" \
	"Connection: Close\r\n\r\n"
#define RESPONSE "HTTP/1.0 200 OK"

/* open URL */
FILE *
fopenurl(const struct Package *pkg, int *isprocess) {
	char *p, *path = astrcpy(0, DBPREFIX "/dl/");
	int l = strlen(path);

	*isprocess = 1;
	path = astrcat(path, pkg->url);
	for(p = path + l; *p && *p != ':'; p++)
		if(!isalnum(*p))
			*p = '_';
	*p = 0;
	fflush(NULL);
	*isprocess = 0;
	free(path);
	return fhttp(pkg->url);
}

/* builtin http download
 * only as fallback */
FILE *fhttp(const char *url) {
	int sock;
	struct sockaddr_in sin;
	struct hostent *host;
	char *addr, *path, *port, *urlbuf;
	char buf[BUFSIZ],
		headerbuf[BUFSIZ * 2 + LENGTH(HTTPHEADER)];
	unsigned int p = 80;
	FILE *in;

	/* parsing URI */
	urlbuf = astrcpy(0, url);
	if(!(addr = strchr(urlbuf, ':'))) {
		free(urlbuf);
		return 0;
	}
	addr++;
	for(; *addr && *addr == '/'; addr++);
	if(!(path = strchr(addr,'/'))) {
		free(urlbuf);
		return 0;
	}
	*path = 0;
	path++;
	if((port = strchr(addr, ':'))) {
		*port = 0;
		port++;
		if(!(p = atoi(port))) {
			free(urlbuf);
			return 0;
		}
	}
	/* initializing socket */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		free(urlbuf);
		return 0;
	}
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	if((sin.sin_addr.s_addr = inet_addr(addr)) == INADDR_NONE) {
		host = gethostbyname(addr);
		if(!host) {
			free(urlbuf);
			return 0;
		}
		memcpy((char *)&sin.sin_addr, host->h_addr, host->h_length);
	}
	sin.sin_port = htons(p);
	if(connect(sock, (struct sockaddr *)&sin, sizeof(sin)) ||
			(!(in = fdopen(sock, "r")))) {
		shutdown(sock, SHUT_RDWR);
		free(urlbuf);
		return 0;
	}
	/* send header and receive header */
	snprintf(headerbuf, LENGTH(headerbuf), HTTPHEADER, path, addr, p);
	send(sock, headerbuf, strlen(headerbuf), 0);
	if(!fgets(buf, LENGTH(buf), in) ||
			strncmp(buf, RESPONSE, LENGTH(RESPONSE) - 1)) {
		fclose(in);
		free(urlbuf);
		return 0;
	}
	while(fgets(buf, LENGTH(buf), in) && *buf != '\n' && *buf != '\r');
	free(urlbuf);
	return in;
}

void download_help() {
	APPLETUSAGE("download");
	fputs("	-n	force downloading. Do not use cache.\n", stderr);
}

/* download a Package */
int download(int argc, char *argv[]) {
	FILE *file, *cache;
	int nocache = 0, n, isprocess;
	struct Package pkg = { 0 };
	char path[LENGTH(CACHEPREFIX) + BUFSIZ];
	char buf[BUFSIZ];

	if(argc == 1 && strcmp("-n", argv[0]) == 0)
		nocache = 1;
	else if(argc >= 1) {
		download_help();
		return EXIT_FAILURE;
	}
	while(getfreepkg(&pkg) > 0) {
		snprintf(path, LENGTH(path), CACHEPREFIX "/dl/%s",
				pkg.repo);
		if(mkdirhier(path))
			die(1, "Cannot create dir `%s`");
		snprintf(path, LENGTH(path), CACHEPREFIX "/dl/%s/%s-%s-%u.tar",
				pkg.repo, pkg.name, pkg.ver, pkg.rel);
		fputs("Getting: ", stderr);
		fputs(pkg.url, stderr);
		fputc('\n', stderr);
		if(!(file = fopenurl(&pkg, &isprocess))) {
			perror("Cannot download");
			continue;
		}
		if((cache = fopen(path, "w"))) {
			while((n = fread(buf, sizeof(char), LENGTH(buf), file)) > 0)
				fwrite(buf, sizeof(char), n, cache);
			snprintf(buf, LENGTH(buf), "file:%s", path);
			pkg.url = buf;
			putpkg(&pkg);
			fclose(cache);
			fputs("Finished: ", stderr);
			fputs(pkg.url, stderr);
			fputc('\n', stderr);
		}
		else
			perror("Cannot open cache file");
		if(isprocess)
			pclose(file);
		else
			fclose(file);
	}
	return EXIT_SUCCESS;
}
