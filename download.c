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
#include <socket.h>
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
	int i;
	char protofile[BUFSIZ];
	int len = LENGTH(DBPREFIX "/dl/") - 1;
	//FILE *p;
	
	strncpy(protofile, DBPREFIX "/dl/", BUFSIZ);
	for(i = 0; i < BUFSIZ - len && pkg->repo[i]; i++)
		protofile[len + i] = isalnum(pkg->url[i]) ? pkg->url[i] : '_';
	return fhttp(pkg->url);
}

/* builtin http download
 * only as fallback */
FILE *fhttp(const char *url) {
	int sock;
	struct sockaddr_in sin;
	struct hostent *host;
	char *addr, *path, *port;
	char buf[BUFSIZ], urlbuf[BUFSIZ],
		headerbuf[BUFSIZ * 2 + LENGTH(HTTPHEADER)];
	unsigned int p = 80;
	FILE *in;

	strncpy(urlbuf, url, BUFSIZ);
	if(!(addr = strchr(urlbuf, ':')))
		return 0;
	addr++;
	for(; *addr && *addr == '/'; addr++);
	if(!(path = strchr(addr,'/')))
		return 0;
	*path = 0;
	path++;
	if((port = strchr(addr, ':'))) {
		*port = 0;
		port++;
		if(!(p = atoi(port)))
			return 0;
	}
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 0;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	if((sin.sin_addr.s_addr = inet_addr(addr)) == INADDR_NONE) {
		host = gethostbyname(addr);
		if(!host)
			return 0;
		memcpy((char *)&sin.sin_addr, host->h_addr, host->h_length);
	}
	sin.sin_port = htons(p);
	if(connect(sock, (struct sockaddr *)&sin, sizeof(sin)) ||
			(!(in = fdopen(sock, "r")))) {
		shutdown(sock, SHUT_RDWR);
		return 0;
	}
	snprintf(headerbuf, LENGTH(headerbuf), HTTPHEADER, path, addr, p);
	send(sock, headerbuf, strlen(headerbuf), 0);
	if(!fgets(buf, LENGTH(buf), in) ||
			strncmp(buf, RESPONSE, LENGTH(RESPONSE) - 1)) {
		fclose(in);
		return 0;
	}
	while(fgets(buf, LENGTH(buf), in) && *buf != '\n' && *buf != '\r');
	return in;
}

void download_help() {
	APPLETUSAGE("download");
	fputs("	-n	force downloading. Do not use cache.\n", stderr);
}

/* download a Package */
int download(int argc, char *argv[]) {
	FILE *url, *cache;
	int nocache = 0, n, isprocess;
	struct Package pkg;
	char namebuf[LENGTH(CACHEPREFIX) + BUFSIZ];
	char buf[BUFSIZ];

	if(argc == 1 && strcmp("-n", argv[0]) == 0)
		nocache = 1;
	else if(argc >= 1) {
		download_help();
		return EXIT_FAILURE;
	}
	bzero(&pkg,sizeof(pkg));
	while(getpkg(&pkg) > 0) {
		snprintf(namebuf, LENGTH(namebuf), CACHEPREFIX "/%s-%s-%u.tar",
				pkg.name, pkg.ver, pkg.rel);
		if(!(url = fopenurl(&pkg, &isprocess)))
			eprint(0, "Cannot download.");
		if(!(cache = fopen(namebuf, "w")))
			eprint(0, "Cannot open cache file.");
		while((n = fread(buf, sizeof(char), LENGTH(buf), url)) > 0)
			fwrite(buf, sizeof(char), LENGTH(buf), cache);
		pkg.url = namebuf;
		putpkg(&pkg);
		fclose(cache);
		fclose(url);
	}
	freepkg(&pkg);
	return EXIT_SUCCESS;
}
