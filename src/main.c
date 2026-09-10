#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        perror("Socket Failed");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Bind Failed");
        close(socketfd);
        return 1;
    }

    if (listen(socketfd, 5) == -1)
    {
        perror("Listen Failed");
        close(socketfd);
        return 1;
    }
    printf("Server is listening on port 8080...\n");

    char buffer[1024];
    char headers[512];

    const char *invalid_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 15\r\n"
                                   "\r\n"
                                   "Page Not Found.";

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

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_received == -1)
        {
            perror("Read Failed");
            close(new_socket);
            continue;
        }
        if (bytes_received == 0)
        {
            printf("Client Disconnected.\n");
            close(new_socket);
            continue;
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
            ssize_t res = send(new_socket, invalid_response, strlen(invalid_response), 0);
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

            send(new_socket, headers, header_len, 0);
            send(new_socket, file_buffer, filesize, 0);
            fclose(fptr);
            free(file_buffer);
        }

        close(new_socket);
    }

    close(socketfd);

    return 0;
}