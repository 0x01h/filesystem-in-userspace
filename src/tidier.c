/*****************************************************************************************************************
 Compile: $ gcc tidier.c -o tidier -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lfuse -ltidy
 Mount: $ tidier -d website mount_point
******************************************************************************************************************/

#define FUSE_USE_VERSION 26

static const char* tidierVersion = "1.0.0";

#include <sys/types.h>  // unix data types
#include <sys/stat.h>  // file attributes/stats
#include <sys/statvfs.h>  // get mounted filesystem stats
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>  // manipulate file descriptions
#include <sys/xattr.h>  // extended attributes
#include <dirent.h> // format of directory entries 
#include <unistd.h>
#include <fuse.h> // include fuse library functions
#include <tidy/tidy.h>  // html-tidy library functions
#include <tidy/buffio.h>  // treat buffer as input/output

// Global to store our read-write path
char *rw_path;

// Translate a virtual path into its underlying filesystem path
static char* translate_path(const char* path)
{
    printf("translate_path was called!\n");
    printf("path: %s\n", path);

    char *rPath= malloc(sizeof(char)*(strlen(path)+strlen(rw_path)+1));

    printf("rPath: %s\n", rPath);

    strcpy(rPath,rw_path);

    if (rPath[strlen(rPath)-1]=='/') {
        rPath[strlen(rPath)-1]='\0';
    }

    strcat(rPath,path);

    printf("New rPath: %s\n", rPath);
    return rPath;
}

// ls -l
static int tidier_getattr(const char *path, struct stat *st_data)
{
    printf("tidier_getattr was called!\n");
    int res;
    char *upath=translate_path(path);

    res = lstat(upath, st_data);
    free(upath);

    if(res == -1) {
        return -errno;
    }

    return 0;
}

static int tidier_readlink(const char *path, char *buf, size_t size)
{
    printf("tidier_readlink was called!\n");
    int res;
    char *upath=translate_path(path);

    res = readlink(upath, buf, size - 1);
    free(upath);

    if(res == -1) {
        return -errno;
    }

    buf[res] = '\0';
    return 0;
}

