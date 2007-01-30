// Copyright 2006 Remo Lemma <coloss7@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <getopt.h>
#include <ftw.h>
#include <search.h>
#include <vserver.h>
#include <sys/stat.h>

#define _LUCID_PRINTF_MACROS
#define _LUCID_SCANF_MACROS
#include <lucid/log.h>
#include <lucid/printf.h>
#include <lucid/scanf.h>

static const char *rcsid = "$Id$";

static void *inotable = NULL;

static int errcnt = 0;

static xid_t xid = ~(0UL);

static int do_space = 0;
static int do_inode = 0;
static int do_hr = 0;

static uint64_t used_blocks = 0;
static uint64_t used_inodes = 0;

static
void usage(int rc)
{
	printf("Usage: vdu [-hvcsi] [-b <size>] -x <xid> <path>*\n"
	       "\n"
	       "Available options:\n"
	       "  -h         Display this help text\n"
	       "  -v         Display version information\n"
	       "  -c         Cross filesystem mounts\n"
	       "  -s         Calculate used space\n"
	       "  -i         Calculate used inodes\n"
	       "  -r         Calculate used space in  human readable format\n"
	       "  -b <size>  Blocksize (default: 1024)\n"
	       "  -x <xid>   Context ID\n");
	exit(rc);
}

static
char *pretty_mem(uint64_t mem)
{
	char *buf = NULL, prefix[] = "KMGTBEZY";

	int i, rest = 0;

	mem *= getpagesize() >> 10;

	for (i = 0; mem >= 1024; i++) {
		rest = ((mem % 1024) * 10) >> 10;
		mem = mem >> 10;
	}

	if (rest > 0)
		asprintf(&buf, "%d.%d%c", (int) mem, rest, prefix[i]);
	else if (mem > 0)
		asprintf(&buf, "%d.0%c", (int) mem, prefix[i]);
	else
		asprintf(&buf, "0K");

	return buf;
}

static
char *pretty_ino(uint64_t ino)
{
	char *buf = NULL, prefix[] = "KMGTBEZY";

	int i, rest = 0;

	for (i = 0; ino >= 1000; i++) {
		rest = ((ino % 1000) * 10) / 1000;
		ino = ino / 1000;
	}

	if (i == 0)
		asprintf(&buf, "%d", (int) ino);
	else if (rest > 0)
		asprintf(&buf, "%d.%d%c", (int) ino, rest, prefix[i - 1]);
	else if (ino > 0)
		asprintf(&buf, "%d.0%c", (int) ino, prefix[i - 1]);
	else
		asprintf(&buf, "0");

	return buf;
}

static
int inocmp(const void *a, const void *b)
{
	if (*(const ino_t *)a < *(const ino_t *)b) return -1;
	if (*(const ino_t *)a > *(const ino_t *)b) return  1;
	return 0;
}

static
void inofree(void *nodep)
{
	return;
}

static
int handle_file(const char *fpath, const struct stat *sb,
                int typeflag, struct FTW *ftwb)
{
	ix_attr_t attr;

	attr.filename = fpath + ftwb->base;

	if (ix_attr_get(&attr) == -1) {
		log_perror("ix_get_attr(%s)", fpath);
		errcnt++;
		return FTW_STOP;
	}

	if (!(attr.mask & IATTR_TAG))
		return FTW_CONTINUE;

	if (sb->st_nlink == 1 || tfind(&sb->st_ino, &inotable, inocmp) == NULL) {
		if (attr.xid == xid) {
			used_blocks += sb->st_blocks;
			used_inodes += 1;
		}

		if (tsearch(&sb->st_ino, &inotable, inocmp) == NULL) {
			log_perror("tsearch(%u)", sb->st_ino);
			errcnt++;
			return FTW_STOP;
		}
	}

	return FTW_CONTINUE;
}

static
void count_path(const char *path, int flags)
{
	int olderrcnt = errcnt;

	used_blocks = used_inodes = 0;

	if (inotable)
		tdestroy(inotable, inofree);

	inotable = NULL;

	struct stat sb;

	if (lstat(path, &sb) == -1) {
		log_perror("lstat(%s)", path);
		return;
	}

	nftw(path, handle_file, 20, flags);

	printf("%s", path);

	if (errcnt > olderrcnt)
		printf(" ERR");

	else if (do_hr) {
		if (do_space)
			printf(" %s", pretty_mem(used_blocks * sb.st_blksize));
		if (do_inode)
			printf(" %s", pretty_ino(used_inodes));
	}
	
	else {
		if (do_space)
			printf(" %" PRIu64, used_blocks * sb.st_blksize);
		if (do_inode)
			printf(" %" PRIu64, used_inodes);
	}

	printf("\n");
}

int main (int argc, char **argv)
{
	int i, c, flags = FTW_MOUNT|FTW_PHYS|FTW_CHDIR|FTW_ACTIONRETVAL;

	log_options_t log_options = {
		.ident  = argv[0],
		.stderr = true,
	};

	log_init(&log_options);
	atexit(log_close);

	while (1) {
		c = getopt(argc, argv, "+hvcsirx:");

		if (c == -1)
			break;

		switch (c) {
			case 'h': usage(EXIT_SUCCESS);
			case 'v': printf("%s\n", rcsid); exit(EXIT_SUCCESS); break;

			case 'c': flags &= ~FTW_MOUNT; break;

			case 's': do_space = 1; break;
			case 'i': do_inode = 1; break;
			case 'r': do_hr = 1; break;

			case 'x': sscanf(optarg, "%" SCNu32, &xid); break;

			default: usage(EXIT_FAILURE);
		}
	}

	if (xid == 1 || xid > 65535)
		log_error_and_die("invalid xid: %d", xid);

	if (!do_space && !do_inode)
		log_error_and_die("no action specified. use -s and/or -i");

	if (optind == argc)
		count_path(".", flags);

	else for (i = optind; i < argc; i++)
		count_path(argv[i], flags);

	return errcnt > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
