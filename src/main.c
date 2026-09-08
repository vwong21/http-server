#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
int main() {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(socketfd, (struct sockaddr*) &addr, sizeof(addr));
    listen(socketfd, 5);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_socket = accept(socketfd, (struct sockaddr*) &client_addr, &client_len);
    printf("new connection accepted\n");
    return 0;
}