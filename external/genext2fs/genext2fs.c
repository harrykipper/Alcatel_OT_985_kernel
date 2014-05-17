/* vi: set sw=8 ts=8: */
// genext2fs.c
//
// ext2 filesystem generator for embedded systems
// Copyright (C) 2000 Xavier Bestel <xavier.bestel@free.fr>
//
// Please direct support requests to genext2fs-devel@lists.sourceforge.net
//
// 'du' portions taken from coreutils/du.c in busybox:
//	Copyright (C) 1999,2000 by Lineo, inc. and John Beppu
//	Copyright (C) 1999,2000,2001 by John Beppu <beppu@codepoet.org>
//	Copyright (C) 2002  Edward Betts <edward@debian.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version
// 2 of the License.
//
// Changes:
// 	 3 Jun 2000	Initial release
// 	 6 Jun 2000	Bugfix: fs size multiple of 8
// 			Bugfix: fill blocks with inodes
// 	14 Jun 2000	Bugfix: bad chdir() with -d option
// 			Bugfix: removed size=8n constraint
// 			Changed -d file to -f file
// 			Added -e option
// 	22 Jun 2000	Changed types for 64bits archs
// 	24 Jun 2000	Added endianness swap
// 			Bugfix: bad dir name lookup
// 	03 Aug 2000	Bugfix: ind. blocks endian swap
// 	09 Aug 2000	Bugfix: symlinks endian swap
// 	01 Sep 2000	Bugfix: getopt returns int, not char	proski@gnu.org
// 	10 Sep 2000	Bugfix: device nodes endianness		xavier.gueguen@col.bsf.alcatel.fr
// 			Bugfix: getcwd values for Solaris	xavier.gueguen@col.bsf.alcatel.fr
// 			Bugfix: ANSI scanf for non-GNU C	xavier.gueguen@col.bsf.alcatel.fr
// 	28 Jun 2001	Bugfix: getcwd differs for Solaris/GNU	mike@sowbug.com
// 	 8 Mar 2002	Bugfix: endianness swap of x-indirects
// 	23 Mar 2002	Bugfix: test for IFCHR or IFBLK was flawed
// 	10 Oct 2002	Added comments,makefile targets,	vsundar@ixiacom.com    
// 			endianess swap assert check.  
// 			Copyright (C) 2002 Ixia communications
// 	12 Oct 2002	Added support for triple indirection	vsundar@ixiacom.com
// 			Copyright (C) 2002 Ixia communications
// 	14 Oct 2002	Added support for groups		vsundar@ixiacom.com
// 			Copyright (C) 2002 Ixia communications
// 	 5 Jan 2003	Bugfixes: reserved inodes should be set vsundar@usc.edu
// 			only in the first group; directory names
// 			need to be null padded at the end; and 
// 			number of blocks per group should be a 
// 			multiple of 8. Updated md5 values. 
// 	 6 Jan 2003	Erik Andersen <andersee@debian.org> added
// 			mkfs.jffs2 compatible device table support,
// 			along with -q, -P, -U


#include <config.h>
#include <stdio.h>

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#elif MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
# if HAVE_STDDEF_H
#  include <stddef.h>
# endif
#endif

#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif

#if HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#if HAVE_LIBGEN_H
# include <libgen.h>
#endif

#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_GETOPT_H
# include <getopt.h>
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#endif

#include <private/android_filesystem_config.h>
unsigned source_path_len = 0;

struct stats {
	unsigned long nblocks;
	unsigned long ninodes;
};

// block size

#define BLOCKSIZE         1024
#define BLOCKS_PER_GROUP  8192
#define INODES_PER_GROUP  8192
/* Percentage of blocks that are reserved.*/
#define RESERVED_BLOCKS       5/100
#define MAX_RESERVED_BLOCKS  25/100


// inode block size (why is it != BLOCKSIZE ?!?)

#define INODE_BLOCKSIZE   512
#define INOBLK            (BLOCKSIZE / INODE_BLOCKSIZE)

// reserved inodes

#define EXT2_BAD_INO         1     // Bad blocks inode
#define EXT2_ROOT_INO        2     // Root inode
#define EXT2_ACL_IDX_INO     3     // ACL inode
#define EXT2_ACL_DATA_INO    4     // ACL inode
#define EXT2_BOOT_LOADER_INO 5     // Boot loader inode
#define EXT2_UNDEL_DIR_INO   6     // Undelete directory inode
#define EXT2_FIRST_INO       11    // First non reserved inode

// magic number for ext2

#define EXT2_MAGIC_NUMBER  0xEF53


// direct/indirect block addresses

#define EXT2_NDIR_BLOCKS   11                    // direct blocks
#define EXT2_IND_BLOCK     12                    // indirect block
#define EXT2_DIND_BLOCK    13                    // double indirect block
#define EXT2_TIND_BLOCK    14                    // triple indirect block
#define EXT2_INIT_BLOCK    0xFFFFFFFF            // just initialized (not really a block address)

// end of a block walk

#define WALK_END           0xFFFFFFFE

// file modes

#define FM_IFMT    0170000	// format mask
#define FM_IFSOCK  0140000	// socket
#define FM_IFLNK   0120000	// symbolic link
#define FM_IFREG   0100000	// regular file

#define FM_IFBLK   0060000	// block device
#define FM_IFDIR   0040000	// directory
#define FM_IFCHR   0020000	// character device
#define FM_IFIFO   0010000	// fifo

#define FM_IMASK   0007777	// *all* perms mask for everything below

#define FM_ISUID   0004000	// SUID
#define FM_ISGID   0002000	// SGID
#define FM_ISVTX   0001000	// sticky bit

#define FM_IRWXU   0000700	// entire "user" mask
#define FM_IRUSR   0000400	// read
#define FM_IWUSR   0000200	// write
#define FM_IXUSR   0000100	// execute

#define FM_IRWXG   0000070	// entire "group" mask
#define FM_IRGRP   0000040	// read
#define FM_IWGRP   0000020	// write
#define FM_IXGRP   0000010	// execute

#define FM_IRWXO   0000007	// entire "other" mask
#define FM_IROTH   0000004	// read
#define FM_IWOTH   0000002	// write
#define FM_IXOTH   0000001	// execute

// options

#define OP_HOLES     0x01       // make files with holes

/* Defines for accessing group details */

// Number of groups in the filesystem
#define GRP_NBGROUPS(fs) \
	(((fs)->sb.s_blocks_count - fs->sb.s_first_data_block + \
	  (fs)->sb.s_blocks_per_group - 1) / (fs)->sb.s_blocks_per_group)

// Get group block bitmap (bbm) given the group number
#define GRP_GET_GROUP_BBM(fs,grp) ( get_blk((fs),(fs)->gd[(grp)].bg_block_bitmap) )

// Get group inode bitmap (ibm) given the group number
#define GRP_GET_GROUP_IBM(fs,grp) ( get_blk((fs),(fs)->gd[(grp)].bg_inode_bitmap) )
		
// Given an inode number find the group it belongs to
#define GRP_GROUP_OF_INODE(fs,nod) ( ((nod)-1) / (fs)->sb.s_inodes_per_group)

//Given an inode number get the inode bitmap that covers it
#define GRP_GET_INODE_BITMAP(fs,nod) \
	( GRP_GET_GROUP_IBM((fs),GRP_GROUP_OF_INODE((fs),(nod))) )

//Given an inode number find its offset within the inode bitmap that covers it
#define GRP_IBM_OFFSET(fs,nod) \
	( (nod) - GRP_GROUP_OF_INODE((fs),(nod))*(fs)->sb.s_inodes_per_group )

// Given a block number find the group it belongs to
#define GRP_GROUP_OF_BLOCK(fs,blk) ( ((blk)-1) / (fs)->sb.s_blocks_per_group)
	
//Given a block number get the block bitmap that covers it
#define GRP_GET_BLOCK_BITMAP(fs,blk) \
	( GRP_GET_GROUP_BBM((fs),GRP_GROUP_OF_BLOCK((fs),(blk))) )

//Given a block number find its offset within the block bitmap that covers it
#define GRP_BBM_OFFSET(fs,blk) \
	( (blk) - GRP_GROUP_OF_BLOCK((fs),(blk))*(fs)->sb.s_blocks_per_group )


// used types

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;


// the GNU C library has a wonderful scanf("%as", string) which will
// allocate the string with the right size, good to avoid buffer
// overruns. the following macros use it if available or use a
// hacky workaround
// moreover it will define a snprintf() like a sprintf(), i.e.
// without the buffer overrun checking, to work around bugs in
// older solaris. Note that this is still not very portable, in that
// the return value cannot be trusted.

#if 0 // SCANF_CAN_MALLOC
// C99 define "a" for floating point, so you can have runtime surprise
// according the library versions
# define SCANF_PREFIX "a"
# define SCANF_STRING(s) (&s)
#else
# define SCANF_PREFIX "511"
# define SCANF_STRING(s) (s = malloc(512))
#endif /* SCANF_CAN_MALLOC */

#if PREFER_PORTABLE_SNPRINTF
static inline int
portable_snprintf(char *str, size_t n, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsprintf(str, fmt, ap);
	va_end(ap);
	return ret;
}
# define SNPRINTF portable_snprintf
#else
# define SNPRINTF snprintf
#endif /* PREFER_PORTABLE_SNPRINTF */

#if !HAVE_GETLINE
// getline() replacement for Darwin and Solaris etc.
// This code uses backward seeks (unless rchunk is set to 1) which can't work
// on pipes etc. However, add2fs_from_file() only calls getline() for
// regular files, so a larger rchunk and backward seeks are okay.

ssize_t 
getdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
	char *p;                    // reads stored here
	size_t const rchunk = 512;  // number of bytes to read
	size_t const mchunk = 512;  // number of extra bytes to malloc
	size_t m = rchunk + 1;      // initial buffer size
	
	if (*lineptr) {
		if (*n < m) {
			*lineptr = (char*)realloc(*lineptr, m);
			if (!*lineptr) return -1;
			*n = m;
		}
	} else {
		*lineptr = (char*)malloc(m);
		if (!*lineptr) return -1;
		*n = m;
	}

	m = 0; // record length including seperator

	do {
		size_t i;     // number of bytes read etc
		size_t j = 0; // number of bytes searched

		p = *lineptr + m;

		i = fread(p, 1, rchunk, stream);
		if (i < rchunk && ferror(stream))
			return -1;
		while (j < i) {
			++j;
			if (*p++ == (char)delim) {
				*p = '\0';
				if (j != i) {
					if (fseek(stream, j - i, SEEK_CUR))
						return -1;
					if (feof(stream))
						clearerr(stream);
				}
				m += j;
				return m;
			}
		}

		m += j;
		if (feof(stream)) {
			if (m) return m;
			if (!i) return -1;
		}

		// allocate space for next read plus possible null terminator
		i = ((m + (rchunk + 1 > mchunk ? rchunk + 1 : mchunk) +
		      mchunk - 1) / mchunk) * mchunk;
		if (i != *n) {
			*lineptr = (char*)realloc(*lineptr, i);
			if (!*lineptr) return -1;
			*n = i;
		}
	} while (1);
}
#define getline(a,b,c) getdelim(a,b,'\n',c)
#endif /* HAVE_GETLINE */

// Convert a numerical string to a float, and multiply the result by an
// IEC or SI multiplier if provided; supported multipliers are Ki, Mi, Gi, k, M
// and G.

