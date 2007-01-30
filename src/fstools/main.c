// Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

#include <stdlib.h>
#include <inttypes.h>
#include <getopt.h>
#include <ftw.h>

#define _LUCID_PRINTF_MACROS
#define _LUCID_SCANF_MACROS
#include <lucid/flist.h>
#include <lucid/log.h>
#include <lucid/printf.h>
#include <lucid/scanf.h>

#include "fstool.h"

extern const char *optstring;
extern const char *rcsid;

const fstool_args_t *fstool_args = NULL;
int   errcnt = 0;

FLIST32_START(iattr_list)
FLIST32_NODE(IATTR, TAG)
FLIST32_NODE(IATTR, ADMIN)
FLIST32_NODE(IATTR, WATCH)
FLIST32_NODE(IATTR, HIDE)
FLIST32_NODE(IATTR, BARRIER)
FLIST32_NODE(IATTR, IUNLINK)
FLIST32_NODE(IATTR, IMMUTABLE)
FLIST32_END

int main(int argc, char **argv)
{
	int i, c, flags = FTW_MOUNT|FTW_PHYS|FTW_CHDIR|FTW_ACTIONRETVAL;

	fstool_args_t args = {
		.recurse = 0,
		.dironly = 0,
		.xid     = ~(0U),
		.flags   = 0,
		.mask    = 0,
	};

	log_options_t log_options = {
		.ident  = argv[0],
		.stderr = true,
	};

	log_init(&log_options);
	atexit(log_close);

	fstool_args = &args;

	while (1) {
		c = getopt(argc, argv, optstring);

		if (c == -1)
			break;

		switch (c) {
			/* generic */
			case 'h': usage(EXIT_SUCCESS);
			case 'v': printf("%s\n", rcsid); exit(EXIT_SUCCESS); break;

			/* recurse options */
			case 'R': args.recurse = true; break;
			case 'c': flags       &= ~FTW_MOUNT; break;
			case 'd': args.dironly = true; break;

			/* xid/flag options */
			case 'x':
				sscanf(optarg, "%" SCNu32, &args.xid);

				if (args.xid == 1 || args.xid > 65535)
					log_error_and_die("invalid xid: %d", args.xid);

				break;

			case 'f':
				if (flist32_from_str(optarg, iattr_list, &args.flags,
							&args.mask, '~', ",") == -1)
					log_perror_and_die("flist32_from_str");

				break;

			default: usage(EXIT_FAILURE);
		}
	}

	if (optind == argc)
		nftw(".", handle_file, 20, flags);

	else for (i = optind; i < argc; i++)
		nftw(argv[i], handle_file, 20, flags);

	return errcnt > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
