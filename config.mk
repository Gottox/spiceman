# spiceman version
VERSION = 0.0

# builtin repository link:
INITREPONAME="init-gottox"
INITREPOADDR="http://s01.de/~gottox/repositories/init/repos.tar"

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
#CFLAGS = -static -Os -g -Wall -Werror ${INCS} -DVERSION=\"${VERSION}\" -DDEFREPO="\"${DEFREPO}\"" -DDBPREFIX=\"${DBPREFIX}\"
CFLAGS = -O0 -g -Wall -Werror ${INCS} -DVERSION=\"${VERSION}\" -DDBPREFIX=\"${DBPREFIX}\" -DINITREPONAME=\"${INITREPONAME}\" -DINITREPOADDR=\"${INITREPOADDR}\"
LDFLAGS = -static -g ${LIBS}

# compiler and linker
CC = cc