float
SI_atof(const char *nptr)
{
	float f = 0;
	float m = 1;
	char *suffixptr;

#if HAVE_STRTOF
	f = strtof(nptr, &suffixptr);
#else
	f = (float)strtod(nptr, &suffixptr);
#endif /* HAVE_STRTOF */

	if (*suffixptr) {
		if (!strcmp(suffixptr, "Ki"))
			m = 1 << 10;
		else if (!strcmp(suffixptr, "Mi"))
			m = 1 << 20;
		else if (!strcmp(suffixptr, "Gi"))
			m = 1 << 30;
		else if (!strcmp(suffixptr, "k"))
			m = 1000;
		else if (!strcmp(suffixptr, "M"))
			m = 1000 * 1000;
		else if (!strcmp(suffixptr, "G"))
			m = 1000 * 1000 * 1000;
	}
	return f * m;
}

// endianness swap

static inline uint16
swab16(uint16 val)
{
	return (val >> 8) | (val << 8);
}

static inline uint32
swab32(uint32 val)
{
	return ((val>>24) | ((val>>8)&0xFF00) |
			((val<<8)&0xFF0000) | (val<<24));
}


// on-disk structures
// this trick makes me declare things only once
// (once for the structures, once for the endianness swap)

#define superblock_decl \
	udecl32(s_inodes_count)        /* Count of inodes in the filesystem */ \
	udecl32(s_blocks_count)        /* Count of blocks in the filesystem */ \
	udecl32(s_r_blocks_count)      /* Count of the number of reserved blocks */ \
	udecl32(s_free_blocks_count)   /* Count of the number of free blocks */ \
	udecl32(s_free_inodes_count)   /* Count of the number of free inodes */ \
	udecl32(s_first_data_block)    /* The first block which contains data */ \
	udecl32(s_log_block_size)      /* Indicator of the block size */ \
	decl32(s_log_frag_size)        /* Indicator of the size of the fragments */ \
	udecl32(s_blocks_per_group)    /* Count of the number of blocks in each block group */ \
	udecl32(s_frags_per_group)     /* Count of the number of fragments in each block group */ \
	udecl32(s_inodes_per_group)    /* Count of the number of inodes in each block group */ \
	udecl32(s_mtime)               /* The time that the filesystem was last mounted */ \
	udecl32(s_wtime)               /* The time that the filesystem was last written to */ \
	udecl16(s_mnt_count)           /* The number of times the file system has been mounted */ \
	decl16(s_max_mnt_count)        /* The number of times the file system can be mounted */ \
	udecl16(s_magic)               /* Magic number indicating ex2fs */ \
	udecl16(s_state)               /* Flags indicating the current state of the filesystem */ \
	udecl16(s_errors)              /* Flags indicating the procedures for error reporting */ \
	udecl16(s_minor_rev_level)     /* The minor revision level of the filesystem */ \
	udecl32(s_lastcheck)           /* The time that the filesystem was last checked */ \
	udecl32(s_checkinterval)       /* The maximum time permissable between checks */ \
	udecl32(s_creator_os)          /* Indicator of which OS created the filesystem */ \
	udecl32(s_rev_level)           /* The revision level of the filesystem */ \
	udecl16(s_def_resuid)          /* The default uid for reserved blocks */ \
	udecl16(s_def_resgid)          /* The default gid for reserved blocks */

#define groupdescriptor_decl \
	udecl32(bg_block_bitmap)       /* Block number of the block bitmap */ \
	udecl32(bg_inode_bitmap)       /* Block number of the inode bitmap */ \
	udecl32(bg_inode_table)        /* Block number of the inode table */ \
	udecl16(bg_free_blocks_count)  /* Free blocks in the group */ \
	udecl16(bg_free_inodes_count)  /* Free inodes in the group */ \
	udecl16(bg_used_dirs_count)    /* Number of directories in the group */ \
	udecl16(bg_pad)

#define inode_decl \
	udecl16(i_mode)                /* Entry type and file mode */ \
	udecl16(i_uid)                 /* User id */ \
	udecl32(i_size)                /* File/dir size in bytes */ \
	udecl32(i_atime)               /* Last access time */ \
	udecl32(i_ctime)               /* Creation time */ \
	udecl32(i_mtime)               /* Last modification time */ \
	udecl32(i_dtime)               /* Deletion time */ \
	udecl16(i_gid)                 /* Group id */ \
	udecl16(i_links_count)         /* Number of (hard) links to this inode */ \
	udecl32(i_blocks)              /* Number of blocks used (1 block = 512 bytes) */ \
	udecl32(i_flags)               /* ??? */ \
	udecl32(i_reserved1) \
	utdecl32(i_block,15)           /* Blocks table */ \
	udecl32(i_version)             /* ??? */ \
	udecl32(i_file_acl)            /* File access control list */ \
	udecl32(i_dir_acl)             /* Directory access control list */ \
	udecl32(i_faddr)               /* Fragment address */ \
	udecl8(i_frag)                 /* Fragments count*/ \
	udecl8(i_fsize)                /* Fragment size */ \
	udecl16(i_pad1)

#define directory_decl \
	udecl32(d_inode)               /* Inode entry */ \
	udecl16(d_rec_len)             /* Total size on record */ \
	udecl16(d_name_len)            /* Size of entry name */

#define decl8(x) int8 x;
#define udecl8(x) uint8 x;
#define decl16(x) int16 x;
#define udecl16(x) uint16 x;
#define decl32(x) int32 x;
#define udecl32(x) uint32 x;
#define utdecl32(x,n) uint32 x[n];

typedef struct
{
	superblock_decl
	uint32 s_reserved[235];       // Reserved
} superblock;

typedef struct
{
	groupdescriptor_decl
	uint32 bg_reserved[3];
} groupdescriptor;

typedef struct
{
	inode_decl
	uint32 i_reserved2[2];
} inode;

typedef struct
{
	directory_decl
	char d_name[0];
} directory;

typedef uint8 block[BLOCKSIZE];

   
typedef struct
{
	uint32 bnum;
	uint32 bpdir;
	uint32 bpind;
	uint32 bpdind;
	uint32 bptind;
} blockwalker;


/* Filesystem structure that support groups */
#if BLOCKSIZE == 1024
typedef struct
{
	block zero;            // The famous block 0
	superblock sb;         // The superblock
	groupdescriptor gd[0]; // The group descriptors
} filesystem;
#else
#error UNHANDLED BLOCKSIZE
#endif

// now the endianness swap

#undef decl8
#undef udecl8
#undef decl16
#undef udecl16
#undef decl32
#undef udecl32
#undef utdecl32

#define decl8(x)
#define udecl8(x)
#define decl16(x) this->x = swab16(this->x);
#define udecl16(x) this->x = swab16(this->x);
#define decl32(x) this->x = swab32(this->x);
#define udecl32(x) this->x = swab32(this->x);
#define utdecl32(x,n) { int i; for(i=0; i<n; i++) this->x[i] = swab32(this->x[i]); }

#define HDLINK_CNT   16
static int32 hdlink_cnt = HDLINK_CNT;
struct hdlink_s
{
	uint32	src_inode;
	uint32	dst_nod;
};

struct hdlinks_s 
{
	int32 count;
	struct hdlink_s *hdl;
};

static struct hdlinks_s hdlinks;

static void
swap_sb(superblock *sb)
{
#define this sb
	superblock_decl
#undef this
}

static void
swap_gd(groupdescriptor *gd)
{
#define this gd
	groupdescriptor_decl
#undef this
}

static void
swap_nod(inode *nod)
{
#define this nod
	inode_decl
#undef this
}

static void
swap_dir(directory *dir)
{
#define this dir
	directory_decl
#undef this
}

static void
swap_block(block b)
{
	int i;
	uint32 *blk = (uint32*)b;
	for(i = 0; i < BLOCKSIZE/4; i++)
		blk[i] = swab32(blk[i]);
}

#undef decl8
#undef udecl8
#undef decl16
#undef udecl16
#undef decl32
#undef udecl32
#undef utdecl32

static char * app_name;
static const char *const memory_exhausted = "memory exhausted";

// error (un)handling
static void
verror_msg(const char *s, va_list p)
{
	fflush(stdout);
	fprintf(stderr, "%s: ", app_name);
	vfprintf(stderr, s, p);
}
static void
error_msg(const char *s, ...)
{
	va_list p;
	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
}

static void
error_msg_and_die(const char *s, ...)
{
	va_list p;
	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
}

static void
vperror_msg(const char *s, va_list p)
{
	int err = errno;
	if (s == 0)
		s = "";
	verror_msg(s, p);
	if (*s)
		s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
}

static void
perror_msg_and_die(const char *s, ...)
{
	va_list p;
	va_start(p, s);
	vperror_msg(s, p);
	va_end(p);
	exit(EXIT_FAILURE);
}

static FILE *
xfopen(const char *path, const char *mode)
{
	FILE *fp;
	if ((fp = fopen(path, mode)) == NULL)
		perror_msg_and_die("%s", path);
	return fp;
}

static char *
xstrdup(const char *s)
{
	char *t;

	if (s == NULL)
		return NULL;
	t = strdup(s);
	if (t == NULL)
		error_msg_and_die(memory_exhausted);
	return t;
}

static void *
xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		error_msg_and_die(memory_exhausted);
	return ptr;
}

static char *
xreadlink(const char *path)
{
	static const int GROWBY = 80; /* how large we will grow strings by */

	char *buf = NULL;
	int bufsize = 0, readsize = 0;

	do {
		buf = xrealloc(buf, bufsize += GROWBY);
		readsize = readlink(path, buf, bufsize); /* 1st try */
		if (readsize == -1) {
			perror_msg_and_die("%s:%s", app_name, path);
		}
	}
	while (bufsize < readsize + 1);

	buf[readsize] = '\0';
	return buf;
}

int
is_hardlink(ino_t inode)
{
	int i;

	for(i = 0; i < hdlinks.count; i++) {
		if(hdlinks.hdl[i].src_inode == inode)
			return i;
	}
	return -1;
}

// printf helper macro
#define plural(a) (a), ((a) > 1) ? "s" : ""

// temporary working block
static inline uint8 *
get_workblk(void)
{
	unsigned char* b=calloc(1,BLOCKSIZE);
	return b;
}
static inline void
free_workblk(block b)
{
	free(b);
}

/* Rounds qty upto a multiple of siz. siz should be a power of 2 */
static inline uint32
rndup(uint32 qty, uint32 siz)
{
	return (qty + (siz - 1)) & ~(siz - 1);
}

// check if something is allocated in the bitmap
static inline uint32
allocated(block b, uint32 item)
{
	return b[(item-1) / 8] & (1 << ((item-1) % 8));
}

// return a given block from a filesystem
static inline uint8 *
get_blk(filesystem *fs, uint32 blk)
{
	return (uint8*)fs + blk*BLOCKSIZE;
}

// return a given inode from a filesystem
static inline inode *
get_nod(filesystem *fs, uint32 nod)
{
	int grp,offset;
	inode *itab;

	offset = GRP_IBM_OFFSET(fs,nod);
	grp = GRP_GROUP_OF_INODE(fs,nod);
	itab = (inode *)get_blk(fs, fs->gd[grp].bg_inode_table);
	return itab+offset-1;
}

