#!/bin/sh
chroot=/work/gnuffy_chroot
make CC=tcc > /dev/null 2>&1 || { make; exit; }
cp -f spiceman $chroot/tmp
PATH=$PATH:/tmp sudo chroot "$chroot" /tmp/spiceman "$@"
