# spiceman - suckless package management tools
# (c) 2007 Enno Boland

include config.mk

SRC = db.c extract.c delete.c main.c common.c depency.c
OBJ = ${SRC:.c=.o}
TARGET = spiceman
COMMON = common.o

all: options ${TARGET}

options:
	@echo spiceman build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${OBJ}: config.mk

%.o: %.c
	@echo CC $@
	@${CC} -c ${CFLAGS} $<

${TARGET}: ${OBJ}
	@echo LD $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

%: %.c ${COMMON}
	@echo LD $@
	@echo -e "int main(int argc, char *argv[]) { return $@(argc,argv,stdin,stdout); }\n#include \"$<\"" > temp.c
	@${CC} -o $@ temp.c ${COMMON} ${LDFLAGS}
	@rm temp.c


clean:
	@echo cleaning
	@rm -f -- ${OBJ} ${OBJ:.o=} ${TARGET} ${TARGET}-${VERSION}.tar.gz

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
