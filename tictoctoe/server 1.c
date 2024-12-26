#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define BOARD_SIZE 3

char board[BOARD_SIZE][BOARD_SIZE];
int client_sockets[MAX_CLIENTS];
int current_player = 0;
int end_game = 0;

void init_board() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }
}

void send_to_all(const char* message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > 0) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
}

void send_board() {
    char board_str[BOARD_SIZE * BOARD_SIZE + 1];
    int index = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board_str[index++] = board[i][j];
        }
    }
    board_str[index] = '\0';
    
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "0x03\n%s", board_str);
    send_to_all(message);
}

int check_winner(int row, int col) {
    char symbol = board[row][col];
    
    // Check row
    if (board[row][0] == symbol && board[row][1] == symbol && board[row][2] == symbol)
        return 1;
    
    // Check column
    if (board[0][col] == symbol && board[1][col] == symbol && board[2][col] == symbol)
        return 1;
    
    // Check diagonals
    if (row == col) {
        if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol)
            return 1;
    }
    if (row + col == 2) {
        if (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol)
            return 1;
    }
    
    return 0;
}

int is_board_full() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == ' ')
                return 0;
        }
    }
    return 1;
}

void display_board() {
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf(" %c ", board[i][j]);
            if (j < BOARD_SIZE - 1) printf("|");
        }
        printf("\n");
        if (i < BOARD_SIZE - 1) printf("---+---+---\n");
    }
    printf("\n");
}

void handle_disconnect(int client_index) {
    printf("Client %d disconnected...\n", client_index + 1);
    close(client_sockets[client_index]);
    client_sockets[client_index] = -1;
}

int main() {
    int server_socket, max_fds = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    fd_set readfds;
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("Server listening on port 8080...\n");
    
    init_board();
    
    // Accept connections from clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        printf("Client %d connected...\n", i + 1);                
    }
    
    current_player = 0;    
    
    while (!end_game) { 
        // Notify current player it's their turn
        char turn_message[BUFFER_SIZE];
        snprintf(turn_message, sizeof(turn_message), "0x05 Your Turn\n");
        send(client_sockets[current_player], turn_message, strlen(turn_message), 0);
        
        FD_ZERO(&readfds);
        max_fds = server_socket;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
                if (client_sockets[i] > max_fds)
                    max_fds = client_sockets[i];
            }
        }

        int act = select(max_fds + 1, &readfds, NULL, NULL, NULL);

        if (act < 0) {
            perror("select()");
        } else if (act) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (FD_ISSET(client_sockets[i], &readfds)) {
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
        
                    int n = recv(client_sockets[i], buffer, sizeof(buffer) - 1, 0);
                    if (n <= 0) { 
                        handle_disconnect(i);
                        continue;
                    }

                    if (strncmp(buffer, "0x02", 4) == 0) {
                        if (current_player == i) {
                            int row, col;
                            sscanf(buffer + 4, "%d %d", &row, &col);
                            if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && board[row][col] == ' ') {
                                board[row][col] = (current_player == 0) ? 'X' : 'O';
                                send_board();
                                
                                if (check_winner(row, col)) {
                                    char win_message[BUFFER_SIZE];
                                    snprintf(win_message, sizeof(win_message), "0x04 Won\n");
                                    send(client_sockets[i], win_message, strlen(win_message), 0);
                                    end_game = 1;
                                } else if (is_board_full()) {
                                    send_to_all("0x04 DRAW\n");
                                    end_game = 1;
                                } else {
                                    current_player = (current_player + 1) % MAX_CLIENTS;
                                }
                            } else {
                                send(client_sockets[i], "INVALID_MOVE\n", 13, 0);
                            }
                        } else {
                            send(client_sockets[i], "NOT_YOUR_TURN\n", 14, 0);
                        }
                    } else {
                        send(client_sockets[i], "INVALID_COMMAND\n", 16, 0);
                    }
                }
            }
        }
        
        display_board();
    }
    
    // Close all sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > 0) {
            close(client_sockets[i]);
        }
    }
    close(server_socket);
    return 0;
}