// allocate a given block/inode in the bitmap
// allocate first free if item == 0
static uint32
allocate(block b, uint32 item)
{
	if(!item)
	{
		int i;
		uint8 bits;
		for(i = 0; i < BLOCKSIZE; i++)
			if((bits = b[i]) != (uint8)-1)
			{
				int j;
				for(j = 0; j < 8; j++)
					if(!(bits & (1 << j)))
						break;
				item = i * 8 + j + 1;
				break;
			}
		if(i == BLOCKSIZE)
			return 0;
	}
	b[(item-1) / 8] |= (1 << ((item-1) % 8));
	return item;
}

// deallocate a given block/inode
static void
deallocate(block b, uint32 item)
{
	b[(item-1) / 8] &= ~(1 << ((item-1) % 8));
}

// allocate a block
static uint32
alloc_blk(filesystem *fs, uint32 nod)
{
	uint32 bk=0;
	uint32 grp,nbgroups;

	grp = GRP_GROUP_OF_INODE(fs,nod);
	nbgroups = GRP_NBGROUPS(fs);
	if(!(bk = allocate(get_blk(fs,fs->gd[grp].bg_block_bitmap), 0))) {
		for(grp=0;grp<nbgroups && !bk;grp++)
			bk=allocate(get_blk(fs,fs->gd[grp].bg_block_bitmap),0);
		grp--;
	}
	if (!bk)
		error_msg_and_die("couldn't allocate a block (no free space)");
	if(!(fs->gd[grp].bg_free_blocks_count--))
		error_msg_and_die("group descr %d. free blocks count == 0 (corrupted fs?)",grp);
	if(!(fs->sb.s_free_blocks_count--))
		error_msg_and_die("superblock free blocks count == 0 (corrupted fs?)");
	return fs->sb.s_blocks_per_group*grp + bk;
}

// free a block
static void
free_blk(filesystem *fs, uint32 bk)
{
	uint32 grp;

	grp = bk / fs->sb.s_blocks_per_group;
	bk %= fs->sb.s_blocks_per_group;
	deallocate(get_blk(fs,fs->gd[grp].bg_block_bitmap), bk);
	fs->gd[grp].bg_free_blocks_count++;
	fs->sb.s_free_blocks_count++;
}

// allocate an inode
static uint32
alloc_nod(filesystem *fs)
{
	uint32 nod,best_group=0;
	uint32 grp,nbgroups,avefreei;

	nbgroups = GRP_NBGROUPS(fs);

	/* Distribute inodes amongst all the blocks                           */
	/* For every block group with more than average number of free inodes */
	/* find the one with the most free blocks and allocate node there     */
	/* Idea from find_group_dir in fs/ext2/ialloc.c in 2.4.19 kernel      */
	/* We do it for all inodes.                                           */
	avefreei  =  fs->sb.s_free_inodes_count / nbgroups;
	for(grp=0; grp<nbgroups; grp++) {
		if (fs->gd[grp].bg_free_inodes_count < avefreei ||
		    fs->gd[grp].bg_free_inodes_count == 0)
			continue;
		if (!best_group || 
			fs->gd[grp].bg_free_blocks_count > fs->gd[best_group].bg_free_blocks_count)
			best_group = grp;
	}
	if (!(nod = allocate(get_blk(fs,fs->gd[best_group].bg_inode_bitmap),0)))
		error_msg_and_die("couldn't allocate an inode (no free inode)");
	if(!(fs->gd[best_group].bg_free_inodes_count--))
		error_msg_and_die("group descr. free blocks count == 0 (corrupted fs?)");
	if(!(fs->sb.s_free_inodes_count--))
		error_msg_and_die("superblock free blocks count == 0 (corrupted fs?)");
	return fs->sb.s_inodes_per_group*best_group+nod;
}

// print a bitmap allocation
static void
print_bm(block b, uint32 max)
{
	uint32 i;
	printf("----+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0\n");
	for(i=1; i <= max; i++)
	{
		putchar(allocated(b, i) ? '*' : '.');
		if(!(i % 100))
			printf("\n");
	}
	if((i-1) % 100)
		printf("\n");
}

// initalize a blockwalker (iterator for blocks list)
static inline void
init_bw(blockwalker *bw)
{
	bw->bnum = 0;
	bw->bpdir = EXT2_INIT_BLOCK;
}

