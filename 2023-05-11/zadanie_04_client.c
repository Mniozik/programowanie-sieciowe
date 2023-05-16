
/*
 * Copyright (C) 2023 Michal Kalewski <mkalewski at cs.put.poznan.pl>
 *
 * Compilation:  gcc -Wall ./zadanie_04_client.c -o ./zadanie
 * Usage:        ./zadanie SERVER PORT
 *
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int _connect(const char *host, const char *service)
{
    int sfd;
    struct addrinfo hints, *res, *ressave;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, service, &hints, &res);
    ressave = res;
    do
    {
        sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (connect(sfd, res->ai_addr, res->ai_addrlen) == 0)
            break;
        close(sfd);
        sfd = -1;
    } while ((res = res->ai_next) != NULL);
    freeaddrinfo(ressave);
    return sfd; // zwracamy deskryptor gniazda
}

int main(int argc, char **argv)
{
    int sfd, rc;
    char buf[128];

    sfd = _connect(argv[1], argv[2]);
    rc = read(sfd, buf, 128);
    write(1, buf, rc); // wypisujemy to co odczytalismy
    close(sfd);
    return EXIT_SUCCESS;
}
