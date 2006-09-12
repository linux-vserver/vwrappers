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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include <getopt.h>
#include <ftw.h>
#include <search.h>
#include <vserver.h>

#include "../wrapper.h"

static const char *rcsid = "$Id";

static void *inotable = NULL;

static int errcnt = 0;

static xid_t xid = ~(0UL);

static bool do_space = false;
static bool do_inode = false;

static uint64_t used_blocks = 0;
static uint64_t used_inodes = 0;

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
	       "  -b <size>  Blocksize (default: 1024)\n"
	       "  -x <xid>   Context ID\n");
	exit(rc);
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
	struct vx_iattr iattr;
	
	iattr.filename = fpath + ftwb->base;
	
	if (vx_get_iattr(&iattr) == -1) {
		perr("vx_get_iattr(%s)", fpath);
		errcnt++;
	}
	
	if (!(iattr.mask & IATTR_TAG))
		return FTW_CONTINUE;
	
	if ((sb->st_nlink == 1 || tfind(&sb->st_ino, &inotable, inocmp) == NULL)) {
		if (iattr.xid == xid) {
			used_blocks += sb->st_blocks;
			used_inodes += 1;
		}
		
		if (tsearch(&sb->st_ino, &inotable, inocmp) == NULL) {
			perr("tsearch(%u)", sb->st_ino);
			errcnt++;
		}
	}
	
	return FTW_CONTINUE;
}

static
void count_path(const char *path, int flags, int bs)
{
	int olderrcnt = errcnt;
	
	used_blocks = used_inodes = 0;
	
	if (inotable)
		tdestroy(inotable, inofree);
	
	inotable = NULL;
	
	nftw(path, handle_file, 20, flags);
	
	printf("%s", path);
	
	if (errcnt > olderrcnt)
		printf(" ERR");
	
	else {
		if (do_space)
			printf(" %" PRIu64, used_blocks * 512 / bs);
		
		if (do_inode)
			printf(" %" PRIu64, used_inodes);
	}
	
	printf("\n");
}

int main (int argc, char **argv)
{
	int i, c, bs = 1024, flags = FTW_MOUNT|FTW_PHYS|FTW_CHDIR|FTW_ACTIONRETVAL;
	
	while (1) {
		c = getopt(argc, argv, "hvcsib:x:");
		
		if (c == -1)
			break;
		
		switch (c) {
			case 'h': usage(EXIT_SUCCESS);
			case 'v': printf("%s\n", rcsid); exit(EXIT_SUCCESS); break;
			
			case 'c': flags &= ~FTW_MOUNT; break;
			
			case 's': do_space = true; break;
			case 'i': do_inode = true; break;
			
			case 'b': bs = atoi(optarg); break;
			
			case 'x':
				xid = atoi(optarg);
				
				if (xid == 1 || xid > 65535)
					die("invalid xid: %d", xid);
				
				break;
			
			default: usage(EXIT_FAILURE);
		}
	}
	
	if (xid == 1 || xid > 65535)
		die("invalid xid: %d", xid);
	
	if (!do_space && !do_inode)
		die("no action specified. use -s and/or -i");
	
	if (optind == argc)
		count_path(".", flags, bs);
	
	else for (i = optind; i < argc; i++)
		count_path(argv[i], flags, bs);
	
	return errcnt > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
