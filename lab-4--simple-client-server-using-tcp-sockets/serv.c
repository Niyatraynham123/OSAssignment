#include <netinet/in.h> // structure for storing address information
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h> // for socket APIs
#include <sys/types.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK"

int main(int argc, char const* argv[]) {
    int n, val, idx;
    // Create server socket
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    if (servSockD < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // String store data to recv/send to/from client
    char buf[1024];
    char sbuf[1024];
    char* token;

    // Define server address
    struct sockaddr_in servAddr;

    // List
    list_t *mylist;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to the specified IP and port
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Listen for connections
    if (listen(servSockD, 1) < 0) {
        perror("Listen failed");
        exit(1);
    }

    // Integer to hold client socket
    int clientSocket = accept(servSockD, NULL, NULL);
    if (clientSocket < 0) {
        perror("Accept failed");
        exit(1);
    }

    // Initialize the list
    mylist = list_alloc();

    while (1) {
        // Receive messages from the client socket
        n = recv(clientSocket, buf, sizeof(buf), 0);
        if (n <= 0) {
            if (n == 0) {
                printf("Client disconnected\n");
            } else {
                perror("Recv failed");
            }
            break;
        }

        buf[n] = '\0';

        // Tokenize the received buffer
        token = strtok(buf, " ");

        if (strcmp(token, "exit") == 0) {
            list_free(mylist);
            close(clientSocket);
            exit(0);
        } else if (strcmp(token, "get_length") == 0) {
            // Get the length of the list
            val = list_length(mylist);
            sprintf(sbuf, "%s%d", "Length = ", val);
        } else if (strcmp(token, "add_front") == 0) {
            token = strtok(NULL, " ");  // Get next token (value)
            val = atoi(token);
            list_add_to_front(mylist, val);
            sprintf(sbuf, "%s%d", ACK, val);
        } else if (strcmp(token, "add_back") == 0) {
            token = strtok(NULL, " ");  // Get next token (value)
            val = atoi(token);
            list_add_to_back(mylist, val);
            sprintf(sbuf, "%s%d", ACK, val);
        } else if (strcmp(token, "add_position") == 0) {
            token = strtok(NULL, " ");  // Get index
            idx = atoi(token);
            token = strtok(NULL, " ");  // Get value
            val = atoi(token);
            list_add_at_index(mylist, idx, val);
            sprintf(sbuf, "%s%d at position %d", ACK, val, idx);
        } else if (strcmp(token, "remove_front") == 0) {
            val = list_remove_from_front(mylist);
            sprintf(sbuf, "%s%d", ACK, val);
        } else if (strcmp(token, "remove_back") == 0) {
            val = list_remove_from_back(mylist);
            sprintf(sbuf, "%s%d", ACK, val);
        } else if (strcmp(token, "remove_position") == 0) {
            token = strtok(NULL, " ");
            idx = atoi(token);
            val = list_remove_at_index(mylist, idx);
            sprintf(sbuf, "%s%d", ACK, val);
        } else if (strcmp(token, "print") == 0) {
            // Print the list
            sprintf(sbuf, "%s", listToString(mylist));
        } else if (strcmp(token, "get") == 0) {
            token = strtok(NULL, " ");  // Get index
            idx = atoi(token);
            val = list_get_at_index(mylist, idx);
            sprintf(sbuf, "Value at index %d = %d", idx, val);
        } else {
            sprintf(sbuf, "Unknown command: %s", token);
        }

        // Send the response to the client
        if (send(clientSocket, sbuf, strlen(sbuf), 0) < 0) {
            perror("Send failed");
            break;
        }

        memset(buf, '\0', sizeof(buf));  // Clear buffer for next command
    }

    return 0;
}
