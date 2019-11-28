#include <iostream>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h> // socket
#include <netinet/in.h>
#include <arpa/inet.h>

// Server's IP address
const char *SERVER_IP_ADDRESS = "127.0.0.1";
// Which port to connect to
const int PORT_NUMBER = 56765;

int main(int argc, char **argv) {
    // System socket for the client
    int socket_fd;
    // Struct with representation of server address
    struct sockaddr_in server_address;
    // Message for the server
    const char *message = "Hi, server!";
    int ret = 0;

    // Create client socket file descriptor
    // AF_INET - IPv4
    // SOCK_STREAM - TCP
    // 0 - Only one stream type for TCP is available
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Could not create client socket");
        return 1;
    }

    // Reset address structure
    memset(&server_address, 0, sizeof(struct sockaddr_in));

    // Setup server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    server_address.sin_port = htons(PORT_NUMBER);

    // Try to connecto to the server
    ret = connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (ret != 0) {
        close(socket_fd);
        perror("Connection failed");
        return 2;
    }

    // Send message
    ret = send(socket_fd, message, strlen(message), 0);
    if (ret == -1) {
        close(socket_fd);
        perror("Message was not sent");
        return 3;
    }

    std::cout << "Message sent." << std::endl;

    close(socket_fd);
    return 0;
}