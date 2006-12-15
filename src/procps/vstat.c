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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <vserver.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <inttypes.h>
#include <time.h>
#include <getopt.h>

#include <lucid/argv.h>
#include <lucid/list.h>
#include <lucid/misc.h>
#include <lucid/open.h>
#include <lucid/str.h>

#define BUF 256
#define PROCDIR "/proc"
#define UPTIME_FILE "/proc/uptime"

#define min(x, y) (((x)<=(y))?(x):(y))

static const char *rcsid = "$Id$";

xid_t masterxid = 1, max_xid = 0;
static uint64_t pagesize, hz, uptime;

struct vs_proc_t{
	struct list_head list;
	char vname[65];
	xid_t xid;
	uint64_t procs;
	uint64_t vm;
	uint64_t rss;
	uint64_t uptime;
	uint64_t utime;
	uint64_t ctime;
};

void usage (int rc)
{
	fprintf(stdout,
		"Usage: vserver-stat takes no arguments\n\n"
		"Information about vserver-stat output:\n"
		"XID:   Context ID\n"
		"PROCS: Number of running processes\n"
		"VS:    Number of Virtual memory pages\n"
		"RSS:   Resident Set Size\n"
		"UTIME  User-mode time accumulated\n"
		"CTIME: Kernel-mode time accumulated\n"
		"NAME:  Vserver Name\n");
	exit(rc);
}

uint64_t mst(uint64_t value)
{
	return (value*1000llu/hz);
}

char *convt(uint64_t value) 
{
	uint64_t h = 0, m = 0, s = 0, ms = 0, d  = 0, y = 0;
	struct tm *tt;
	time_t rtime;
	char year[32], *ttime;

	time (&rtime);
	tt = localtime(&rtime);
	strftime(year, sizeof(year) - 1, "%Y", tt);

	ms = value %1000;
	value /= 1000;
	s = value %60;
	value /= 60;
	m = value %60;
	value /= 60;
	h = value %24;
	value /= 24;	


	/* Max value -> 999 years */
	if (value > 999*365) {
		asprintf(&ttime, "undef");
		return (strdup(ttime));
	}

	while (value > 999) {
		if (atoi(year) %4 == 0) {
			y = value %366;
			if (y > 0)
				value /= 366;
				if (y >= 4)
					value += ((y %4) - 1);
		}
		else {
			y = value %365; 
			if (y > 0)
				value /= 365;
				if (y >= 4)
					value += (y %4);
		}
	}

	d = value;

	if (y > 0) {
		asprintf(&ttime, "%" PRIu64 "y%" PRIu64 "d%" PRIu64, y, d, h);
		return (strdup(ttime));
	}
	else if (d > 0) {
		asprintf(&ttime, "%" PRIu64 "d%" PRIu64 "h%" PRIu64, d, h, m);
		return (strdup(ttime));
	}
	else if (h > 0) {
		asprintf(&ttime, "%" PRIu64 "h%" PRIu64 "m%" PRIu64, h, m, s);
		return (strdup(ttime));
	}
	else if (m > 0) {
		asprintf(&ttime, "%" PRIu64 "m%" PRIu64 "s%" PRIu64, m, s, ms);
		return (strdup(ttime));
	}
	else if (s > 0) {
		asprintf(&ttime, "%" PRIu64 "s%" PRIu64, s, ms);
		return (strdup(ttime));
	}
	else {
		asprintf(&ttime, "%" PRIu64 "ms", ms);
		return (strdup(ttime));
	}
}

char  *tail_memdata(uint64_t value) {
	char *SUFF[] = { " ", "K", "M", "G" };
	char *buf;
	int i;
	float vl;

	vl = (float) value;

	for (i=0; i < 4; i++) {
		if ((int) (vl / 1024) == 0)
			break;
		vl /= 1024;
	}
	/* Max value -> 999.99 */
	if (vl > 999.99)
		asprintf(&buf, "undef");
	else
		asprintf(&buf, "%.2f%s", vl, SUFF[i]);

	return (strdup(buf));
}