// return next block of inode (WALK_END for end)
// if *create>0, append a newly allocated block at the end
// if *create<0, free the block - warning, the metadata blocks contents is
//				  used after being freed, so once you start
//				  freeing blocks don't stop until the end of
//				  the file. moreover, i_blocks isn't updated.
//				  in fact, don't do that, just use extend_blk
// if hole!=0, create a hole in the file
static uint32
walk_bw(filesystem *fs, uint32 nod, blockwalker *bw, int32 *create, uint32 hole)
{
	uint32 *bkref = 0;
	uint32 *b;
	int extend = 0, reduce = 0;
	if(create && (*create) < 0)
		reduce = 1;
	if(bw->bnum >= get_nod(fs, nod)->i_blocks / INOBLK)
	{
		if(create && (*create) > 0)
		{
			(*create)--;
			extend = 1;
		}
		else	
			return WALK_END;
	}
	// first direct block
	if(bw->bpdir == EXT2_INIT_BLOCK)
	{
		bkref = &get_nod(fs, nod)->i_block[bw->bpdir = 0];
		if(extend) // allocate first block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}
	// direct block
	else if(bw->bpdir < EXT2_NDIR_BLOCKS)
	{
		bkref = &get_nod(fs, nod)->i_block[++bw->bpdir];
		if(extend) // allocate block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free block
			free_blk(fs, *bkref);
	}
	// first block in indirect block
	else if(bw->bpdir == EXT2_NDIR_BLOCKS)
	{
		bw->bnum++;
		bw->bpdir = EXT2_IND_BLOCK;
		bw->bpind = 0;
		if(extend) // allocate indirect block
			get_nod(fs, nod)->i_block[bw->bpdir] = alloc_blk(fs,nod);
		if(reduce) // free indirect block
			free_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		bkref = &b[bw->bpind];
		if(extend) // allocate first block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}
	// block in indirect block
	else if((bw->bpdir == EXT2_IND_BLOCK) && (bw->bpind < BLOCKSIZE/4 - 1))
	{
		bw->bpind++;
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		bkref = &b[bw->bpind];
		if(extend) // allocate block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free block
			free_blk(fs, *bkref);
	}
	// first block in first indirect block in first double indirect block
	else if(bw->bpdir == EXT2_IND_BLOCK)
	{
		bw->bnum += 2;
		bw->bpdir = EXT2_DIND_BLOCK;
		bw->bpind = 0;
		bw->bpdind = 0;
		if(extend) // allocate double indirect block
			get_nod(fs, nod)->i_block[bw->bpdir] = alloc_blk(fs,nod);
		if(reduce) // free double indirect block
			free_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		if(extend) // allocate first indirect block
			b[bw->bpind] = alloc_blk(fs,nod);
		if(reduce) // free  firstindirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		bkref = &b[bw->bpdind];
		if(extend) // allocate first block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}
	// block in indirect block in double indirect block
	else if((bw->bpdir == EXT2_DIND_BLOCK) && (bw->bpdind < BLOCKSIZE/4 - 1))
	{
		bw->bpdind++;
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		bkref = &b[bw->bpdind];
		if(extend) // allocate block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free block
			free_blk(fs, *bkref);
	}
	// first block in indirect block in double indirect block
	else if((bw->bpdir == EXT2_DIND_BLOCK) && (bw->bpind < BLOCKSIZE/4 - 1))
	{
		bw->bnum++;
		bw->bpdind = 0;
		bw->bpind++;
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		if(extend) // allocate indirect block
			b[bw->bpind] = alloc_blk(fs,nod);
		if(reduce) // free indirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		bkref = &b[bw->bpdind];
		if(extend) // allocate first block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}

	/* Adding support for triple indirection */
	/* Just starting triple indirection. Allocate the indirection
	   blocks and the first data block
	 */
	else if (bw->bpdir == EXT2_DIND_BLOCK) 
	{
	  	bw->bnum += 3;
		bw->bpdir = EXT2_TIND_BLOCK;
		bw->bpind = 0;
		bw->bpdind = 0;
		bw->bptind = 0;
		if(extend) // allocate triple indirect block
			get_nod(fs, nod)->i_block[bw->bpdir] = alloc_blk(fs,nod);
		if(reduce) // free triple indirect block
			free_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		if(extend) // allocate first double indirect block
			b[bw->bpind] = alloc_blk(fs,nod);
		if(reduce) // free first double indirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		if(extend) // allocate first indirect block
			b[bw->bpdind] = alloc_blk(fs,nod);
		if(reduce) // free first indirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpdind]);
		bkref = &b[bw->bptind];
		if(extend) // allocate first data block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}
	/* Still processing a single indirect block down the indirection
	   chain.Allocate a data block for it
	 */
	else if ( (bw->bpdir == EXT2_TIND_BLOCK) && 
		  (bw->bptind < BLOCKSIZE/4 -1) )
	{
		bw->bptind++;
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpdind]);
		bkref = &b[bw->bptind];
		if(extend) // allocate data block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free block
			free_blk(fs, *bkref);
	}
	/* Finished processing a single indirect block. But still in the 
	   same double indirect block. Allocate new single indirect block
	   for it and a data block
	 */
	else if ( (bw->bpdir == EXT2_TIND_BLOCK) &&
		  (bw->bpdind < BLOCKSIZE/4 -1) )
	{
		bw->bnum++;
		bw->bptind = 0;
		bw->bpdind++;
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		if(extend) // allocate single indirect block
			b[bw->bpdind] = alloc_blk(fs,nod);
		if(reduce) // free indirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpdind]);
		bkref = &b[bw->bptind];
		if(extend) // allocate first data block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}
	/* Finished processing a double indirect block. Allocate the next
	   double indirect block and the single,data blocks for it
	 */
	else if ( (bw->bpdir == EXT2_TIND_BLOCK) && 
		  (bw->bpind < BLOCKSIZE/4 - 1) )
	{
		bw->bnum += 2;
		bw->bpdind = 0;
		bw->bptind = 0;
		bw->bpind++;
		b = (uint32*)get_blk(fs, get_nod(fs, nod)->i_block[bw->bpdir]);
		if(extend) // allocate double indirect block
			b[bw->bpind] = alloc_blk(fs,nod);
		if(reduce) // free double indirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpind]);
		if(extend) // allocate single indirect block
			b[bw->bpdind] = alloc_blk(fs,nod);
		if(reduce) // free indirect block
			free_blk(fs, b[bw->bpind]);
		b = (uint32*)get_blk(fs, b[bw->bpdind]);
		bkref = &b[bw->bptind];
		if(extend) // allocate first block
			*bkref = hole ? 0 : alloc_blk(fs,nod);
		if(reduce) // free first block
			free_blk(fs, *bkref);
	}
	else
		error_msg_and_die("file too big !"); 
	/* End change for walking triple indirection */

	if(*bkref)
	{
		bw->bnum++;
		if(!reduce && !allocated(GRP_GET_BLOCK_BITMAP(fs,*bkref), GRP_BBM_OFFSET(fs,*bkref)))
			error_msg_and_die("[block %d of inode %d is unallocated !]", *bkref, nod);
	}
	if(extend)
		get_nod(fs, nod)->i_blocks = bw->bnum * INOBLK;
	return *bkref;
}

// add blocks to an inode (file/dir/etc...)
static void
extend_blk(filesystem *fs, uint32 nod, block b, int amount)
{
	int create = amount;
	blockwalker bw, lbw;
	uint32 bk;
	init_bw(&bw);
	if(amount < 0)
	{
		uint32 i;
		for(i = 0; i < get_nod(fs, nod)->i_blocks / INOBLK + amount; i++)
			walk_bw(fs, nod, &bw, 0, 0);
		while(walk_bw(fs, nod, &bw, &create, 0) != WALK_END)
			/*nop*/;
		get_nod(fs, nod)->i_blocks += amount * INOBLK;
	}
	else
	{
		lbw = bw;
		while((bk = walk_bw(fs, nod, &bw, 0, 0)) != WALK_END)
			lbw = bw;
		bw = lbw;
		while(create)
		{
			int i, copyb = 0;
			if(!(fs->sb.s_reserved[200] & OP_HOLES))
				copyb = 1;
			else
				for(i = 0; i < BLOCKSIZE / 4; i++)
					if(((int32*)(b + BLOCKSIZE * (amount - create)))[i])
					{
						copyb = 1;
						break;
					}
			if((bk = walk_bw(fs, nod, &bw, &create, !copyb)) == WALK_END)
				break;
			if(copyb)
				memcpy(get_blk(fs, bk), b + BLOCKSIZE * (amount - create - 1), BLOCKSIZE);
		}
	}
}

// link an entry (inode #) to a directory
static void
add2dir(filesystem *fs, uint32 dnod, uint32 nod, const char* name)
{
	blockwalker bw;
	uint32 bk;
	uint8 *b;
	directory *d;
	int reclen, nlen;
	inode *node;
	inode *pnode;

	pnode = get_nod(fs, dnod);
	if((pnode->i_mode & FM_IFMT) != FM_IFDIR)
		error_msg_and_die("can't add '%s' to a non-directory", name);
	if(!*name)
		error_msg_and_die("can't create an inode with an empty name");
	if(strchr(name, '/'))
		error_msg_and_die("bad name '%s' (contains a slash)", name);
	nlen = strlen(name);
	reclen = sizeof(directory) + rndup(nlen, 4);
	if(reclen > BLOCKSIZE)
		error_msg_and_die("bad name '%s' (too long)", name);
	init_bw(&bw);
	while((bk = walk_bw(fs, dnod, &bw, 0, 0)) != WALK_END) // for all blocks in dir
	{
		b = get_blk(fs, bk);
		// for all dir entries in block
		for(d = (directory*)b; (int8*)d + sizeof(*d) < (int8*)b + BLOCKSIZE; d = (directory*)((int8*)d + d->d_rec_len))
		{
			// if empty dir entry, large enough, use it
			if((!d->d_inode) && (d->d_rec_len >= reclen))
			{
				d->d_inode = nod;
				node = get_nod(fs, nod);
				node->i_links_count++;
				d->d_name_len = nlen;
				strncpy(d->d_name, name, nlen);
				return;
			}
			// if entry with enough room (last one?), shrink it & use it
			if(d->d_rec_len >= (sizeof(directory) + rndup(d->d_name_len, 4) + reclen))
			{
				reclen = d->d_rec_len;
				d->d_rec_len = sizeof(directory) + rndup(d->d_name_len, 4);
				reclen -= d->d_rec_len;
				d = (directory*) (((int8*)d) + d->d_rec_len);
				d->d_rec_len = reclen;
				d->d_inode = nod;
				node = get_nod(fs, nod);
				node->i_links_count++;
				d->d_name_len = nlen;
				strncpy(d->d_name, name, nlen);
				return;
			}
		}
	}
	// we found no free entry in the directory, so we add a block
	if(!(b = get_workblk()))
		error_msg_and_die("get_workblk() failed.");
	d = (directory*)b;
	d->d_inode = nod;
	node = get_nod(fs, nod);
	node->i_links_count++;
	d->d_rec_len = BLOCKSIZE;
	d->d_name_len = nlen;
	strncpy(d->d_name, name, nlen);
	extend_blk(fs, dnod, b, 1);
	get_nod(fs, dnod)->i_size += BLOCKSIZE;
	free_workblk(b);
}

// find an entry in a directory
static uint32
find_dir(filesystem *fs, uint32 nod, const char * name)
{
	blockwalker bw;
	uint32 bk;
	int nlen = strlen(name);
	init_bw(&bw);
	while((bk = walk_bw(fs, nod, &bw, 0, 0)) != WALK_END)
	{
		directory *d;
		uint8 *b;
		b = get_blk(fs, bk);
		for(d = (directory*)b; (int8*)d + sizeof(*d) < (int8*)b + BLOCKSIZE; d = (directory*)((int8*)d + d->d_rec_len))
			if(d->d_inode && (nlen == d->d_name_len) && !strncmp(d->d_name, name, nlen))
				return d->d_inode;
	}
	return 0;
}

// find the inode of a full path
static uint32
find_path(filesystem *fs, uint32 nod, const char * name)
{
	char *p, *n, *n2 = xstrdup(name);
	n = n2;
	while(*n == '/')
	{
		nod = EXT2_ROOT_INO;
		n++;
	}
	while(*n)
	{
		if((p = strchr(n, '/')))
			(*p) = 0;
		if(!(nod = find_dir(fs, nod, n)))
			break;
		if(p)
			n = p + 1;
		else
			break;
	}
	free(n2);
	return nod;
}

// chmod an inode
void
chmod_fs(filesystem *fs, uint32 nod, uint16 mode, uint16 uid, uint16 gid)
{
	inode *node;
	node = get_nod(fs, nod);
	node->i_mode = (node->i_mode & ~FM_IMASK) | (mode & FM_IMASK);
	node->i_uid = uid;
	node->i_gid = gid;
}

// create a simple inode
static uint32
mknod_fs(filesystem *fs, uint32 parent_nod, const char *name, uint16 mode, uint16 uid, uint16 gid, uint8 major, uint8 minor, uint32 ctime, uint32 mtime)
{
	uint32 nod;
	inode *node;
	{
		nod = alloc_nod(fs);
		node = get_nod(fs, nod);
		node->i_mode = mode;
		add2dir(fs, parent_nod, nod, name);
		switch(mode & FM_IFMT)
		{
			case FM_IFLNK:
				mode = FM_IFLNK | FM_IRWXU | FM_IRWXG | FM_IRWXO;
				break;
			case FM_IFBLK:
			case FM_IFCHR:
				((uint8*)get_nod(fs, nod)->i_block)[0] = minor;
				((uint8*)get_nod(fs, nod)->i_block)[1] = major;
				break;
			case FM_IFDIR:
				add2dir(fs, nod, nod, ".");
				add2dir(fs, nod, parent_nod, "..");
				fs->gd[GRP_GROUP_OF_INODE(fs,nod)].bg_used_dirs_count++;
				break;
		}
	}
	node->i_uid = uid;
	node->i_gid = gid;
	node->i_atime = mtime;
	node->i_ctime = ctime;
	node->i_mtime = mtime;
	return nod;
}

// make a full-fledged directory (i.e. with "." & "..")
static inline uint32
mkdir_fs(filesystem *fs, uint32 parent_nod, const char *name, uint32 mode,
	uid_t uid, gid_t gid, uint32 ctime, uint32 mtime)
{
	return mknod_fs(fs, parent_nod, name, mode|FM_IFDIR, uid, gid, 0, 0, ctime, mtime);
}

// make a symlink
static uint32
mklink_fs(filesystem *fs, uint32 parent_nod, const char *name, size_t size, uint8 *b, uid_t uid, gid_t gid, uint32 ctime, uint32 mtime)
{
	uint32 nod = mknod_fs(fs, parent_nod, name, FM_IFLNK | FM_IRWXU | FM_IRWXG | FM_IRWXO, uid, gid, 0, 0, ctime, mtime);
	extend_blk(fs, nod, 0, - (int)get_nod(fs, nod)->i_blocks / INOBLK);
	get_nod(fs, nod)->i_size = size;
	if(size <= 4 * (EXT2_TIND_BLOCK+1))
	{
		strncpy((char*)get_nod(fs, nod)->i_block, (char*)b, size);
		return nod;
	}
	extend_blk(fs, nod, b, rndup(size, BLOCKSIZE) / BLOCKSIZE);
	return nod;
}

// make a file from a FILE*
static uint32
mkfile_fs(filesystem *fs, uint32 parent_nod, const char *name, uint32 mode, size_t size, FILE *f, uid_t uid, gid_t gid, uint32 ctime, uint32 mtime)
{
	uint8 * b;
	uint32 nod = mknod_fs(fs, parent_nod, name, mode|FM_IFREG, uid, gid, 0, 0, ctime, mtime);
	extend_blk(fs, nod, 0, - (int)get_nod(fs, nod)->i_blocks / INOBLK);
	get_nod(fs, nod)->i_size = size;
	if (size) {
		if(!(b = (uint8*)calloc(rndup(size, BLOCKSIZE), 1)))
			error_msg_and_die("not enough mem to read file '%s'", name);
		if(f)
			fread(b, size, 1, f); // FIXME: ugly. use mmap() ...
		extend_blk(fs, nod, b, rndup(size, BLOCKSIZE) / BLOCKSIZE);
		free(b);
	}
	return nod;
}

// retrieves a mode info from a struct stat
static uint32
get_mode(struct stat *st)
{
	uint32 mode = 0;

	if(st->st_mode & S_IRUSR)
		mode |= FM_IRUSR;
	if(st->st_mode & S_IWUSR)
		mode |= FM_IWUSR;
	if(st->st_mode & S_IXUSR)
		mode |= FM_IXUSR;
	if(st->st_mode & S_IRGRP)
		mode |= FM_IRGRP;
	if(st->st_mode & S_IWGRP)
		mode |= FM_IWGRP;
	if(st->st_mode & S_IXGRP)
		mode |= FM_IXGRP;
	if(st->st_mode & S_IROTH)
		mode |= FM_IROTH;
	if(st->st_mode & S_IWOTH)
		mode |= FM_IWOTH;
	if(st->st_mode & S_IXOTH)
		mode |= FM_IXOTH;
	if(st->st_mode & S_ISUID)
		mode |= FM_ISUID;
	if(st->st_mode & S_ISGID)
		mode |= FM_ISGID;
	if(st->st_mode & S_ISVTX)
		mode |= FM_ISVTX;
	return mode;
}

// add or fixup entries to the filesystem from a text file

static void
add2fs_from_file(filesystem *fs, uint32 this_nod, FILE * fh, uint32 fs_timestamp, struct stats *stats)
{
	unsigned long mode, uid, gid, major, minor;
	unsigned long start, increment, count;
	uint32 nod, ctime, mtime;
	char *c, type, *path = NULL, *path2 = NULL, *dir, *name, *line = NULL;
	size_t len;
	struct stat st;
	int nbargs, lineno = 0;

	fstat(fileno(fh), &st);
	ctime = fs_timestamp;
	mtime = st.st_mtime;
	while(getline(&line, &len, fh) >= 0)
	{
		mode = uid = gid = major = minor = 0;
		start = 0; increment = 1; count = 0;
		lineno++;
		if((c = strchr(line, '#')))
			*c = 0;
		if (path) {
			free(path);
			path = NULL;
		}
		if (path2) {
			free(path2);
			path2 = NULL;
		}
		nbargs = sscanf (line, "%" SCANF_PREFIX "s %c %lo %lu %lu %lu %lu %lu %lu %lu",
					SCANF_STRING(path), &type, &mode, &uid, &gid, &major, &minor,
					&start, &increment, &count);
		if(nbargs < 3)
		{
			if(nbargs > 0)
				error_msg("device table line %d skipped: bad format for entry '%s'", lineno, path);
			continue;
		}
		mode &= FM_IMASK;
		path2 = strdup(path);
		name = basename(path);
		dir = dirname(path2);
		if((!strcmp(name, ".")) || (!strcmp(name, "..")))
		{
			error_msg("device table line %d skipped", lineno);
			continue;
		}
		if(fs)
		{
			if(!(nod = find_path(fs, this_nod, dir)))
			{
				error_msg("device table line %d skipped: can't find directory '%s' to create '%s''", lineno, dir, name);
				continue;
			}
		}
		else
			nod = 0;
		switch (type)
		{
			case 'd':
				mode |= FM_IFDIR;
				break;
			case 'f':
				mode |= FM_IFREG;
				break;
			case 'p':
				mode |= FM_IFIFO;
				break;
			case 's':
				mode |= FM_IFSOCK;
				break;
			case 'c':
				mode |= FM_IFCHR;
				break;
			case 'b':
				mode |= FM_IFBLK;
				break;
			default:
				error_msg("device table line %d skipped: bad type '%c' for entry '%s'", lineno, type, name);
				continue;
		}
		if(stats) {
			if(count > 0)
				stats->ninodes += count - start;
			else
				stats->ninodes++;
		} else {
			if(count > 0)
			{
				char *dname;
				unsigned long i;
				unsigned len;
				len = strlen(name) + 10;
				dname = malloc(len + 1);
				for(i = start; i < count; i++)
				{
					uint32 oldnod;
					SNPRINTF(dname, len, "%s%lu", name, i);
					oldnod = find_dir(fs, nod, dname);
					if(oldnod)
						chmod_fs(fs, oldnod, mode, uid, gid);
					else
						mknod_fs(fs, nod, dname, mode, uid, gid, major, minor + (i * increment - start), ctime, mtime);
				}
				free(dname);
			}
			else
			{
				uint32 oldnod = find_dir(fs, nod, name);
				if(oldnod)
					chmod_fs(fs, oldnod, mode, uid, gid);
				else
					mknod_fs(fs, nod, name, mode, uid, gid, major, minor, ctime, mtime);
			}
		}
	}
	if (line)
		free(line);
	if (path) 
		free(path);
	if (path2)
		free(path2);
}

static void
prep_stat(const char *root_path)
{
	int len = strlen(root_path);

	if((len >= 4) && (!strcmp(root_path + len - 4, "data"))) {
		source_path_len = len - 4;
	} else if((len >= 7) && (!strcmp(root_path + len - 6, "system"))) {
		source_path_len = len - 6;
	} else {
		error_msg_and_die("Fixstats (-a) option requested but "
				  "filesystem is not data or android!");
	}
}

static void
fix_stat(const char *path, struct stat *s)
{
	path += source_path_len;
	fs_config(path, S_ISDIR(s->st_mode), &s->st_uid, &s->st_gid, &s->st_mode);
}

// adds a tree of entries to the filesystem from current dir
static void
add2fs_from_dir(filesystem *fs, const char *path, uint32 this_nod, int squash_uids, int squash_perms, int fixstats, uint32 fs_timestamp, struct stats *stats)
{
	uint32 nod;
	uint32 uid, gid, mode, ctime, mtime;
	const char *name;
	FILE *fh;
	DIR *dh;
	struct dirent *dent;
	struct stat st;
	char *lnk;
	uint32 save_nod;
	char full_name[2048];

	if(!(dh = opendir(".")))
		perror_msg_and_die(".");
	while((dent = readdir(dh)))
	{
		if((!strcmp(dent->d_name, ".")) || (!strcmp(dent->d_name, "..")))
			continue;

		lstat(dent->d_name, &st);

		if(fixstats) {
			int tmp = snprintf(full_name, sizeof(full_name),
			                   "%s/%s", path, dent->d_name);
			if(tmp >= (int)sizeof(full_name))
				error_msg_and_die("Path too long!");
			fix_stat(full_name, &st);
		} else
			full_name[0] = '\0';
		uid = st.st_uid;
		gid = st.st_gid;
		ctime = fs_timestamp;
		mtime = st.st_mtime;
		name = dent->d_name;
		mode = get_mode(&st);
		if(squash_uids)
			uid = gid = 0;
		if(squash_perms)
			mode &= ~(FM_IRWXG | FM_IRWXO);
		if(stats)
			switch(st.st_mode & S_IFMT)
			{
				case S_IFLNK:
				case S_IFREG:
					if((st.st_mode & S_IFMT) == S_IFREG || st.st_size > 4 * (EXT2_TIND_BLOCK+1))
						stats->nblocks += (st.st_size + BLOCKSIZE - 1) / BLOCKSIZE;
				case S_IFCHR:
				case S_IFBLK:
				case S_IFIFO:
				case S_IFSOCK:
					stats->ninodes++;
					break;
				case S_IFDIR:
					stats->ninodes++;
					if(chdir(dent->d_name) < 0)
						perror_msg_and_die(dent->d_name);
					add2fs_from_dir(fs, full_name, this_nod, squash_uids, squash_perms, fixstats, fs_timestamp, stats);
					chdir("..");
					break;
				default:
					break;
			}
		else
		{
			if((nod = find_dir(fs, this_nod, name)))
			{
				error_msg("ignoring duplicate entry %s", name);
				if(S_ISDIR(st.st_mode)) {
					if(chdir(dent->d_name) < 0)
						perror_msg_and_die(name);
					add2fs_from_dir(fs, full_name, nod, squash_uids, squash_perms, fixstats, fs_timestamp, stats);
					chdir("..");
				}
				continue;
			}
			save_nod = 0;
			/* Check for hardlinks */
			if (!S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode) && st.st_nlink > 1) {
				int32 hdlink = is_hardlink(st.st_ino);
				if (hdlink >= 0) {
					add2dir(fs, this_nod, hdlinks.hdl[hdlink].dst_nod, name);
					continue;
				} else {
					save_nod = 1;
				}
			}
			switch(st.st_mode & S_IFMT)
			{
#if HAVE_STRUCT_STAT_ST_RDEV
				case S_IFCHR:
					nod = mknod_fs(fs, this_nod, name, mode|FM_IFCHR, uid, gid, major(st.st_rdev), minor(st.st_rdev), ctime, mtime);
					break;
				case S_IFBLK:
					nod = mknod_fs(fs, this_nod, name, mode|FM_IFBLK, uid, gid, major(st.st_rdev), minor(st.st_rdev), ctime, mtime);
					break;
#endif
				case S_IFIFO:
					nod = mknod_fs(fs, this_nod, name, mode|FM_IFIFO, uid, gid, 0, 0, ctime, mtime);
					break;
				case S_IFSOCK:
					nod = mknod_fs(fs, this_nod, name, mode|FM_IFSOCK, uid, gid, 0, 0, ctime, mtime);
					break;
				case S_IFLNK:
					lnk = xreadlink(dent->d_name);
					mklink_fs(fs, this_nod, name, st.st_size, (uint8*)lnk, uid, gid, ctime, mtime);
					free(lnk);
					break;
				case S_IFREG:
					fh = xfopen(dent->d_name, "rb");
					nod = mkfile_fs(fs, this_nod, name, mode, st.st_size, fh, uid, gid, ctime, mtime);
					fclose(fh);
					break;
				case S_IFDIR:
					nod = mkdir_fs(fs, this_nod, name, mode, uid, gid, ctime, mtime);
					if(chdir(dent->d_name) < 0)
						perror_msg_and_die(name);
					add2fs_from_dir(fs, full_name, nod, squash_uids, squash_perms, fixstats, fs_timestamp, stats);
					chdir("..");
					break;
				default:
					error_msg("ignoring entry %s", name);
			}
			if (save_nod) {
				if (hdlinks.count == hdlink_cnt) {
					if ((hdlinks.hdl = 
						 realloc (hdlinks.hdl, (hdlink_cnt + HDLINK_CNT) *
								  sizeof (struct hdlink_s))) == NULL) {
						error_msg_and_die("Not enough memory");
					}
					hdlink_cnt += HDLINK_CNT;
				}
				hdlinks.hdl[hdlinks.count].src_inode = st.st_ino;
				hdlinks.hdl[hdlinks.count].dst_nod = nod;
				hdlinks.count++;
			}
		}
	}
	closedir(dh);
}

// endianness swap of x-indirect blocks
static void
swap_goodblocks(filesystem *fs, inode *nod)
{
	uint32 i,j;
	int done=0;
	uint32 *b,*b2;

	uint32 nblk = nod->i_blocks / INOBLK;
	if((nod->i_size && !nblk) || ((nod->i_mode & FM_IFBLK) == FM_IFBLK) || ((nod->i_mode & FM_IFCHR) == FM_IFCHR))
		for(i = 0; i <= EXT2_TIND_BLOCK; i++)
			nod->i_block[i] = swab32(nod->i_block[i]);
	if(nblk <= EXT2_IND_BLOCK)
		return;
	swap_block(get_blk(fs, nod->i_block[EXT2_IND_BLOCK]));
	if(nblk <= EXT2_DIND_BLOCK + BLOCKSIZE/4)
		return;
	/* Currently this will fail b'cos the number of blocks as stored
	   in i_blocks also includes the indirection blocks (see
	   walk_bw). But this function assumes that i_blocks only
	   stores the count of data blocks ( Actually according to
	   "Understanding the Linux Kernel" (Table 17-3 p502 1st Ed)
	   i_blocks IS supposed to store the count of data blocks). so
	   with a file of size 268K nblk would be 269.The above check
	   will be false even though double indirection hasn't been
	   started.This is benign as 0 means block 0 which has been
	   zeroed out and therefore points back to itself from any offset
	 */
	// FIXME: I have fixed that, but I have the feeling the rest of
	// ths function needs to be fixed for the same reasons - Xav
	assert(nod->i_block[EXT2_DIND_BLOCK] != 0);
	for(i = 0; i < BLOCKSIZE/4; i++)
		if(nblk > EXT2_IND_BLOCK + BLOCKSIZE/4 + (BLOCKSIZE/4)*i )
			swap_block(get_blk(fs, ((uint32*)get_blk(fs, nod->i_block[EXT2_DIND_BLOCK]))[i]));
	swap_block(get_blk(fs, nod->i_block[EXT2_DIND_BLOCK]));
	if(nblk <= EXT2_IND_BLOCK + BLOCKSIZE/4 + BLOCKSIZE/4 * BLOCKSIZE/4)
		return;
	/* Adding support for triple indirection */
	b = (uint32*)get_blk(fs,nod->i_block[EXT2_TIND_BLOCK]);
	for(i=0;i < BLOCKSIZE/4 && !done ; i++) {
		b2 = (uint32*)get_blk(fs,b[i]); 
		for(j=0; j<BLOCKSIZE/4;j++) {
			if (nblk > ( EXT2_IND_BLOCK + BLOCKSIZE/4 + 
				     (BLOCKSIZE/4)*(BLOCKSIZE/4) + 
				     i*(BLOCKSIZE/4)*(BLOCKSIZE/4) + 
				     j*(BLOCKSIZE/4)) ) 
			  swap_block(get_blk(fs,b2[j]));
			else {
			  done = 1;
			  break;
			}
		}
		swap_block((uint8 *)b2);
	}
	swap_block((uint8 *)b);
	return;
}

static void
swap_badblocks(filesystem *fs, inode *nod)
{
	uint32 i,j;
	int done=0;
	uint32 *b,*b2;

	uint32 nblk = nod->i_blocks / INOBLK;
	if((nod->i_size && !nblk) || ((nod->i_mode & FM_IFBLK) == FM_IFBLK) || ((nod->i_mode & FM_IFCHR) == FM_IFCHR))
		for(i = 0; i <= EXT2_TIND_BLOCK; i++)
			nod->i_block[i] = swab32(nod->i_block[i]);
	if(nblk <= EXT2_IND_BLOCK)
		return;
	swap_block(get_blk(fs, nod->i_block[EXT2_IND_BLOCK]));
	if(nblk <= EXT2_DIND_BLOCK + BLOCKSIZE/4)
		return;
	/* See comment in swap_goodblocks */
	assert(nod->i_block[EXT2_DIND_BLOCK] != 0);
	swap_block(get_blk(fs, nod->i_block[EXT2_DIND_BLOCK]));
	for(i = 0; i < BLOCKSIZE/4; i++)
		if(nblk > EXT2_IND_BLOCK + BLOCKSIZE/4 + (BLOCKSIZE/4)*i )
			swap_block(get_blk(fs, ((uint32*)get_blk(fs, nod->i_block[EXT2_DIND_BLOCK]))[i]));
	if(nblk <= EXT2_IND_BLOCK + BLOCKSIZE/4 + BLOCKSIZE/4 * BLOCKSIZE/4)
		return;
	/* Adding support for triple indirection */
	b = (uint32*)get_blk(fs,nod->i_block[EXT2_TIND_BLOCK]);
	swap_block((uint8 *)b);
	for(i=0;i < BLOCKSIZE/4 && !done ; i++) {
		b2 = (uint32*)get_blk(fs,b[i]); 
		swap_block((uint8 *)b2);
		for(j=0; j<BLOCKSIZE/4;j++) {
			if (nblk > ( EXT2_IND_BLOCK + BLOCKSIZE/4 + 
				     (BLOCKSIZE/4)*(BLOCKSIZE/4) + 
				     i*(BLOCKSIZE/4)*(BLOCKSIZE/4) + 
				     j*(BLOCKSIZE/4)) ) 
			  swap_block(get_blk(fs,b2[j]));
			else {
			  done = 1;
			  break;
			}
		}
	}
	return;
}

// endianness swap of the whole filesystem
static void
swap_goodfs(filesystem *fs)
{
	uint32 i;
	for(i = 1; i < fs->sb.s_inodes_count; i++)
	{
		inode *nod = get_nod(fs, i);
		if(nod->i_mode & FM_IFDIR)
		{
			blockwalker bw;
			uint32 bk;
			init_bw(&bw);
			while((bk = walk_bw(fs, i, &bw, 0, 0)) != WALK_END)
			{
				directory *d;
				uint8 *b;
				b = get_blk(fs, bk);
				for(d = (directory*)b; (int8*)d + sizeof(*d) < (int8*)b + BLOCKSIZE; d = (directory*)((int8*)d + swab16(d->d_rec_len)))
					swap_dir(d);
			}
		}
		swap_goodblocks(fs, nod);
		swap_nod(nod);
	}
	for(i=0;i<GRP_NBGROUPS(fs);i++)
		swap_gd(&(fs->gd[i]));
	swap_sb(&fs->sb);
}

static void
swap_badfs(filesystem *fs)
{
	uint32 i;
	swap_sb(&fs->sb);
	for(i=0;i<GRP_NBGROUPS(fs);i++)
		swap_gd(&(fs->gd[i]));
	for(i = 1; i < fs->sb.s_inodes_count; i++)
	{
		inode *nod = get_nod(fs, i);
		swap_nod(nod);
		swap_badblocks(fs, nod);
		if(nod->i_mode & FM_IFDIR)
		{
			blockwalker bw;
			uint32 bk;
			init_bw(&bw);
			while((bk = walk_bw(fs, i, &bw, 0, 0)) != WALK_END)
			{
				directory *d;
				uint8 *b;
				b = get_blk(fs, bk);
				for(d = (directory*)b; (int8*)d + sizeof(*d) < (int8*)b + BLOCKSIZE; d = (directory*)((int8*)d + d->d_rec_len))
					swap_dir(d);
			}
		}
	}
}

// initialize an empty filesystem
static filesystem *
init_fs(int nbblocks, int nbinodes, int nbresrvd, int holes, uint32 fs_timestamp)
{
	uint32 i;
	filesystem *fs;
	directory *d;
	uint8 * b;
	uint32 nod, first_block;
	uint32 nbgroups,nbinodes_per_group,overhead_per_group,free_blocks,
		free_blocks_per_group,nbblocks_per_group,min_nbgroups;
	uint32 gdsz,itblsz,bbmpos,ibmpos,itblpos;
	uint32 j;
	uint8 *bbm,*ibm;
	inode *itab0;
	
	if(nbresrvd < 0)
		error_msg_and_die("reserved blocks value is invalid. Note: options have changed, see --help or the man page.");
	if(nbinodes < EXT2_FIRST_INO - 1 + (nbresrvd ? 1 : 0))
		error_msg_and_die("too few inodes. Note: options have changed, see --help or the man page.");
	if(nbblocks < 8)
		error_msg_and_die("too few blocks. Note: options have changed, see --help or the man page.");

	/* nbinodes is the total number of inodes in the system.
	 * a block group can have no more than 8192 inodes.
	 */
	min_nbgroups = (nbinodes + INODES_PER_GROUP - 1) / INODES_PER_GROUP;

	/* nbblocks is the total number of blocks in the filesystem.
	 * a block group can have no more than 8192 blocks.
	 */
	first_block = (BLOCKSIZE == 1024);
	nbgroups = (nbblocks - first_block + BLOCKS_PER_GROUP - 1) / BLOCKS_PER_GROUP;
	if(nbgroups < min_nbgroups) nbgroups = min_nbgroups;
	nbblocks_per_group = rndup((nbblocks - first_block + nbgroups - 1)/nbgroups, 8);
	nbinodes_per_group = rndup((nbinodes + nbgroups - 1)/nbgroups,
						(BLOCKSIZE/sizeof(inode)));
	if (nbinodes_per_group < 16)
		nbinodes_per_group = 16; //minimum number b'cos the first 10 are reserved

	gdsz = rndup(nbgroups*sizeof(groupdescriptor),BLOCKSIZE)/BLOCKSIZE;
	itblsz = nbinodes_per_group * sizeof(inode)/BLOCKSIZE;
	overhead_per_group = 3 /*sb,bbm,ibm*/ + gdsz + itblsz;
	if((uint32)nbblocks - 1 < overhead_per_group * nbgroups)
		error_msg_and_die("too much overhead, try fewer inodes or more blocks. Note: options have changed, see --help or the man page.");
	free_blocks = nbblocks - overhead_per_group*nbgroups - 1 /*boot block*/;
	free_blocks_per_group = nbblocks_per_group - overhead_per_group;

	if(!(fs = (filesystem*)calloc(nbblocks, BLOCKSIZE)))
		error_msg_and_die("not enough memory for filesystem");

	// create the superblock for an empty filesystem
	fs->sb.s_inodes_count = nbinodes_per_group * nbgroups;
	fs->sb.s_blocks_count = nbblocks;
	fs->sb.s_r_blocks_count = nbresrvd;
	fs->sb.s_free_blocks_count = free_blocks;
	fs->sb.s_free_inodes_count = fs->sb.s_inodes_count - EXT2_FIRST_INO + 1;
	fs->sb.s_first_data_block = first_block;
	fs->sb.s_log_block_size = BLOCKSIZE >> 11;
	fs->sb.s_log_frag_size = BLOCKSIZE >> 11;
	fs->sb.s_blocks_per_group = nbblocks_per_group;
	fs->sb.s_frags_per_group = nbblocks_per_group;
	fs->sb.s_inodes_per_group = nbinodes_per_group;
	fs->sb.s_wtime = fs_timestamp;
	fs->sb.s_magic = EXT2_MAGIC_NUMBER;
	fs->sb.s_lastcheck = fs_timestamp;

	// set up groupdescriptors
	for(i=0, bbmpos=gdsz+2, ibmpos=bbmpos+1, itblpos=ibmpos+1;
		i<nbgroups;
		i++, bbmpos+=nbblocks_per_group, ibmpos+=nbblocks_per_group, itblpos+=nbblocks_per_group)
	{
		if(free_blocks > free_blocks_per_group) {
			fs->gd[i].bg_free_blocks_count = free_blocks_per_group;
			free_blocks -= free_blocks_per_group;
		} else {
			fs->gd[i].bg_free_blocks_count = free_blocks;
			free_blocks = 0; // this is the last block group
		}
		if(i)
			fs->gd[i].bg_free_inodes_count = nbinodes_per_group;
		else
			fs->gd[i].bg_free_inodes_count = nbinodes_per_group -
							EXT2_FIRST_INO + 2;
		fs->gd[i].bg_used_dirs_count = 0;
		fs->gd[i].bg_block_bitmap = bbmpos;
		fs->gd[i].bg_inode_bitmap = ibmpos;
		fs->gd[i].bg_inode_table = itblpos;
	}

	/* Mark non-filesystem blocks and inodes as allocated */
	/* Mark system blocks and inodes as allocated         */
	for(i = 0; i<nbgroups;i++) {

		/* Block bitmap */
		bbm = get_blk(fs,fs->gd[i].bg_block_bitmap);	
		//non-filesystem blocks
		for(j = fs->gd[i].bg_free_blocks_count
		        + overhead_per_group + 1; j <= BLOCKSIZE * 8; j++)
			allocate(bbm, j); 
		//system blocks
		for(j = 1; j <= overhead_per_group; j++)
			allocate(bbm, j); 

		/* Inode bitmap */
		ibm = get_blk(fs,fs->gd[i].bg_inode_bitmap);	
		//non-filesystem inodes
		for(j = fs->sb.s_inodes_per_group+1; j <= BLOCKSIZE * 8; j++)
			allocate(ibm, j);

		//system inodes
		if(i == 0)
			for(j = 1; j < EXT2_FIRST_INO; j++)
				allocate(ibm, j);
	}

	// make root inode and directory
	/* We have groups now. Add the root filesystem in group 0 */
	/* Also increment the directory count for group 0 */
	fs->gd[0].bg_free_inodes_count--;
	fs->gd[0].bg_used_dirs_count = 1;
	itab0 = (inode *)get_blk(fs,fs->gd[0].bg_inode_table);
	itab0[EXT2_ROOT_INO-1].i_mode = FM_IFDIR | FM_IRWXU | FM_IRGRP | FM_IROTH | FM_IXGRP | FM_IXOTH; 
	itab0[EXT2_ROOT_INO-1].i_ctime = fs_timestamp;
	itab0[EXT2_ROOT_INO-1].i_mtime = fs_timestamp;
	itab0[EXT2_ROOT_INO-1].i_atime = fs_timestamp;
	itab0[EXT2_ROOT_INO-1].i_size = BLOCKSIZE;
	itab0[EXT2_ROOT_INO-1].i_links_count = 2;

	if(!(b = get_workblk()))
		error_msg_and_die("get_workblk() failed.");
	d = (directory*)b;
	d->d_inode = EXT2_ROOT_INO;
	d->d_rec_len = sizeof(directory)+4;
	d->d_name_len = 1;
	strcpy(d->d_name, ".");
	d = (directory*)(b + d->d_rec_len);
	d->d_inode = EXT2_ROOT_INO;
	d->d_rec_len = BLOCKSIZE - (sizeof(directory)+4);
	d->d_name_len = 2;
	strcpy(d->d_name, "..");
	extend_blk(fs, EXT2_ROOT_INO, b, 1);

	// make lost+found directory and reserve blocks
	if(fs->sb.s_r_blocks_count)
	{
		nod = mkdir_fs(fs, EXT2_ROOT_INO, "lost+found", FM_IRWXU, 0, 0, fs_timestamp, fs_timestamp);
		memset(b, 0, BLOCKSIZE);
		((directory*)b)->d_rec_len = BLOCKSIZE;
		/* We run into problems with e2fsck if directory lost+found grows
		 * bigger than this. Need to find out why this happens - sundar
		 */
		if (fs->sb.s_r_blocks_count > fs->sb.s_blocks_count * MAX_RESERVED_BLOCKS ) 
			fs->sb.s_r_blocks_count = fs->sb.s_blocks_count * MAX_RESERVED_BLOCKS;
		for(i = 1; i < fs->sb.s_r_blocks_count; i++)
			extend_blk(fs, nod, b, 1);
		get_nod(fs, nod)->i_size = fs->sb.s_r_blocks_count * BLOCKSIZE;
	}
	free_workblk(b);

	// administrative info
	fs->sb.s_state = 1;
	fs->sb.s_max_mnt_count = 20;

	// options for me
	if(holes)
		fs->sb.s_reserved[200] |= OP_HOLES;
	
	return fs;
}

// loads a filesystem from disk
static filesystem *
load_fs(FILE * fh, int swapit)
{
	size_t fssize;
	filesystem *fs;
	if((fseek(fh, 0, SEEK_END) < 0) || ((ssize_t)(fssize = ftell(fh)) == -1))
		perror_msg_and_die("input filesystem image");
	rewind(fh);
	fssize = (fssize + BLOCKSIZE - 1) / BLOCKSIZE;
	if(fssize < 16) // totally arbitrary
		error_msg_and_die("too small filesystem");
	if(!(fs = (filesystem*)calloc(fssize, BLOCKSIZE)))
		error_msg_and_die("not enough memory for filesystem");
	if(fread(fs, BLOCKSIZE, fssize, fh) != fssize)
		perror_msg_and_die("input filesystem image");
	if(swapit)
		swap_badfs(fs);
	if(fs->sb.s_rev_level || (fs->sb.s_magic != EXT2_MAGIC_NUMBER))
		error_msg_and_die("not a suitable ext2 filesystem");
	return fs;
}

static void
free_fs(filesystem *fs)
{
	free(fs);
}

// just walk through blocks list
static void
flist_blocks(filesystem *fs, uint32 nod, FILE *fh)
{
	blockwalker bw;
	uint32 bk;
	init_bw(&bw);
	while((bk = walk_bw(fs, nod, &bw, 0, 0)) != WALK_END)
		fprintf(fh, " %d", bk);
	fprintf(fh, "\n");
}

// walk through blocks list
static void
list_blocks(filesystem *fs, uint32 nod)
{
	int bn = 0;
	blockwalker bw;
	uint32 bk;
	init_bw(&bw);
	printf("blocks in inode %d:", nod);
	while((bk = walk_bw(fs, nod, &bw, 0, 0)) != WALK_END)
		printf(" %d", bk), bn++;
	printf("\n%d blocks (%d bytes)\n", bn, bn * BLOCKSIZE);
}

// saves blocks to FILE*
static void
write_blocks(filesystem *fs, uint32 nod, FILE* f)
{
	blockwalker bw;
	uint32 bk;
	int32 fsize = get_nod(fs, nod)->i_size;
	init_bw(&bw);
	while((bk = walk_bw(fs, nod, &bw, 0, 0)) != WALK_END)
	{
		if(fsize <= 0)
			error_msg_and_die("wrong size while saving inode %d", nod);
		if(fwrite(get_blk(fs, bk), (fsize > BLOCKSIZE) ? BLOCKSIZE : fsize, 1, f) != 1)
			error_msg_and_die("error while saving inode %d", nod);
		fsize -= BLOCKSIZE;
	}
}


// print block/char device minor and major
static void
print_dev(filesystem *fs, uint32 nod)
{
	int minor, major;
	minor = ((uint8*)get_nod(fs, nod)->i_block)[0];
	major = ((uint8*)get_nod(fs, nod)->i_block)[1];
	printf("major: %d, minor: %d\n", major, minor);
}

// print an inode as a directory
static void
print_dir(filesystem *fs, uint32 nod)
{
	blockwalker bw;
	uint32 bk;
	init_bw(&bw);
	printf("directory for inode %d:\n", nod);
	while((bk = walk_bw(fs, nod, &bw, 0, 0)) != WALK_END)
	{
		directory *d;
		uint8 *b;
		b = get_blk(fs, bk);
		for(d = (directory*)b; (int8*)d + sizeof(*d) < (int8*)b + BLOCKSIZE; d = (directory*)((int8*)d + d->d_rec_len))
			if(d->d_inode)
			{
				int i;
				printf("entry '");
				for(i = 0; i < d->d_name_len; i++)
					putchar(d->d_name[i]);
				printf("' (inode %d): rec_len: %d (name_len: %d)\n", d->d_inode, d->d_rec_len, d->d_name_len);
			}
	}
}

// print a symbolic link
static void
print_link(filesystem *fs, uint32 nod)
{
	if(!get_nod(fs, nod)->i_blocks)
		printf("links to '%s'\n", (char*)get_nod(fs, nod)->i_block);
	else
	{
		printf("links to '");
		write_blocks(fs, nod, stdout);
		printf("'\n");
	}
}

// make a ls-like printout of permissions
static void
make_perms(uint32 mode, char perms[11])
{
	strcpy(perms, "----------");
	if(mode & FM_IRUSR)
		perms[1] = 'r';
	if(mode & FM_IWUSR)
		perms[2] = 'w';
	if(mode & FM_IXUSR)
		perms[3] = 'x';
	if(mode & FM_IRGRP)
		perms[4] = 'r';
	if(mode & FM_IWGRP)
		perms[5] = 'w';
	if(mode & FM_IXGRP)
		perms[6] = 'x';
	if(mode & FM_IROTH)
		perms[7] = 'r';
	if(mode & FM_IWOTH)
		perms[8] = 'w';
	if(mode & FM_IXOTH)
		perms[9] = 'x';
	if(mode & FM_ISUID)
		perms[3] = 's';
	if(mode & FM_ISGID)
		perms[6] = 's';
	if(mode & FM_ISVTX)
		perms[9] = 't';
	switch(mode & FM_IFMT)
	{
		case 0:
			*perms = '0';
			break;
		case FM_IFSOCK:
			*perms = 's';
			break;
		case FM_IFLNK:
			*perms = 'l';
			break;
		case FM_IFREG:
			*perms = '-';
			break;
		case FM_IFBLK:
			*perms = 'b';
			break;
		case FM_IFDIR:
			*perms = 'd';
			break;
		case FM_IFCHR:
			*perms = 'c';
			break;
		case FM_IFIFO:
			*perms = 'p';
			break;
		default:
			*perms = '?';
	}
}

// print an inode
static void
print_inode(filesystem *fs, uint32 nod)
{
	char *s;
	char perms[11];
	if(!get_nod(fs, nod)->i_mode)
		return;
	switch(nod)
	{
		case EXT2_BAD_INO:
			s = "bad blocks";
			break;
		case EXT2_ROOT_INO:
			s = "root";
			break;
		case EXT2_ACL_IDX_INO:
		case EXT2_ACL_DATA_INO:
			s = "ACL";
			break;
		case EXT2_BOOT_LOADER_INO:
			s = "boot loader";
			break;
		case EXT2_UNDEL_DIR_INO:
			s = "undelete directory";
			break;
		default:
			s = (nod >= EXT2_FIRST_INO) ? "normal" : "unknown reserved"; 
	}
	printf("inode %d (%s, %d links): ", nod, s, get_nod(fs, nod)->i_links_count);
	if(!allocated(GRP_GET_INODE_BITMAP(fs,nod), GRP_IBM_OFFSET(fs,nod)))
	{
		printf("unallocated\n");
		return;
	}
	make_perms(get_nod(fs, nod)->i_mode, perms);
	printf("%s,  size: %d byte%s (%d block%s)\n", perms, plural(get_nod(fs, nod)->i_size), plural(get_nod(fs, nod)->i_blocks / INOBLK));
	switch(get_nod(fs, nod)->i_mode & FM_IFMT)
	{
		case FM_IFSOCK:
			list_blocks(fs, nod);
			break;
		case FM_IFLNK:
			print_link(fs, nod);
			break;
		case FM_IFREG:
			list_blocks(fs, nod);
			break;
		case FM_IFBLK:
			print_dev(fs, nod);
			break;
		case FM_IFDIR:
			list_blocks(fs, nod);
			print_dir(fs, nod);
			break;
		case FM_IFCHR:
			print_dev(fs, nod);
			break;
		case FM_IFIFO:
			list_blocks(fs, nod);
			break;
		default:
			list_blocks(fs, nod);
	}
	printf("Done with inode %d\n",nod);
}

// describes various fields in a filesystem
static void
print_fs(filesystem *fs)
{
	uint32 i;
	uint8 *ibm;

	printf("%d blocks (%d free, %d reserved), first data block: %d\n",
	       fs->sb.s_blocks_count, fs->sb.s_free_blocks_count,
	       fs->sb.s_r_blocks_count, fs->sb.s_first_data_block);
	printf("%d inodes (%d free)\n", fs->sb.s_inodes_count,
	       fs->sb.s_free_inodes_count);
	printf("block size = %d, frag size = %d\n",
	       fs->sb.s_log_block_size ? (fs->sb.s_log_block_size << 11) : 1024,
	       fs->sb.s_log_frag_size ? (fs->sb.s_log_frag_size << 11) : 1024);
	printf("number of groups: %d\n",GRP_NBGROUPS(fs));
	printf("%d blocks per group,%d frags per group,%d inodes per group\n",
	     fs->sb.s_blocks_per_group, fs->sb.s_frags_per_group,
	     fs->sb.s_inodes_per_group);
	printf("Size of inode table: %d blocks\n",
		(int)(fs->sb.s_inodes_per_group * sizeof(inode) / BLOCKSIZE));
	for (i = 0; i < GRP_NBGROUPS(fs); i++) {
		printf("Group No: %d\n", i+1);
		printf("block bitmap: block %d,inode bitmap: block %d, inode table: block %d\n",
		     fs->gd[i].bg_block_bitmap, fs->gd[i].bg_inode_bitmap,
		     fs->gd[i].bg_inode_table);
		printf("block bitmap allocation:\n");
		print_bm(GRP_GET_GROUP_BBM(fs, i),fs->sb.s_blocks_per_group);
		printf("inode bitmap allocation:\n");
		ibm = GRP_GET_GROUP_IBM(fs, i);
		print_bm(ibm, fs->sb.s_inodes_per_group);
		for (i = 1; i <= fs->sb.s_inodes_per_group; i++)
			if (allocated(ibm, i))
				print_inode(fs, i);
	}
}

static void
dump_fs(filesystem *fs, FILE * fh, int swapit)
{
	uint32 nbblocks = fs->sb.s_blocks_count;
	fs->sb.s_reserved[200] = 0;
	if(swapit)
		swap_goodfs(fs);
	if(fwrite(fs, BLOCKSIZE, nbblocks, fh) < nbblocks)
		perror_msg_and_die("output filesystem image");
	if(swapit)
		swap_badfs(fs);
}

static void
populate_fs(filesystem *fs, char **dopt, int didx, int squash_uids, int squash_perms, int fixstats, uint32 fs_timestamp, struct stats *stats)
{
	int i;
	for(i = 0; i < didx; i++)
	{
		struct stat st;
		FILE *fh;
		int pdir;
		char *pdest;
		uint32 nod = EXT2_ROOT_INO;
		if(fs)
			if((pdest = strchr(dopt[i], ':')))
			{
				*(pdest++) = 0;
				if(!(nod = find_path(fs, EXT2_ROOT_INO, pdest)))
					error_msg_and_die("path %s not found in filesystem", pdest);
			}
		stat(dopt[i], &st);
		switch(st.st_mode & S_IFMT)
		{
			case S_IFREG:
				fh = xfopen(dopt[i], "rb");
				add2fs_from_file(fs, nod, fh, fs_timestamp, stats);
				fclose(fh);
				break;
			case S_IFDIR:
				if((pdir = open(".", O_RDONLY)) < 0)
					perror_msg_and_die(".");
				if(chdir(dopt[i]) < 0)
					perror_msg_and_die(dopt[i]);
				if (fixstats)
					prep_stat(dopt[i]);
				add2fs_from_dir(fs, dopt[i], nod, squash_uids, squash_perms, fixstats, fs_timestamp, stats);
				if(fchdir(pdir) < 0)
					perror_msg_and_die("fchdir");
				if(close(pdir) < 0)
					perror_msg_and_die("close");
				break;
			default:
				error_msg_and_die("%s is neither a file nor a directory", dopt[i]);
		}
	}
}

static void
showversion(void)
{
	printf("genext2fs " VERSION "\n");
}

static void
showhelp(void)
{
	fprintf(stderr, "Usage: %s [options] image\n"
	"Create an ext2 filesystem image from directories/files\n\n"
	"  -x, --starting-image <image>\n"
	"  -d, --root <directory>\n"
	"  -D, --devtable <file>\n"
	"  -b, --size-in-blocks <blocks>\n"
	"  -i, --bytes-per-inode <bytes per inode>\n"
	"  -N, --number-of-inodes <number of inodes>\n"
	"  -m, --reserved-percentage <percentage of blocks to reserve>\n"
	"  -g, --block-map <path>     Generate a block map file for this path.\n"
	"  -e, --fill-value <value>   Fill unallocated blocks with value.\n"
	"  -z, --allow-holes          Allow files with holes.\n"
	"  -f, --faketime             Set filesystem timestamps to 0 (for testing).\n"
	"  -q, --squash               Same as \"-U -P\".\n"
	"  -U, --squash-uids          Squash owners making all files be owned by root.\n"
	"  -P, --squash-perms         Squash permissions on all files.\n"
	"  -a, --fix-android-stats    Fix-up file stats (user, perms, ...)\n"
	"  -h, --help\n"
	"  -V, --version\n"
	"  -v, --verbose\n\n"
	"Report bugs to genext2fs-devel@lists.sourceforge.net\n", app_name);
}

#define MAX_DOPT 128
#define MAX_GOPT 128

#define MAX_FILENAME 255

extern char* optarg;
extern int optind, opterr, optopt;

int
main(int argc, char **argv)
{
	int nbblocks = -1;
	int nbinodes = -1;
	int nbresrvd = -1;
	float bytes_per_inode = -1;
	float reserved_frac = -1;
	int fs_timestamp = -1;
	char * fsout = "-";
	char * fsin = 0;
	char * dopt[MAX_DOPT];
	int didx = 0;
	char * gopt[MAX_GOPT];
	int gidx = 0;
	int verbose = 0;
	int holes = 0;
	int emptyval = 0;
	int squash_uids = 0;
	int squash_perms = 0;
	int fix_android_stats = 0;
	uint16 endian = 1;
	int bigendian = !*(char*)&endian;
	filesystem *fs;
	int i;
	int c;
	struct stats stats;

#if HAVE_GETOPT_LONG
	struct option longopts[] = {
	  { "starting-image",	required_argument,	NULL, 'x' },
	  { "root",		required_argument,	NULL, 'd' },
	  { "devtable",		required_argument,	NULL, 'D' },
	  { "size-in-blocks",	required_argument,	NULL, 'b' },
	  { "bytes-per-inode",	required_argument,	NULL, 'i' },
	  { "number-of-inodes",	required_argument,	NULL, 'N' },
	  { "reserved-percentage", required_argument,	NULL, 'm' },
	  { "block-map",	required_argument,	NULL, 'g' },
	  { "fill-value",	required_argument,	NULL, 'e' },
	  { "allow-holes",	no_argument, 		NULL, 'z' },
	  { "faketime",		no_argument,		NULL, 'f' },
	  { "squash",		no_argument,		NULL, 'q' },
	  { "squash-uids",	no_argument,		NULL, 'U' },
	  { "squash-perms",	no_argument,		NULL, 'P' },
	  { "fix-android-stats",no_argument,		NULL, 'a' },
	  { "help",		no_argument,		NULL, 'h' },
	  { "version",		no_argument,		NULL, 'V' },
	  { "verbose",		no_argument,		NULL, 'v' },
	  { 0, 0, 0, 0}
	} ;

	app_name = argv[0];

	while((c = getopt_long(argc, argv, "x:d:D:b:i:N:m:g:e:zfqUPahVv", longopts, NULL)) != EOF) {
#else
	app_name = argv[0];

	while((c = getopt(argc, argv,      "x:d:D:b:i:N:m:g:e:zfqUPahVv")) != EOF) {
#endif /* HAVE_GETOPT_LONG */
		switch(c)
		{
			case 'x':
				fsin = optarg;
				break;
			case 'd':
			case 'D':
				dopt[didx++] = optarg;
				break;
			case 'b':
				nbblocks = SI_atof(optarg);
				break;
			case 'i':
				bytes_per_inode = SI_atof(optarg);
				break;
			case 'N':
				nbinodes = SI_atof(optarg);
				break;
			case 'm':
				reserved_frac = SI_atof(optarg) / 100;
				break;
			case 'g':
				gopt[gidx++] = optarg;
				break;
			case 'e':
				emptyval = atoi(optarg);
				break;
			case 'z':
				holes = 1;
				break;
			case 'f':
				fs_timestamp = 0;
				break;
			case 'q':
				squash_uids = 1;
				squash_perms = 1;
				break;
			case 'U':
				squash_uids = 1;
				break;
			case 'P':
				squash_perms = 1;
				break;
			case 'a':
				fix_android_stats = 1;
				break;
			case 'h':
				showhelp();
				exit(0);
			case 'V':
				showversion();
				exit(0);
			case 'v':
				verbose = 1;
				showversion();
				break;
			default:
				error_msg_and_die("Note: options have changed, see --help or the man page.");
		}
	}

	if(optind < (argc - 1))
		error_msg_and_die("Too many arguments. Try --help or else see the man page.");
	if(optind > (argc - 1))
		error_msg_and_die("Not enough arguments. Try --help or else see the man page.");

	if(fix_android_stats && (squash_uids || squash_perms))
		error_msg_and_die("Cannot squash uid/perms and fix them up for Android at the same time.");

	fsout = argv[optind];

	hdlinks.hdl = (struct hdlink_s *)malloc(hdlink_cnt * sizeof(struct hdlink_s));
	if (!hdlinks.hdl)
		error_msg_and_die("Not enough memory");
	hdlinks.count = 0 ;

	if(fsin)
	{
		if(strcmp(fsin, "-"))
		{
			FILE * fh = xfopen(fsin, "rb");
			fs = load_fs(fh, bigendian);
			fclose(fh);
		}
		else
			fs = load_fs(stdin, bigendian);
	}
	else
	{
		if(reserved_frac == -1)
			nbresrvd = nbblocks * RESERVED_BLOCKS;
		else 
			nbresrvd = nbblocks * reserved_frac;

		stats.ninodes = EXT2_FIRST_INO - 1 + (nbresrvd ? 1 : 0);
		stats.nblocks = 0;

		populate_fs(NULL, dopt, didx, squash_uids, squash_perms, 0, fs_timestamp, &stats);

		if(nbinodes == -1)
			nbinodes = stats.ninodes;
		else
			if(stats.ninodes > (unsigned long)nbinodes)
			{
				fprintf(stderr, "number of inodes too low, increasing to %ld\n", stats.ninodes);
				nbinodes = stats.ninodes;
			}

		if(bytes_per_inode != -1) {
			int tmp_nbinodes = nbblocks * BLOCKSIZE / bytes_per_inode;
			if(tmp_nbinodes > nbinodes)
				nbinodes = tmp_nbinodes;
		}
		if(fs_timestamp == -1)
			fs_timestamp = time(NULL);
		fs = init_fs(nbblocks, nbinodes, nbresrvd, holes, fs_timestamp);
	}
	
	populate_fs(fs, dopt, didx, squash_uids, squash_perms, fix_android_stats, fs_timestamp, NULL);

	if(emptyval) {
		uint32 b;
		for(b = 1; b < fs->sb.s_blocks_count; b++)
			if(!allocated(GRP_GET_BLOCK_BITMAP(fs,b),GRP_BBM_OFFSET(fs,b)))
				memset(get_blk(fs, b), emptyval, BLOCKSIZE);
	}
	if(verbose)
		print_fs(fs);
	for(i = 0; i < gidx; i++)
	{
		uint32 nod;
		char fname[MAX_FILENAME];
		char *p;
		FILE *fh;
		if(!(nod = find_path(fs, EXT2_ROOT_INO, gopt[i])))
			error_msg_and_die("path %s not found in filesystem", gopt[i]);
		while((p = strchr(gopt[i], '/')))
			*p = '_';
		SNPRINTF(fname, MAX_FILENAME-1, "%s.blk", gopt[i]);
		fh = xfopen(fname, "wb");
		fprintf(fh, "%d:", get_nod(fs, nod)->i_size);
		flist_blocks(fs, nod, fh);
		fclose(fh);
	}
	if(strcmp(fsout, "-"))
	{
		FILE * fh = xfopen(fsout, "wb");
		dump_fs(fs, fh, bigendian);
		fclose(fh);
	}
	else
		dump_fs(fs, stdout, bigendian);
	free_fs(fs);
	return 0;
}
