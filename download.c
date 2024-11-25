#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libsocket/libinetsocket.h>

#define PORT "3456"

// function that handles the HELO command
void handle_helo(FILE *sock)
{
    fprintf(sock, "HELO\n");
    fflush(sock);

    char response[256];
    fgets(response, sizeof(response), sock);
    printf("%s", response);
}

int main()
{
    char server[256];
    printf("Enter server to connect (e.g newark or london): ");
    scanf("%s", server);

    char hostname[512];
    snprintf(hostname, sizeof(hostname), "%s.cs.sierracollege.edu", server);

    int fd = create_inet_stream_socket(hostname, PORT, LIBSOCKET_IPv4, 0);
    if (fd < 0)
    {
        perror("Could not connect to server");
        exit(1);
    }

    FILE *sock = fdopen(fd, "r+");
    if (!sock)
    {
        perror("Failed to open socket as FILE*");
        close(fd);
        exit(1);
    }

    printf("Connected to %s\n", hostname);
    handle_helo(sock);

    fclose(sock);
    close(fd);
    return 0;
}