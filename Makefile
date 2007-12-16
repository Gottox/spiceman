# spice - suckless pkgmanager tools
# © 2006-2007 Enno Boland

include config.mk

SRC = spicedb.c spiceins.c spicerm.c spiceget.c libpkg.c
OBJ = ${SRC:.c=.o}
TARGET = spicedb spiceins spicerm spiceget
COMMON = libpkg.o

all: options ${COMMON} ${TARGET}

options:
	@echo spice build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

%.o: %.c
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

% : %.o ${COMMON}
	@echo LD $< $@
	@${CC} -o $@ $< ${COMMON} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f  ${OBJ} ${TARGET} spiceman-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p spiceman-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk \
		spiceman.1 ${SRC} spiceman-${VERSION}
	@tar -cf spiceman-${VERSION}.tar spiceman-${VERSION}
	@gzip spiceman-${VERSION}.tar
	@rm -rf spiceman-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f spiceman ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/spiceman
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < spiceman.1 > ${DESTDIR}${MANPREFIX}/man1/spiceman.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/spiceman.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/spiceman
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/spiceman.1

.PHONY: all options clean dist install uninstall
