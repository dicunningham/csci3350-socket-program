/*
 * File:        server.c
 * Course:      CSCI 3550-851 – Communication Networks
 * Assignment:  1 – Socket Programming
 * Author:      Danie Cunningham
 * Date:        June 20, 2025
 *
 * Description:
 *   A simple TCP server that listens on port 50072. Upon accepting a client
 *   connection, it receives a fixed‐length message containing the client’s
 *   name and an integer (1–100). It prints both names and numbers, picks its
 *   own random integer (1–100), computes and displays the sum, then replies
 *   to the client with its name and chosen integer. If it ever receives an
 *   out‐of‐range integer, it cleanly shuts down.
 *
 * Build:
 *     gcc -o server server.c
 *
 * Run:
 *     ./server &
 *     # use 'ps' and 'kill' to manage the process
 *
 * References:
 *   – GeeksforGeeks: “TCP Server–Client Implementation in C”
 *     https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
 *   – POSIX socket API (socket, bind, listen, accept)
 *   – Lowtek Sockets Tutorial
 *     http://www.lowtek.com/sockets/
 *   – Beej’s Guide to Network Programming
 *     http://beej.us/guide/bgnet/
 *   – Codequoi: Sockets and Network Programming in C
 *     https://www.codequoi.com/en/sockets-and-network-programming-in-c/
 *   – W3Schools srand Reference
 *     https://www.w3schools.com/c/ref_stdlib_srand.php
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_PORT 50072
#define NAME_LEN    256
#define BACKLOG     5

struct message {
    char name[NAME_LEN];
    int  number;
};

int main() {
    int listen_fd, conn_fd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    struct message recv_msg, send_msg;
    const char *server_name = "Server of Danie Cunningham";
    int client_num, server_num, sum;
    int running = 1;

    // 1. Create listening socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("[Server] Socket created.\n");

    // Enable the socket address to be reused after the program terminates
    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // 2. Bind to SERVER_PORT on any interface
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(SERVER_PORT);

    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Server] Bound to port %d.\n", SERVER_PORT);

    // 3. Listen
    if (listen(listen_fd, BACKLOG) < 0) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    printf("[Server] Listening…\n");

    srand((unsigned)time(NULL));

    while (running) {
        // 4. Accept
        conn_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &cli_len);
        if (conn_fd < 0) {
            perror("accept");
            break;
        }
        printf("\n[Server] Connection from %s:%d\n",
               inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        // 5. Receive client message
        if (read(conn_fd, &recv_msg, sizeof(recv_msg)) != sizeof(recv_msg)) {
            printf("[Server] Read error.\n");
            close(conn_fd);
            continue;
        }
        printf("[Server] Message received.\n");

        // Print Client name and the server's name
        printf("[Server] Client name:  \"%s\"\n", recv_msg.name);
        printf("[Server] Server name:  \"%s\"\n", server_name);

        recv_msg.number = ntohl(recv_msg.number);
        client_num      = recv_msg.number;

        // Print Client number
        printf("[Server] Client number: %d\n", client_num);

        // Shutdown on out-of-range
        if (client_num < 1 || client_num > 100) {
            printf("[Server] Out-of-range (%d). Shutting down.\n", client_num);
            close(conn_fd);
            break;
        }

        // Choose server number & calculate sum
        server_num = (rand() % 100) + 1;
        printf("[Server] Server number: %d\n", server_num);

        // Calculate sum
        sum = client_num + server_num;
        printf("[Server] Sum: %d + %d  = %d\n", client_num, server_num, sum);

        // prepare and send reply
        memset(&send_msg, 0, sizeof(send_msg));
        strncpy(send_msg.name, server_name, NAME_LEN-1);
        send_msg.number = htonl(server_num);
        write(conn_fd, &send_msg, sizeof(send_msg));
        printf("[Server] Sent name \"%s\" and integer %d\n",
          server_name, server_num);

        close(conn_fd);
        printf("[Server] Connection closed.\n");
    }

    close(listen_fd);
    printf("[Server] Shutdown complete.\n");
    return 0;
}
