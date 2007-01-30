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
#include <vserver.h>
#include <sys/resource.h>

#define _LUCID_PRINTF_MACROS
#include <lucid/log.h>
#include <lucid/printf.h>

static const char *rcsid = "$Id$";

static int nr_running = 0;
static int nr_cpus = 1;

static
char *pretty_time(uint64_t msec)
{
	char *buf = NULL;
	int d = 0, h = 0, m = 0, s = 0, ms = 0;

	ms = msec % 1000;
	msec /= 1000;
	s = msec % 60;
	msec /= 60;
	m = msec % 60;
	msec /= 60;
	h = msec % 24;
	msec /= 24;
	d = msec;

	if (d > 0)
		asprintf(&buf, "%4dd%02dh%02dm", d, h, m);
	else if (h > 0)
		asprintf(&buf, "%4dh%02dm%02ds", h, m, s);
	else
		asprintf(&buf, "%2dm%02ds%03dms", m, s, ms);

	return buf;
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
void show_vx(xid_t xid)
{
	vx_stat_t statb;
	vx_sched_info_t schedb;
	vx_limit_stat_t limnproc, limvm, limrss;
	vx_uname_t unameb;

	limnproc.id = RLIMIT_NPROC;
	limvm.id = RLIMIT_AS;
	limrss.id = RLIMIT_RSS;
	unameb.id = VHIN_CONTEXT;
	schedb.cpu_id = 0;

	if (vx_stat(xid, &statb) == -1)
		log_perror("vx_stat(%d)", xid);

	else if (vx_limit_stat(xid, &limnproc) == -1)
		log_perror("vx_limit_stat(NPROC, %d)", xid);

	else if (vx_limit_stat(xid, &limvm) == -1)
		log_perror("vx_limit_stat(VM, %d)", xid);

	else if (vx_limit_stat(xid, &limrss) == -1)
		log_perror("vx_limit_stat(RSS, %d)", xid);

	else if (vx_uname_get(xid, &unameb) == -1)
		log_perror("vx_uname_get(CONTEXT, %d)", xid);

	else if (vx_sched_info(xid, &schedb) == -1)
		log_perror("vx_sched_info(%d, 0)", xid);

	else printf("%-5u %-5u %5s %5s %3d %11s %11s %11s %s\n",
			xid, (int) limnproc.value,
			pretty_mem(limvm.value), pretty_mem(limrss.value), 0,
			pretty_time(schedb.user_msec), pretty_time(schedb.sys_msec),
			pretty_time(statb.uptime/1000000), unameb.value);

	if (nr_cpus < 2)
		return;

	int i;

	for (i = 1; i < nr_cpus; i++) {
		schedb.cpu_id = i;

		if (vx_sched_info(xid, &schedb) == -1)
			log_perror("vx_sched_info(%d, %d)", xid, i);

		else printf("%-5u %-5s %5s %5s %3d %11s %11s %11s %s\n",
				xid, "", "", "", i,
				pretty_time(schedb.user_msec), pretty_time(schedb.sys_msec),
				"", unameb.value);
	}
}

int main(int argc, char **argv)
{
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
		       "  CPU    - Processor ID\n"
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

	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	printf("%-5s %-5s %5s %5s %3s %11s %11s %11s %s\n",
			"XID", "NPROC", "VM", "RSS", "CPU", "UTIME", "STIME", "UPTIME", "NAME");

	int i;

	for (i = 2; i < 65535; i++) {
		if (vx_info(i, NULL) == -1)
			continue;

		nr_running++;
		show_vx(i);
	}

	if (nr_running < 1)
		printf("no running contexts found ...\n");

	return EXIT_SUCCESS;
}
