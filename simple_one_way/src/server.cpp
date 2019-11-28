#include <iostream>
#include <cstdio>   // perror
#include <cstring> // memset
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h> // socket
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>

// Settings
const int MAX_MESSAGE_LENGTH = 256;         // Maximum length of incomming client message
const int PORT_NUMBER = 56765;              // Server port
const char *IP_ADDRESS = "127.0.0.1";       // Server IP

/**
 * Structure with server data.
 */
typedef struct Server {
    int socket_fd;                          // File descriptor with server's socket
    struct sockaddr_in address;             // Stucture with server's IP address and port number
} Server;

/**
 * Structure with client data.
 */
typedef struct Client {
    int socket_fd;                          // File descriptor with client's socket
    struct sockaddr_in address;             // Structure with client's IP adrress
    socklen_t address_length;               // Length of sockaddr_in structure
    char address_string[INET_ADDRSTRLEN];   // Client's IP address as a string

    char message[MAX_MESSAGE_LENGTH];       // Message received from client
    int message_length;                     // Length of message received from client
} Client;

// Global instances necessary for signal handler.
Server server;
Client client;

/**
 * This function handles "premature" end of execution (when user exits it).
 */
void signal_handler(int sig) {
    std::cout << "Exiting..." << std::endl;
    close(server.socket_fd);
    close(client.socket_fd);
}

int register_signal_handler(void (*handler)(int)) {
    int ret = 0;

    struct sigaction signal_action;
    signal_action.sa_handler = handler;
    signal_action.sa_flags = 0;
    sigemptyset(&signal_action.sa_mask);

    // SIGINT - CTRL + C
    // SIGQUIT - CTRL + Q
    // SIGTSTP - CTRL + Z
    ret =      sigaction(SIGINT, &signal_action, NULL) 
            && sigaction(SIGQUIT, &signal_action, NULL) 
            && sigaction(SIGTSTP, &signal_action, NULL);

    return ret;
}

int main(int argc, char **argv) {
    int ret = 0;

    // Register custom signal handler
    ret = register_signal_handler(&signal_handler);
    if (ret == -1) {
        perror("Could not register signal handler");
        return 1;
    }

    // AF_INET - IPv4
    // SOCK_STREAM - Two way communication
    // 0 - For this type of stream, there is only one, default type
    server.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server.socket_fd == -1) {
        perror("Unable to create server socket");
        return 2;
    }

    // Reset address structure
    memset(&server.address, 0, sizeof(struct sockaddr_in));

    // AF_INET - IPv4
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server.address.sin_port = htons(PORT_NUMBER);

    // Bind address to socket
    ret = bind(server.socket_fd, (struct sockaddr *)&server.address, sizeof(server.address));
    if (ret != 0) {
        perror("Could not bind address to server");
        close(server.socket_fd);
        return 3;
    }

    // Listen for up to 20 connections in queue
    ret = listen(server.socket_fd, 20);
    if (ret != 0) {
        perror("Listening failed");
        close(server.socket_fd);
        return 4;
    }

    std::cout << "Listening on " << IP_ADDRESS << ":" << 56765 << "..." << std::endl;

    // Accept client connection
    client.address_length = sizeof(struct sockaddr_in);
    client.socket_fd = accept(server.socket_fd, (struct sockaddr *)&(client.address), &(client.address_length));

    // Store client's IP address as a string
    inet_ntop(AF_INET, &(client.address.sin_addr), client.address_string, INET_ADDRSTRLEN);

    if (client.socket_fd == -1) {
        perror("Could not accept client");
        close(server.socket_fd);
        return 5;
    }

    // Read up to MAX_MESSAGE_LENGTH bytes from client and store it in client's buffer
    client.message_length = read(client.socket_fd, client.message, MAX_MESSAGE_LENGTH);
    if (client.message_length < 0) {
        perror("Reading from socket failed");
        close(server.socket_fd);
        close(client.socket_fd);
        return 6;
    }

    std::cout << "[" << client.address_string << "]: " << client.message << std::endl;

    // close connection
    close(client.socket_fd);
    close(server.socket_fd);

    std::cout << "Exiting..." << std::endl;

    return 0;
}