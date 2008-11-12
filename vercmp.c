#include <stdlib.h>
#include <stdio.h>
#include "common.h"

int main(int argc, char *argv[]) {
	int ret;

	if(argc != 3)
		die(0, "usage: %s <version> <version>", argv[0]);
	ret = vercmp(argv[1], argv[2]);
	if(ret < 0)
		return 2;
	else if(ret > 0)
		return 1;
	return 0;
}