// ls
static int tidier_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi)
{
    printf("tidier_readdir was called!\n");
    printf("Directory path: %s\n", path);
    DIR *dp;
    struct dirent *de;
    int res;
    int counter = 0;
    char *filename;
    char *file_extension;
    char *selected_extension = ' ';

    (void) offset;
    (void) fi;

    char *upath=translate_path(path);
    printf("Directory translated path: %s\n", upath);

    dp = opendir(upath);
    free(upath);
    if(dp == NULL) {
        res = -errno;
        return res;
    }

    while((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        // The logic below continue in loop when file doesn't have extension or extension doesn't start with 'h'.
        // But still has to be improved, and gets unique: 144, error: -2 (No such file or directory), outsize: 16 error.
        filename = strdup(de->d_name);
        printf("Filename: %s\n", filename);
        while ((file_extension = strsep(&filename, ".")) != NULL) {
            if (counter == 0) {
                counter++;
            }
            else if (counter == 1) {
                printf("File extension: %s\n", file_extension);
                selected_extension = file_extension;
                break;
            }
            else {
                break;
            }
        }

        counter = 0;
        if ((strlen(selected_extension) > 0) && (selected_extension[0] == 'h'))
            printf("Selected extension: %s\n", selected_extension);
        else
            continue;

        if (filler(buf, de->d_name, &st, 0))
            break;

    }

    closedir(dp);
    return 0;
}

// touch
static int tidier_mknod(const char *path, mode_t mode, dev_t rdev)
{
    printf("tidier_mknod was called!\n");
    (void)path;
    (void)mode;
    (void)rdev;
    return -EROFS;
}

// mkdir
static int tidier_mkdir(const char *path, mode_t mode)
{
    printf("tidier_mkdir was called!\n");
    (void)path;
    (void)mode;
    return -EROFS;
}

// rm
static int tidier_unlink(const char *path)
{
    printf("tidier_unlink was called!\n");
    (void)path;
    return -EROFS;
}


// rmdir
static int tidier_rmdir(const char *path)
{
    printf("tidier_rmdir was called!\n");
    (void)path;
    return -EROFS;
}


// ln -s
static int tidier_symlink(const char *from, const char *to)
{
    printf("tidier_symlink was called!\n");
    (void)from;
    (void)to;
    return -EROFS;
}

// rename
static int tidier_rename(const char *from, const char *to)
{
    printf("tidier_rename was called!\n");
    (void)from;
    (void)to;
    return -EROFS;
}

// ln  -l
static int tidier_link(const char *from, const char *to)
{
    printf("tidier_link was called!\n");
    (void)from;
    (void)to;
    return -EROFS;
}

static int tidier_chmod(const char *path, mode_t mode)
{
    printf("tidier_chmod was called!\n");
    (void)path;
    (void)mode;
    return -EROFS;

}

static int tidier_chown(const char *path, uid_t uid, gid_t gid)
{
    printf("tidier_chown was called!\n");
    (void)path;
    (void)uid;
    (void)gid;
    return -EROFS;
}

static int tidier_truncate(const char *path, off_t size)
{
    printf("tidier_truncate was called!\n");
    (void)path;
    (void)size;
    return -EROFS;
}

static int tidier_utime(const char *path, struct utimbuf *buf)
{
    printf("tidier_utime was called!\n");
    (void)path;
    (void)buf;
    return -EROFS;
}

static int tidier_open(const char *path, struct fuse_file_info *finfo)
{
    printf("tidier_open was called!\n");
    int res;

    int flags = finfo->flags;

    if ((flags & O_WRONLY) || (flags & O_RDWR) || 
    (flags & O_CREAT) || (flags & O_EXCL) || 
    (flags & O_TRUNC) || (flags & O_APPEND)) {
        return -EROFS;
    }

    char *upath=translate_path(path);
    printf("Translated path: %s\n", upath);

    res = open(upath, flags);

    free(upath);
    if(res == -1) {
        return -errno;
    }
    close(res);
    return 0;
}

static int tidier_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{
    printf("tidier_read was called!\n");
    int fd;
    int res;
    (void)finfo;

    printf("File path: %s\n", path);
    char *upath=translate_path(path);
    printf("Translated path: %s\n", upath);

    fd = open(upath, O_RDONLY);
    free(upath);
    if(fd == -1) {
        res = -errno;
        return res;
    }
    res = pread(fd, buf, size, offset);

    if(res == -1) {
        res = -errno;
    }
    close(fd);

  	TidyBuffer output = {0};
  	TidyBuffer errbuf = {0};
  	int rc = -1;
  	Bool ok;

  	TidyDoc tdoc = tidyCreate();                     // Initialize "document"

	ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Convert to XHTML
  	if ( ok )
    		rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
  	if ( rc >= 0 )
    		rc = tidyParseString( tdoc, buf );           // Parse the input
  	if ( rc >= 0 )
    		rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
  	if ( rc >= 0 )
    		rc = tidyRunDiagnostics( tdoc );               // Kvetch
  	if ( rc > 1 )                                    // If error, force output.
    		rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
	if ( rc >= 0 )
       		 rc = tidySaveBuffer( tdoc, &output );        // Pretty Print
		//önce bufı boşaltıp yeni sizea göre yer açmak gerekebilir.
			//free(buf);
			buf = output.bp;
			//char * newSize = output.bp;
			//buf = (char *)malloc(strlen(newSize));
			//strcpy(buf, newSize);
			//buf = malloc(sizeof(char)*strlen(newSize));
			//uint sizer = strlen(newSize);
    			//rc = tidySaveString( tdoc, buf, &sizer);          // tidy hali buf'a yazma(size = ?)
		//or
			//rc = tidySaveFile( tdoc, mirrorfilename);          // Doğrudan  mirror dosyaya yazma 
		//or
			//rc = tidySaveBuffer( tdoc, &amp;output );

			//strcpy(buf, newSize);
  	tidyBufFree( &output );
	tidyBufFree( &errbuf );
  	tidyRelease( tdoc );

    return res;
}

static int tidier_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{
    printf("tidier_write was called!\n");
    (void)path;
    (void)buf;
    (void)size;
    (void)offset;
    (void)finfo;
    return -EROFS;
}

static int tidier_statfs(const char *path, struct statvfs *st_buf)
{
    printf("tidier_statfs was called!\n");
    printf("File path (statfs): %s\n", path);
    int res;
    char *upath=translate_path(path);

    printf("Translated path (statfs): %s\n", upath);

    res = statvfs(upath, st_buf);
    free(upath);
    if (res == -1) {
        return -errno;
    }
    return 0;
}

static int tidier_release(const char *path, struct fuse_file_info *finfo)
{
    printf("tidier_release was called!\n");
    (void) path;
    (void) finfo;
    return 0;
}

static int tidier_fsync(const char *path, int crap, struct fuse_file_info *finfo)
{
    printf("tidier_fsync was called!\n");
    (void) path;
    (void) crap;
    (void) finfo;
    return 0;
}

static int tidier_access(const char *path, int mode)
{
    printf("tidier_access was called!\n");
    int res;
    char *upath=translate_path(path);

    if (mode & W_OK)
        return -EROFS;

    res = access(upath, mode);
    free(upath);
    if (res == -1) {
        return -errno;
    }
    return res;
}

/*
 * Set the value of an extended attribute
 */
static int tidier_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    printf("tidier_setxattr was called!\n");
    (void)path;
    (void)name;
    (void)value;
    (void)size;
    (void)flags;
    return -EROFS;
}

