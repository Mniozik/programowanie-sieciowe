/*
Compilation:  gcc -Wall server.c -o server
Usage:        ./server 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    printf("|--- ARP-SCAN (Server) ---|\n");
    int socket_fd, cfd;
    char buffer[100];
    socklen_t sl;
    struct sockaddr_in server_addr, client_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("Error: socket");
        return 1;
    }

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Błąd bind");
        return 1;
    }

    if (listen(socket_fd, 5) == -1)
    {
        perror("Error: listen");
        return 1;
    }

    sl = sizeof(client_addr);
    cfd = accept(socket_fd, (struct sockaddr *)&client_addr, &sl);
    if (cfd == -1)
    {
        perror("Error: accept");
        return 1;
    }

    while (1)
    {
        ssize_t bytes_received = recv(cfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    close(cfd);
    close(socket_fd);
    return 0;
}
