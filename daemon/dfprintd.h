/*
 * dfprintd header file
 * Copyright (C) 2015 Iurii Shikin <shikin@reg.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _DFPRINTD_H
#define _DFPRINTD_H

#include <stdarg.h>
#include <sys/types.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

/* general */
#define NAME "dfprintd"
#define VERSION "0.0.1"

int daemonize(void);
//int already_running(void);

/* config */
#define CONFFILE "/etc/dfprintd.conf"
#define DEBUG 0
#define MAXPATHL 255
#define WORKERS 5
#define SOCKET "/var/run/dfprintd.socket"
#define PIDFILE "/var/run/dfprintd.pid"
#define MAXFILEL 255

struct config {
    char * socket;
    char * pidfile;
    char * config;
    int * workers;
    int * debug;
};
void read_config(struct config *, char *);

/* locks */
#define LOCKFILE "/var/run/dfprintd.lock"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

/* signals */
void sigterm(int);
void sighup(int);

/* errors */
#define MAXLINE 4096
static void err_doit(int, int, const char *, va_list);
void err_quit(const char *, ...);

/* other */
void show_usage(void);
void show_version(void);

#endif /* dfprintd.h */