/*
 * Get the value of an extended attribute.
 */
static int tidier_getxattr(const char *path, const char *name, char *value, size_t size)
{
    printf("tidier_getxattr was called!\n");
    int res;

    char *upath=translate_path(path);
    res = lgetxattr(upath, name, value, size);
    free(upath);
    if(res == -1) {
        return -errno;
    }
    return res;
}

/*
 * List the supported extended attributes.
 */
static int tidier_listxattr(const char *path, char *list, size_t size)
{
    printf("tidier_listxattr was called!\n");
    int res;

    char *upath=translate_path(path);
    res = llistxattr(upath, list, size);
    free(upath);
    if(res == -1) {
        return -errno;
    }
    return res;

}

/*
 * Remove an extended attribute.
 */
static int tidier_removexattr(const char *path, const char *name)
{
    printf("tidier_removexattr was called!\n");
    (void)path;
    (void)name;
    return -EROFS;

}

struct fuse_operations tidier_oper = {
        .getattr     = tidier_getattr,
        .readlink    = tidier_readlink,
        .readdir     = tidier_readdir, // read contents of the directory
        .mknod       = tidier_mknod, // create a file node. this will be called for all non-directory, non-symlink nodes
        .mkdir       = tidier_mkdir, // make directory
        .symlink     = tidier_symlink,
        .unlink      = tidier_unlink, // remove a file
        .rmdir       = tidier_rmdir, // remove directory
        .rename      = tidier_rename, // rename a file
        .link        = tidier_link, // create a hard link for a file
        .chmod       = tidier_chmod,
        .chown       = tidier_chown,
        .truncate    = tidier_truncate,
        .utime       = tidier_utime,
        .open        = tidier_open, // file open operation
        .read        = tidier_read, // read data from an open file
        .write       = tidier_write, // write data to an open file
        .statfs      = tidier_statfs, // get filesystem statistics
        .release     = tidier_release, // release an open file, each opened file must be released
        .fsync       = tidier_fsync, // synchronize file contents
        .access      = tidier_access,

        /* Extended attributes support for userland interaction */
        .setxattr    = tidier_setxattr, // set extended attributes
        .getxattr    = tidier_getxattr, // get extended attributes
        .listxattr   = tidier_listxattr, // list extended attributes
        .removexattr = tidier_removexattr // remove extended attributes
};
enum {
    KEY_HELP,
    KEY_VERSION,
};

static void usage(const char* progname)
{
    fprintf(stdout,
            "usage: %s readwritepath mountpoint [options]\n"
            "\n"
            "   Mounts readwritepath as a read-only mount at mountpoint\n"
            "\n"
            "general options:\n"
            "   -o opt,[opt...]     mount options\n"
            "   -h  --help          print help\n"
            "   -V  --version       print version\n"
            "\n", progname);
}

static int tidier_parse_opt(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
    (void) data;

    switch (key)
    {
        case FUSE_OPT_KEY_NONOPT:
            if (rw_path == 0)
            {
                rw_path = strdup(arg);
                return 0;
            }
            else
            {
                return 1;
            }
        case FUSE_OPT_KEY_OPT:
            return 1;
        case KEY_HELP:
            usage(outargs->argv[0]);
            exit(0);
        case KEY_VERSION:
            fprintf(stdout, "Tidier version %s\n", tidierVersion);
            exit(0);
        default:
            fprintf(stderr, "See `%s -h' for usage.\n", outargs->argv[0]);
            exit(1);
    }
    return 1;
}

static struct fuse_opt tidier_opts[] = {
        FUSE_OPT_KEY("-h",          KEY_HELP),
        FUSE_OPT_KEY("--help",      KEY_HELP),
        FUSE_OPT_KEY("-V",          KEY_VERSION),
        FUSE_OPT_KEY("--version",   KEY_VERSION),
        FUSE_OPT_END
};

int main(int argc, char *argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int res;

    res = fuse_opt_parse(&args, &rw_path, tidier_opts, tidier_parse_opt);
    if (res != 0)
    {
        fprintf(stderr, "Invalid arguments!\n");
        fprintf(stderr, "see `%s -h' for usage.\n", argv[0]);
        exit(1);
    }
    if (rw_path == 0)
    {
        fprintf(stderr, "Missing readwritepath!\n");
        fprintf(stderr, "see `%s -h' for usage.\n", argv[0]);
        exit(1);
    }

    fuse_main(args.argc, args.argv, &tidier_oper, NULL);

    return 0;
}
