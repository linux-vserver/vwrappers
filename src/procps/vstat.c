// Copyright 2006 Benedikt Böhm <hollow@gentoo.org>
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
#include <ftw.h>
#include <vserver.h>
#include <sys/resource.h>

#define _LUCID_PRINTF_MACROS
#include <lucid/flist.h>
#include <lucid/log.h>
#include <lucid/misc.h>
#include <lucid/printf.h>
#include <lucid/str.h>

static const char *rcsid = "$Id$";

static int nr_running = 0;
static int nr_cpus = 1;

static
char *pretty_time(uint64_t msec)
{
	char *buf = NULL;
	int d = 0, h = 0, m = 0;

	msec /= 60000;
	m = msec % 60;
	msec /= 60;
	h = msec % 24;
	msec /= 24;
	d = msec;

	asprintf(&buf, "%4dd%02dh%02dm", d, h, m);

	return buf;
}

static
char *pretty_mem(uint64_t mem)
{
	char *buf = NULL, prefix[] = "KMGTBEZY";

	int i, rest = 0;

	mem *= getpagesize() >> 10;

	for (i = 0; mem >= 1024; i++) {
		rest = ((mem % 1024) * 1000) >> 10;
		mem = mem >> 10;
	}

	if (rest > 0)
		asprintf(&buf, "%d.%03d%c", (int) mem, rest, prefix[i]);
	else if (mem > 0)
		asprintf(&buf, "%d.0%c", (int) mem, prefix[i]);
	else
		buf = str_dup("0K");

	return buf;
}

static
int show_vx(xid_t xid)
{
	vx_stat_t statb;
	vx_sched_info_t schedb;
	vx_limit_stat_t limvm, limrss;
	vx_uname_t unameb;

	if (vx_stat(xid, &statb) == -1) {
		log_perror("vx_stat(%d)", xid);
		return FTW_STOP;
	}

	limvm.id = RLIMIT_AS;

	if (vx_limit_stat(xid, &limvm) == -1) {
		log_perror("vx_limit_stat(VM, %d)", xid);
		return FTW_STOP;
	}

	limrss.id = RLIMIT_RSS;

	if (vx_limit_stat(xid, &limrss) == -1) {
		log_perror("vx_limit_stat(RSS, %d)", xid);
		return FTW_STOP;
	}

	unameb.id = VHIN_CONTEXT;

	if (vx_uname_get(xid, &unameb) == -1) {
		log_perror("vx_uname_get(CONTEXT, %d)", xid);
		return FTW_STOP;
	}

	schedb.cpu_id = 0;

	if (vx_sched_info(xid, &schedb) == -1) {
		log_perror("vx_sched_info(%d)", xid);
		return FTW_STOP;
	}

	printf("%-5d %-5d %6d %6d %11s %11s %11s %s%s\n",
			xid, statb.tasks,
			pretty_mem(limvm.value), pretty_mem(limrss.value),
			pretty_time(schedb.user_msec), pretty_time(schedb.sys_msec),
			pretty_time(statb.uptime), unameb.value,
			nr_cpus > 1 ? "[0]" : "");

	if (nr_cpus < 2)
		return FTW_SKIP_SUBTREE;

	int i;

	for (i = 1; i < nr_cpus; i++) {
		schedb.cpu_id = i;

		if (vx_sched_info(xid, &schedb) == -1) {
			log_perror("vx_sched_info(%d)", xid);
			return FTW_STOP;
		}

		printf("%-5s %-5s %6s %6s %11s %11s %11s %s[%d]\n",
				xid, "", "", "",
				pretty_time(schedb.user_msec), pretty_time(schedb.sys_msec),
				"", unameb.value, i);
	}

	return FTW_SKIP_SUBTREE;
}

static
int handle_file(const char *fpath, const struct stat *sb,
		int tflag, struct FTW *ftwb)
{
	switch(tflag) {
	case FTW_D:
		if (!str_isdigit(fpath + ftwb->base))
			return FTW_CONTINUE;

		nr_running++;

		xid_t xid = atoi(fpath + ftwb->base);

		return show_vx(xid);

	case FTW_DNR:
	case FTW_NS:
		log_error("cannot stat: %s", fpath);
		return FTW_STOP;

	default:
		break;
	}

	return FTW_CONTINUE;
}

int main(int argc, char **argv)
{
	int flags = FTW_PHYS|FTW_ACTIONRETVAL;

	log_options_t log_options = {
		.ident  = argv[0],
		.stderr = true,
	};

	if (argc > 1) {
		printf("Usage: vstat takes no arguments\n"
		       "\n"
		       "The following information is shown:\n"
		       "  XID    - Context ID\n"
		       "  TASKS  - Number of processes\n"
		       "  VM     - Number of virtual memory pages\n"
		       "  RSS    - Number of memory pages locked into RAM\n"
		       "  UTIME  - User-mode time accumulated\n"
		       "  STIME  - Kernel-mode time accumulated\n"
		       "  UPTIME - Context lifetime\n"
		       "  NAME   - Context name\n"
		       "\n"
		       "%s\n", rcsid);
		exit(EXIT_FAILURE);
	}

	log_init(&log_options);
	atexit(log_close);

	if (!isdir("/proc/virtual"))
		log_perror_and_die("isdir(/proc/virtual)");

	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	printf("%-5s %-5s %6s %6s %11s %11s %11s %s\n",
			"XID", "TASKS", "VM", "RSS", "UTIME", "STIME", "UPTIME", "NAME");

	if (nftw("/proc/virtual", handle_file, 20, flags) == -1)
		log_perror_and_die("nftw(/proc/virtual)");

	if (nr_running < 1)
		printf("no running contexts found ...");

	return EXIT_SUCCESS;
}
