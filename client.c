/*
 * File:        client.c
 * Course:      CSCI 3550-851 – Communication Networks
 * Assignment:  1 – Socket Programming
 * Author:      Danie Cunningham
 * Date:        June 20, 2025
 *
 * Description:
 *   A TCP client that prompts the user for an integer (1–100). It binds its
 *   own source port 57476, then connects to the server at IP (default
 *   127.0.0.1) port 50072, sends its name & number, waits for reply (server
 *   name & number), prints both names/numbers and sum, then exits.
 *
 * Build:
 *     gcc -o client client.c
 *
 * Usage:
 *     ./client [server_IP]
 *   e.g.: ./client 192.168.1.5
 *
 * References:
 *   – GeeksforGeeks: “TCP Server–Client Implementation in C”
 *     https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
 *   – Lowtek Sockets Tutorial
 *     http://www.lowtek.com/sockets/
 *   – Beej’s Guide to Network Programming
 *     http://beej.us/guide/bgnet/
 *   – Codequoi: Sockets and Network Programming in C
 *     https://www.codequoi.com/en/sockets-and-network-programming-in-c/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_PORT 50072
#define CLIENT_PORT 57476
#define NAME_LEN    256

struct message {
    char name[NAME_LEN];
    int  number;
};

int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_in serv_addr, local_addr;
    struct message send_msg, recv_msg;
    const char *client_name = "Client of Danie Cunningham";
    int num, server_num, sum;

    // Validate input
    while (1) {
        printf("Enter an integer [1–100]: ");
        if (scanf("%d", &num) != 1) {
            fprintf(stderr, "Invalid input: not an integer. Please try again.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        break; // Input is a number - passing to server
    }

    // Create socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("[Client] Socket created.\n");

    // Bind to CLIENT_PORT
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port        = htons(CLIENT_PORT);
    if (bind(sock_fd, (struct sockaddr*)&local_addr,
             sizeof(local_addr)) < 0) {
        perror("bind(local)");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Client] Bound to local port %d.\n", CLIENT_PORT);

    // Prepare server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(SERVER_PORT);
    const char *ip = (argc > 1) ? argv[1] : "127.0.0.1";
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP: %s\n", ip);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Connect
    if (connect(sock_fd, (struct sockaddr*)&serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Client] Connected to %s:%d\n", ip, SERVER_PORT);

    // Send name + number
    memset(&send_msg, 0, sizeof(send_msg));
    strncpy(send_msg.name, client_name, NAME_LEN - 1);
    send_msg.number = htonl(num);
    if (write(sock_fd, &send_msg, sizeof(send_msg)) !=
        sizeof(send_msg)) {
        perror("[Client] write");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Client] Sent name \"%s\" and integer %d\n",
           client_name, num);

    printf("[Client] Waiting for reply...\n");

    // Receive reply
    if (read(sock_fd, &recv_msg, sizeof(recv_msg)) !=
        sizeof(recv_msg)) {
        fprintf(stderr, "[Client] Read error or server closed.\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    recv_msg.number = ntohl(recv_msg.number);
    server_num      = recv_msg.number;
    sum             = num + server_num;

    printf("[Client] Reply received.\n");

    // Display results 
    printf("[Client] Client name: \"%s\"\n", send_msg.name);
    printf("[Client] Server name: \"%s\"\n", recv_msg.name);
    printf("[Client] Client number: %d\n", num);
    printf("[Client] Server number: %d\n", server_num);
    printf("[Client] Sum: %d + %d = %d\n", num, server_num, sum);

    // Close socket
    close(sock_fd);
    printf("[Client] Socket closed. Exiting.\n");
    return 0;
}