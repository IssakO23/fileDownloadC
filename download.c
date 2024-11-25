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

// function to handle PING command
void handle_list(FILE *sock)
{
    fprintf(sock, "LIST\n");
    fflush(sock);

    char response[256];
    
    // read and print the first line (+OK or error)
    if (fgets(response, sizeof(response), sock))
    {
        // displays +OK
        printf("%s", response);
    }

    // read file list until the end "."
    printf("Available files:\n");
    while (fgets(response, sizeof(response), sock))
    {
        if (strcmp(response, ".\n") == 0)
        {
            break;
        }
        printf("%s", response);
    }
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

    char choice[10];
    while(1)
    {
        printf("\nMenu:\n");
        printf("1. List Files\n");
        printf("2. Quit\n");
        printf("enter your choice: ");
        scanf("%s", choice);

        if (strcmp(choice, "1") == 0)
        {
            handle_list(sock);
        }
        else if (strcmp(choice, "2") == 0)
        {
            printf("exiting...\n");
            break;
        }
        else
        {
            printf("Invalid choice. Please try again.\n");
        }
    }
}

