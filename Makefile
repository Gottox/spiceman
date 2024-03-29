# spiceman - distributed package management tools
# (c) 2007 Enno Boland

include config.mk

SRC = common.c db.c delete.c download.c filter.c install.c relate.c ui.c \
      validate.c main.c
OBJ = ${SRC:.c=.o}
TARGET = spiceman

all: options ${TARGET}

options:
	@echo spiceman build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${OBJ}: config.mk

main.o: main.c common.h
	@echo CC $@
	@${CC} -c ${CFLAGS} $<

%.o: %.c %.h common.h
	@echo CC $@
	@${CC} -c ${CFLAGS} $<

${TARGET}: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

vercmp: vercmp.c common.o
	@echo CC -o $@
	@${CC} -o $@ vercmp.c common.o ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f -- ${OBJ} ${OBJ:.o=} vercmp ${TARGET} ${TARGET}-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p ${TARGET}-${VERSION}
	@cp -R LICENSE Makefile \
		${TARGET}.1 ${SRC} ${TARGET}-${VERSION}
	@tar -cf ${TARGET}-${VERSION}.tar ${TARGET}-${VERSION}
	@gzip ${TARGET}-${VERSION}.tar
	@rm -rf ${TARGET}-${VERSION}

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
