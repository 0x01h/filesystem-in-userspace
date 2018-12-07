# filesystem-in-userspace
FUSE (filesystem in userspace) for Linux.


## Installation
Install FUSE: ```libfuse-dev```

## How to generate a mirror folder
1- Compile rofs.c: ```gcc rofs.c -o rofs -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lfuse```
2- Manually create a folder that will mirror another folder: you can name it as ```mirror```
3- Mount rofs, specialized for the purpose: ```./rofs -d website/ mirror/```
This should take half a minute to complete. Then you can access the mirrored folder. As said above, ```mirror``` folder has the same structure, is read only and reflects any changes immediately.
