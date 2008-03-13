# spiceman version
VERSION = 0.0

# initial config:
DEFREPO = "R:gtx-base:0.0:1:0::::gtx-base::::::::::http://s01.de/~gottox/rep/gtx-base"

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
#DBPREFIX = ${PREFIX}/var/db/pkg
# This is better for debugging
DBPREFIX = .

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

# flags
CFLAGS = -static -Os -g -Wall -Werror ${INCS} -DVERSION=\"${VERSION}\" -DDEFREPO="\"${DEFREPO}\"" -DDBPREFIX=\"${DBPREFIX}\"
LDFLAGS = -g ${LIBS}

# compiler and linker
CC = cc
