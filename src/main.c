#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
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
        perror("Bind Failed\n");
        return 1;
    }

    if (listen(socketfd, 5) == -1)
    {
        perror("Listen Failed\n");
        return 1;
    }
    printf("Server is listening on port 8080...\n");

    char buffer[1024];
    const char *http_response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 18\r\n"
                                "\r\n"
                                "Hello from server!";

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_socket = accept(socketfd, (struct sockaddr *)&client_addr, &client_len);
        if (new_socket == -1)
        {
            perror("Accept Failed\n");
            return 1;
        }
        printf("Connection accepted from client.\n");

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_received == -1)
        {
            perror("Read Failed\n");
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

        ssize_t bytes_sent = send(new_socket, http_response, strlen(http_response), 0);
        if (bytes_sent == -1)
        {
            perror("Send Failed\n");
            close(new_socket);
            continue;
        }
        close(new_socket);
    }

    close(socketfd);

    return 0;
}