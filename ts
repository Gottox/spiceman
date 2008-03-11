#!/bin/sh
[ "$1" = "." ] && gdb=1
chroot=/work/gnuffy_chroot
make CC=tcc > /dev/null 2>&1 || { make; exit; }
sudo mkdir -p /work/gnuffy_chroot/usr/local/var/db/pkg
sudo cp packages /work/gnuffy_chroot/usr/local/var/db/pkg/packages
cp -f spiceman $chroot/tmp
if [ "$gdb" ]; then
	PATH=$PATH:/tmp sudo gdb chroot "$chroot" /tmp/spiceman "$@"
else
	PATH=$PATH:/tmp sudo chroot "$chroot" /tmp/spiceman "$@"
fi
