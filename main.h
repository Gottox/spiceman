/* Applets */
#include <db.h>
#include <install.h>
#include <remove.h>

#define APPLETPREFIX "sp-"
#define LENGTH(x) sizeof(x)/sizeof(x[0])

struct Applet {
	int (*function)(int, char *[]);
	char *name;
};

static struct Applet applets[] = {
	{ db, APPLETPREFIX "db" },
	{ ins, APPLETPREFIX "ins" },
	{ db, APPLETPREFIX "db" },
};

