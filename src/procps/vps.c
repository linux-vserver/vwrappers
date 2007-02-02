// Copyright 2006      Remo Lemma <coloss7@gmail.com>
//           2006-2007 Benedikt Böhm <hollow@gentoo.org>
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

#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <getopt.h>
#include <vserver.h>
#include <sys/wait.h>

#include <lucid/log.h>
#include <lucid/open.h>
#include <lucid/printf.h>
#include <lucid/scanf.h>
#include <lucid/str.h>

static const char *rcsid = "$Id$";

static int error_mode = 0;

static
void parse_line(char *line, int n)
{
	char *pid_pos, *name = NULL;
	static int pid_start;
	pid_t pid = -1;
	xid_t xid = -1;
	vx_uname_t uname;

	if (n == 0) {
		if ((pid_pos = str_str(line, "  PID")) == 0) {
			log_error("PID column not found, dumping ps output as-is");
			printf("%s\n", line);
			error_mode = 1;
			return;
		}

		pid_start = pid_pos - line;

		printf("  XID NAME     %s\n", line);
		return;
	}

	else {
		sscanf(line + pid_start, "%d", &pid);

		if (pid < 0) {
			log_error("invalid pid %d on line %d", pid, n);
			return;
		}

		if ((xid = vx_task_xid(pid)) == -1) {
			log_perror("could not get xid for pid %d", pid);
			xid  = -1;
			name = "ERR";
		}

		else if (xid == 0)
			name = "ADMIN";

		else if (xid == 1)
			name = "WATCH";

		else {
			uname.id = VHIN_CONTEXT;

			if (vx_uname_get(xid, &uname) == -1) {
				log_perror("could not get name for xid %d", xid);
				name = "ERR";
			}

			else {
				char *p = str_chr(uname.value, ':', str_len(uname.value));

				if (p)
					*p = '\0';

				name = uname.value;
			}
		}

		printf("%5d %-8.8s %s\n", xid, name, line);
	}
}

static
void pipe_ps(int argc, char **argv)
{
	int p[2], fd, i, status, len;
	pid_t pid;
	char *line;

	pipe(p);

	switch ((pid = fork())) {
	case -1:
		log_perror_and_die("fork");

	case 0:
		fd = open_read("/dev/null");

		dup2(fd,   0);
		dup2(p[1], 1);

		close(p[0]);
		close(p[1]);
		close(fd);

		if (vx_migrate(1, NULL) == -1)
			log_perror_and_die("vx_migrate");

		if (execvp(argv[0], argv) == -1)
			log_perror_and_die("execvp");

	default:
		close(p[1]);

		for (i = 0; ; i++) {
			if ((len = str_readline(p[0], &line)) == -1)
				log_perror_and_die("str_readline");

			if (!len)
				break;

			if (error_mode)
				printf("%s\n", line);
			else
				parse_line(line, i);
		}

		close(p[0]);

		if (waitpid(pid, &status, 0) == -1)
			log_perror_and_die("waitpid");
	}

	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	int c;
	xid_t xid = 1;

	log_options_t log_options = {
		.ident  = argv[0],
		.stderr = true,
	};

	log_init(&log_options);
	atexit(log_close);

	while (1) {
		c = getopt(argc, argv, "+hvx:");

		if (c == -1)
			break;

		switch (c) {
			case 'h':
				printf("Usage: %s [-x <xid>] [-- <args>]\n", argv[0]);
				exit(EXIT_SUCCESS);

			case 'v':
				printf("%s\n", rcsid); exit(EXIT_SUCCESS);
				break;

			case 'x':
				sscanf(optarg, "%" SCNu32, &xid);
				break;

			default:
				printf("Usage: %s [-x <xid>] [-- <args>]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	argv[--optind] = "/bin/ps";

	if (xid == 1)
		pipe_ps(argc - optind, argv + optind);

	else if (xid == 0 && execvp(argv[optind], argv+optind) == -1)
		log_perror_and_die("execvp");

	else if (xid > 1 && xid < 65536) {
		if (vx_migrate(xid, NULL) == -1)
			log_perror_and_die("vx_migrate");

		if (execvp(argv[optind], argv+optind) == -1)
			log_perror_and_die("execvp");
	}

	else
		log_error_and_die("invalid xid: %d", xid);

	return EXIT_SUCCESS;
}
