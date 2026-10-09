#include <stdio.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void *handle_request(void *arg) {
    int client_fd = (int)(intptr_t)arg;
    char buffer[1024];
    char headers[512];
    const char *not_found_fp = "public/notFound.html";

    const char *invalid_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 15\r\n"
                                   "\r\n"
                                   "Page Not Found.";
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_received == -1)
    {
        perror("Read Failed");
        close(client_fd);
        return NULL;
    }
    if (bytes_received == 0)
    {
        printf("Client Disconnected.\n");
        close(client_fd);
        return NULL;
    }

    char method[16];
    char path[256];
    char version[16];

    memset(method, 0, sizeof(method));
    memset(path, 0, sizeof(path));
    memset(version, 0, sizeof(version));

    int fields_parsed = sscanf(buffer, "%15s %255s %15s", method, path, version);

    if (fields_parsed < 3)
    {
        printf("Error: Failed to parse a valid HTTP Request Line.\n");
    }
    else
    {
        printf("\n--- Parsed HTTP Request ---\n");
        printf("Method: %s\n", method);
        printf("Path: %s\n", path);
        printf("Version: %s\n", version);
        printf("---------------------------\n\n");
    }

    char filepath[512];

    if (strcmp(path, "/") == 0)
    {
        strcpy(filepath, "public/index.html");
    }
    else
    {
        snprintf(filepath, sizeof(filepath), "public%s", path);
    }

    FILE *fptr;

    fptr = fopen(filepath, "r");
    if (!fptr)
    {
        perror("Path does not exist");
        FILE *not_found_fptr = fopen(not_found_fp, "r");
        if (!not_found_fptr)
        {
            send(client_fd, invalid_response, strlen(invalid_response), 0);
            close(client_fd);
            return NULL;
        }
        fseek(not_found_fptr, 0, SEEK_END);
        long filesize = ftell(not_found_fptr);
        fseek(not_found_fptr, 0, SEEK_SET);

        char *filebuffer = malloc(filesize + 1);

        fread(filebuffer, 1, filesize, not_found_fptr);

        int header_len = snprintf(headers, sizeof(headers), "HTTP/1.1 404 Not Found\r\n"
                                                            "Content-Type: text/html\r\n"
                                                            "Content-Length: %ld\r\n"
                                                            "\r\n",
                                    filesize);
        send(client_fd, headers, header_len, 0);
        send(client_fd, filebuffer, filesize, 0);
        fclose(not_found_fptr);
        free(filebuffer);
    }
    else
    {
        fseek(fptr, 0, SEEK_END);
        long filesize = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);

        char *file_buffer = malloc(filesize + 1);

        fread(file_buffer, 1, filesize, fptr);
        int header_len = snprintf(headers, sizeof(headers), "HTTP/1.1 200 OK\r\n"
                                                            "Content-Type: text/html\r\n"
                                                            "Content-Length: %ld\r\n"
                                                            "\r\n",
                                    filesize);

        send(client_fd, headers, header_len, 0);
        send(client_fd, file_buffer, filesize, 0);
        fclose(fptr);
        free(file_buffer);
    }
    close(client_fd);
    return NULL;
};

int main()
{
    // Create socket with IPv4 and TCP
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    // If socket creation fails, value will be -1
    if (socketfd == -1)
    {
        perror("Socket Failed"); 
        return 1; // Stop program
    }

    // addr used to store server IPv4 address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); // Set addr values to 0, ensuring no garbage values especially in addr.sin_zero
    addr.sin_family = AF_INET; // Address family set to IPv4
    addr.sin_port = htons(8080); // Port set to 8080. Converted to Big Endian
    addr.sin_addr.s_addr = INADDR_ANY; // Set to listen on every network card on the device

    // Attach socket to ip address and port
    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Bind Failed");
        close(socketfd);
        return 1;
    }

    // Listen on the socket, queueing up to 5 connections
    if (listen(socketfd, 5) == -1)
    {
        perror("Listen Failed");
        close(socketfd);
        return 1;
    }
    printf("Server is listening on port 8080...\n");

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int new_socket = accept(socketfd, (struct sockaddr *)&client_addr, &client_len);
        if (new_socket == -1)
        {
            perror("Accept Failed");
            continue;
        }
        printf("Connection accepted from client.\n");

        pthread_t thread_id;
        int rc = pthread_create(&thread_id, NULL, handle_request, (void *)(intptr_t)new_socket);
        if (rc != 0)
        {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            close(new_socket);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(socketfd);

    return 0;
}

