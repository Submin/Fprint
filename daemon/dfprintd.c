/*
 * Dumb frintd daemon
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

#include <yaml.h>

void sigterm(int signo)
{
    syslog(LOG_INFO, "daemon terminating");
    exit(0);
}

void sighup(int signo)
{
//    if(read_config())
        syslog(LOG_INFO, "config successfully reloaded");
}

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[MAXLINE];

    vsnprintf(buf, MAXLINE, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s", strerror(error));
    strcat(buf, "\n");
    fflush(stdout); /* в случае, когда stdout и stderr – */
    /* одно и то же устройство */
    fputs(buf, stderr);
    fflush(NULL); /* сбрасывает все выходные потоки */
}

void
err_quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

int
daemonize(void)
{
    int fd, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /* Сбросить маску режима создания файла */
    umask(0);

    /* Стать лидером новой сессии, чтобы утратить управляющий терминал */
    if ((pid = fork()) < 0)
        /* здесь отладочный вывод */
        return(-1);
    else if (pid != 0) /* родительский процесс */
        /* здесь отладочный вывод */
        return(0);

    setsid();

    /* Обеспечить невозможность обретения управляющего терминала в будущем */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        /* здесь отладочный вывод */
        return(-1);

    if ((pid = fork()) < 0)
        /* здесь отладочный вывод */
        return(-1);
    else if (pid != 0) /* родительский процесс */
        /* здесь отладочный вывод */
        return(0);

    /* Закрыть все открытые файловые дескрипторы */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        /* здесь отладочный вывод */
        return(-1);

    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for (fd = 0; fd < rl.rlim_max; fd++)
        close(fd);

    /* Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /* Инициализировать файл журнала */
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        /* здесь отладочный вывод */
        return(-1);
    }

    /*
     * Назначить корневой каталог текущим рабочим каталогом,
     * чтобы впоследствии можно было отмонтировать файловую систему.
     */
    if (chdir("/") < 0)
        /* здесь отладочный вывод */
        return(-1);

    syslog(LOG_INFO, "daemon successfully started"); // напишем системный лог
    return(1);
}

int
already_running(void)
{
    int fd;
    struct flock lock;

    lock.l_type = F_WRLCK;     /* F_RDLCK, F_WRLCK, F_UNLCK */
    lock.l_start = 0;  /* смещение в байтах относительно l_whence */
    lock.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
    lock.l_len = 1;       /* количество байт (0 – до конца файла) */

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
    if (fd < 0) {
        syslog(LOG_ERR, "Can't open lockfile %s: %s", LOCKFILE, strerror(errno));
        return(-1);
    }
    if (fcntl((fd), F_SETLK, &lock) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return(1);
        }
        syslog(LOG_ERR, "Can't locking %s: %s", LOCKFILE, strerror(errno));
        return(-1);
    }
//    ftruncate(fd, 0);
    return(0);
}

void
read_config(struct config * conf, char * file)
{
    int state = 0;
    char *tk;

    int ** ppdatai;
    char ** ppdatac;

    yaml_parser_t parser;
    yaml_token_t  token;

    conf->config = file;

    FILE *fh = fopen(conf->config, "r");

    /* Initialize parser */
    if(!yaml_parser_initialize(&parser))
        fputs("Failed to initialize parser!\n", stderr);
    if(fh == NULL)
        fputs("Failed to open file!\n", stderr);

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    /* BEGIN new code */
    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type)
        {
            /* Token types (read before actual token) */
            case YAML_KEY_TOKEN:   state = 0; break;
            case YAML_VALUE_TOKEN: state = 1; break;
            case YAML_SCALAR_TOKEN:
                tk = token.data.scalar.value;
                if (state == 0) {
                    if (!strcmp(tk, "socket")) {
                        ppdatac = &conf->socket;
                    } else if (!strcmp(tk, "pidfile")) {
                        ppdatac = &conf->pidfile;
                    } else if (!strcmp(tk, "debug")) {
                        ppdatai = &conf->debug;
                    } else if (!strcmp(tk, "workers")) {
                        ppdatai = &conf->workers;
                    } else {
                        printf("Unrecognised key: %s\n", tk);
                    }
                } else {
                    *ppdatac = strdup(tk);
                }
                break;
            default: break;
        }
        if(token.type != YAML_STREAM_END_TOKEN)
            yaml_token_delete(&token);
    } while(token.type != YAML_STREAM_END_TOKEN);
    yaml_token_delete(&token);
    /* END new code */

    /* Cleanup */
    yaml_parser_delete(&parser);
    fclose(fh);
}

void
show_usage(void)
{
    printf("Usage:\n %s [oprtions]", NAME);
    printf("\n");
    printf("\nOptions:");
    printf("\n -c|--config <path>  full path to file configuration");
    printf("\n -d|--debug          start daemon with debug mode enable");
    printf("\n -v|--version        show version and exit");
    printf("\n -h|--help           show this help and exit");
    printf("\n");
}

void
show_version(void)
{
    printf("%s %s", NAME, VERSION);
    printf("\n");
}




