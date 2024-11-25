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
        printf("DEBUG: Initial response: %s", response);
    }

    // read file list until the end "."
    printf("Available files:\n");
    while (fgets(response, sizeof(response), sock))
    {
        printf("DEBUG: Line from server: %s", response);
        if (strcmp(response, ".\n") == 0)
        {
            printf("End of file list.\n");
            break;
        }
        printf("%s", response);
    }
}

// function for GET command
void handle_get(FILE *sock, const char *filename)
{
    // Send GET command
    fprintf(sock, "GET %s\n", filename);
    fflush(sock);

    // Read server response
    char response[256];
    if (fgets(response, sizeof(response), sock) == NULL || strncmp(response, "+OK", 3) != 0)
    {
        printf("Error: %s", response);
        return;
    }

    // Open file for writing
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Could not open file for writing");
        return;
    }

    // Read file data from the server and write to disk
    char buffer[1024];
    size_t bytes_received;
    while ((bytes_received = fread(buffer, 1, sizeof(buffer), sock)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file);
    }

    fclose(file);
    printf("File '%s' downloaded successfully.\n", filename);
}

// function to handle QUIT command
void handle_quit(FILE *sock)
{
    fprintf(sock, "QUIT\n");
    fflush(sock);

    char response[256];
    if (fgets(response, sizeof(response), sock) && strncmp(response, "+OK", 3) == 0)
    {
        printf("Disconnected from server.\n");
    }
    else
    {
        printf("Error during from server.\n");
    }
}
// function to handle GET
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
        printf("2. Download a File\n");
        printf("3. Quit\n");
        printf("enter your choice: ");
        scanf("%s", choice);

        if (strcmp(choice, "1") == 0)
        {
            handle_list(sock);
        }
        else if (strcmp(choice, "2") == 0)
        {
            char filename[256];
            printf("Enter filename to download: ");
            scanf("%s", filename);;
            handle_get(sock, filename);
        }
        else if (strcmp(choice, "3") == 0)
        {
            handle_quit(sock);
            break;
        }
        else
        {
            printf("Invalid choice. Please try again.\n");
        }
    }

    fclose(sock);
    close(fd);
    return 0;
}