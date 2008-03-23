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
CACHEPREFIX = .


# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

# flags
CFLAGS = -O0 -g -Wall -Werror ${INCS} -DVERSION=\"${VERSION}\" \
	-DDBPREFIX=\"${DBPREFIX}\" -DCACHEPREFIX=\"${CACHEPREFIX}\" \
	-DINITREPONAME=\"${INITREPONAME}\" -DINITREPOADDR=\"${INITREPOADDR}\"
LDFLAGS = -g ${LIBS}

# compiler and linker
CC = cc
