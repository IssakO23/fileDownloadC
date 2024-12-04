#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libsocket/libinetsocket.h>
#include <sys/time.h> // Required for gettimeofday

#define PORT "3456"

// Function to get the current time in seconds (used to calculate download speed)
double current_time()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec + now.tv_usec / 1000000.0;
}

// Function to handle the LIST command
void handle_list(FILE *sock)
{
    fprintf(sock, "LIST\n");
    fflush(sock);

    char response[256];
    
    // Read and print the first line (+OK or error)
    if (fgets(response, sizeof(response), sock))
    {
        printf("Filename: %s", response);
    }

    // Read file list until the end "."
    printf("Available files:\n");
    while (fgets(response, sizeof(response), sock))
    {
        if (strcmp(response, ".\n") == 0)
        {
            printf("End of file list.\n");
            break;
        }
        printf("%s", response);
    }
}

// Function for GET command
void handle_get(FILE *sock, const char *filename)
{
    char response[256];
    size_t file_size = 0, bytes_downloaded = 0;
    unsigned char buffer[1024];
    const size_t buffer_limit = sizeof(buffer);

    // Request the file size
    fprintf(sock, "SIZE %s\n", filename);
    fflush(sock);

    if (fgets(response, sizeof(response), sock) == NULL || strncmp(response, "+OK", 3) != 0)
    {
        printf("Error: Unable to retrieve file size. Server said: %s\n", response);
        return;
    }

    if (sscanf(response + 4, "%zu", &file_size) != 1)
    {
        printf("Error: Could not determine file size from server response: %s\n", response);
        return;
    }

    printf("Preparing to download '%s' (%zu bytes).\n", filename, file_size);

    // Request the file content
    fprintf(sock, "GET %s\n", filename);
    fflush(sock);

    if (fgets(response, sizeof(response), sock) == NULL || strncmp(response, "+OK", 3) != 0)
    {
        printf("Error: Unable to download file. Server said: %s\n", response);
        return;
    }

    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Error: Could not open local file for writing");
        return;
    }

    double start_time = current_time();

    // Read and write file content
    while (bytes_downloaded < file_size)
    {
        size_t remaining = file_size - bytes_downloaded;
        size_t to_read = (remaining < buffer_limit) ? remaining : buffer_limit;

        size_t received = fread(buffer, 1, to_read, sock);
        if (received == 0)
        {
            if (feof(sock))
                printf("Error: Connection closed unexpectedly.\n");
            else
                perror("Error: Unable to read from server");
            fclose(file);
            return;
        }

        size_t written = fwrite(buffer, 1, received, file);
        if (written != received)
        {
            perror("Error: Unable to write to local file");
            fclose(file);
            return;
        }

        bytes_downloaded += received;
        printf("\rProgress: %zu / %zu bytes (%.2f%%)", bytes_downloaded, file_size, (bytes_downloaded / (double)file_size) * 100);
        fflush(stdout);
    }

    fclose(file);

    double end_time = current_time();
    double elapsed_time = end_time - start_time;
    double download_speed = (bytes_downloaded / 1e6) / elapsed_time; // MB/s

    printf("\nDownload complete: '%s' (%zu bytes).\n", filename, bytes_downloaded);
    printf("Average speed: %.2f MB/s.\n", download_speed);
}

// Function to handle the QUIT command
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

    char choice[10];
    while (1)
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
            scanf("%s", filename);
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