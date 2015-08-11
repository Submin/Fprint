/*
 * dfprintd Dumb frintd daemon
 *
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

#include "dfprintd.h"

#include <getopt.h>
#include <talloc.h>

int
main(int argc, char **argv)
{
    struct sigaction sa;
    int opt, daemon;
    int debug, workers;

    struct config *conf = talloc(NULL, struct config);

    /* Init default configuration */
    conf->socket = SOCKET;
    conf->pidfile = PIDFILE;
    conf->config = CONFFILE;

    debug = DEBUG;
    workers = WORKERS;
    conf->debug = &debug;
    conf->workers = &workers;
printf("\nAfter load conf:\n\tsocket: %s\n\tpidfile: %s\n\tconfig: %s\n\tworkers: %d\n\tdebug: %d\n",
    conf->socket,
    conf->pidfile,
    conf->config,
    *conf->workers,
    *conf->debug
);
    while(1){
        int opt_index;
        static struct option long_opt[] = {
            {"config", 1, 0, 'c'},
            {"debug", 0, 0, 'd'},
            {"help", 0, 0, 'h'},
            {"version", 0, 0, 'v'},
            {0,0,0,0}
        };

        if((opt = getopt_long(argc, argv, "c:dhv", long_opt, &opt_index)) == -1)
            break;

        switch(opt){
            case 'c':
                read_config(conf, optarg);
                break;
            case 'd':
                exit(0);
            case 'h':
                show_usage();
                exit(0);
            case 'v':
                show_version();
                exit(0);
            default:
                show_usage();
                exit(-1);
        }
    }

    /* Убедиться в том, что ранее не была запущена другая копия демона 
    if (already_running()) {
        syslog(LOG_ERR, "daemon already is running");
        exit(1);
    }
*/
    /* Перейти в режим демона 
    daemon = daemonize();
    if (daemon < 0)
        exit(-1);
    else if (daemon == 0)
        exit(0);
*/

    /* Установить обработчики сигналов */
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGHUP);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        syslog(LOG_ERR, "невозможно перехватить сигнал SIGTERM: %s", strerror(errno));
        exit(1);
    }

    sa.sa_handler = sighup;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        syslog(LOG_ERR, "невозможно перехватить сигнал SIGHUP: %s", strerror(errno));
        exit(1);
    }

//    for(;;);

    exit(0);
}


/*
printf("\nAfter load conf:\n\tsocket: %s\n\tpidfile: %s\n\tconfig: %s\n\tworkers: %d\n\tdebug: %d\n",
    conf->socket,
    conf->pidfile,
    conf->config,
    conf->workers,
    conf->debug
);

*/