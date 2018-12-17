gcc tidier.c -o tidier -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lfuse -ltidy
./tidier -d website/ mirror/
