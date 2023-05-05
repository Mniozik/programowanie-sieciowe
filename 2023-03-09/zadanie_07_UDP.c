/*
 * Compilation:  gcc -Wall ./zadanie_07_UDP.c -o ./UDP
 * Usage (Terminals):
 *  1) ./UDP localhost 1235
 *  2) nc -u -l 1235
 *  3) echo "Hello" | nc -u localhost 1234
 */

#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char **argv)
{
    int sfd, rc;
    socklen_t sl;
    char buf[128];
    struct sockaddr_in saddr, caddr, caddr2;
    struct hostent *addrent;

    // tworzenie gniazda
    sfd = socket(PF_INET, SOCK_DGRAM, 0);
    

    // --- SERWER ---
    // zerowanie struktury adresu serwera
    memset(&saddr, 0, sizeof(saddr));

    // wypelnianie struktury adresem IP i numerem portu serwera
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);

    // powiazanie gniazda z adresem serwera
    bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));


    //--- ADRESAT WIADOMOSCI ---
    addrent = gethostbyname(argv[1]); // adres IP

    // zerowanie struktury adresu
    memset(&caddr2, 0, sizeof(caddr2));

    // wypelnienie struktury adresata
    caddr2.sin_family = AF_INET;
    memcpy(&caddr2.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
    caddr2.sin_port = htons(atoi(argv[2])); // nr portu 


    while (1)
    {
        memset(&caddr, 0, sizeof(caddr));
        memset(&buf, 0, sizeof(buf));
        sl = sizeof(caddr);

        // odbieranie wiadomosci od klienta
        rc = recvfrom(sfd, buf, 128, 0, (struct sockaddr *)&caddr, &sl);
        printf("Message: %s\n", buf);

        // wysylanie wiadomosci do adresata
        sendto(sfd, buf, rc, 0, (struct sockaddr *)&caddr2, sl);
    }
    close(sfd);
    return EXIT_SUCCESS;
}