uint64_t get_uptime() {
	int fd;
	uint64_t vl;
	char buffer[BUF*4], pt[BUF*4];

	if ((fd = open_read(UPTIME_FILE)) == -1) {
		fprintf(stderr, "open(%s): %s\n", UPTIME_FILE, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (read(fd, buffer, sizeof(buffer)) == -1) {
		fprintf(stderr, "read(%s): %s\n", UPTIME_FILE, strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(fd);

	strncat(pt, buffer, strchr(buffer, ' ')-buffer+1);

	vl = 1000 * atof(pt);
	return (vl);
}

char *tail_name (char *vname) 
{	
	char *pch;
	pch = strrchr(vname, '/') + 1;

	return strdup(pch);
}

char *get_vname (xid_t xid, vx_uname_t uname)
{
	uname.id = VHIN_CONTEXT;
	
	if (vx_uname_get(xid, &uname) == -1)
		return NULL;
	return strdup(uname.value);
}

int main(int argc, char *argv[])
{
	DIR *dirt;
	struct dirent *ditp;
	xid_t xid, lxid;
	pid_t pid;
	struct vs_proc_t *tmp, vsp;
	struct list_head *pos;
	vx_uname_t v_name;
	int lock = 0, fd;
	char c, *file, buffer[BUF], *vargv[BUF], name[65];
	uint64_t stime = 0;

	while ((c = getopt(argc, argv, "hv")) != -1) {
		switch (c) {
			case 'h': usage(EXIT_SUCCESS);
			case 'v': printf("%s\n", rcsid); exit(EXIT_SUCCESS); break;
			default: usage(EXIT_FAILURE);
		}
	}

	/* Init Values */ 
	pagesize = sysconf(_SC_PAGESIZE);
	hz = sysconf(_SC_CLK_TCK);
	uptime = get_uptime();

	INIT_LIST_HEAD(&vsp.list);

	/* Migrate to root server */
	if (vx_migrate(masterxid, NULL) == -1) {
		fprintf(stderr, "cannot migrate to root server, xid = 1\n");
		exit(EXIT_FAILURE);
	}

	if ((dirt = opendir(PROCDIR)) == NULL) {
		fprintf(stderr, "opendir(%s): %s\n", PROCDIR, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Parse '/proc' */
	while ((ditp = readdir(dirt)) != NULL) {
		lock=0;
		pid = atoi(ditp->d_name);
		if (!isdigit(*ditp->d_name))
			continue;
		xid = vx_task_xid(pid);
		if (xid > 1 || xid == 0) {
			tmp= (struct vs_proc_t *) malloc(sizeof(struct vs_proc_t));

			snprintf(buffer, sizeof(buffer) - 1, "%s/%d", PROCDIR, pid);
			file = str_path_concat(buffer, "stat");

			if ((fd = open_read(file)) == -1) {
				fprintf(stderr, "PID '%d', xid '%d' - open(%s): %s\n", pid, xid, file, strerror(errno));
				exit(EXIT_FAILURE);
			}
			if ((read(fd, buffer, sizeof(buffer))) == -1) {
				fprintf(stderr, "PID '%d', xid '%d' - read(%s): %s\n", pid, xid, file, strerror(errno));
				exit(EXIT_FAILURE);
			}
			close(fd);

			argv_from_str(buffer, vargv, BUF);
			
			list_for_each(pos, &vsp.list) {
				tmp=list_entry(pos, struct vs_proc_t, list);
				/* Update existing xid */
				if (tmp->xid == xid) {
					lock = 1;
					stime = mst(strtol(vargv[21], &vargv[21], 10));
					tmp->procs++;
					tmp->vm += strtol(vargv[22], &vargv[22], 10);
					tmp->rss += strtol(vargv[23], &vargv[23], 10);
					tmp->utime += ( mst(strtol(vargv[13], &vargv[13], 10)) + mst(strtol(vargv[15], &vargv[15], 10)) );
					tmp->ctime += ( mst(strtol(vargv[14], &vargv[14], 10)) + mst(strtol(vargv[16], &vargv[16], 10)) );
					tmp->uptime = min(tmp->uptime, stime); 
				}
			}
			/* New xid */
			if (lock != 1) {
				tmp= (struct vs_proc_t *) malloc(sizeof(struct vs_proc_t));
				if (xid != 0) {
					if (get_vname(xid, v_name) == NULL) {
						fprintf(stderr, "cannot retrive vserver name for xid '%d'\n", xid);
						exit(EXIT_FAILURE);
					}
					snprintf(name, sizeof(name) - 1, get_vname(xid, v_name));
						if (name[0] == '/')
							snprintf(tmp->vname, sizeof(tmp->vname) - 1, tail_name(name));
						else
							snprintf(tmp->vname, sizeof(tmp->vname) - 1, name);
				}
				else
					snprintf(tmp->vname, sizeof(tmp->vname) - 1, "root server");
				tmp->xid = xid;
				tmp->utime = ( mst(strtol(vargv[13], &vargv[13], 10)) + mst(strtol(vargv[15], &vargv[15], 10)) );
				tmp->ctime = ( mst(strtol(vargv[14], &vargv[14], 10)) + mst(strtol(vargv[16], &vargv[16], 10)) );
				tmp->vm = strtol(vargv[22], &vargv[22], 10);
				tmp->rss = strtol(vargv[23], &vargv[23], 10);
				tmp->procs = 1;
				tmp->uptime = mst(strtol(vargv[21], &vargv[21], 10));
				list_add(&(tmp->list), &(vsp.list));
				
				if (xid > max_xid) 
					max_xid = xid;
			}
		}
	}
	closedir(dirt);

	fprintf(stdout, "XID   PROCS   VM      RRS     UPTIME     UTIME      CTIME      NAME\n");
	/* Ording list... Any other solution? */
	for (lxid=0;lxid<=max_xid;lxid++) {
		tmp = (struct vs_proc_t *) malloc(sizeof(struct vs_proc_t));

		list_for_each(pos, &vsp.list) {
			tmp=list_entry(pos, struct vs_proc_t, list);		
			if (tmp->xid == lxid) {
				fprintf(stdout, "%-6d%-8" PRIu64 "%-8s%-8s%-11s%-11s%-11s%-14s\n", 
						tmp->xid, tmp->procs, tail_memdata(tmp->vm), 
						tail_memdata(tmp->rss*pagesize), 
						convt(uptime - tmp->uptime), convt(tmp->utime), convt(tmp->ctime), tmp->vname);
			}
		}
	}
	exit(EXIT_SUCCESS);
}
