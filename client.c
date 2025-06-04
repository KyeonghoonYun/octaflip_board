// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include <time.h> 
#include "board.h"


char *name;

void send_json(int sockfd, cJSON *json) {
    char *msg = cJSON_PrintUnformatted(json);
    send(sockfd, msg, strlen(msg), 0);
    send(sockfd, "\n", 1, 0);
    free(msg);
}

int connect_to_server(const char *ip, const char *port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(ip, port, &hints, &res);
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sockfd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    return sockfd;
}

void send_register(int sockfd, const char *username) {
    cJSON *msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "type", "register");
    cJSON_AddStringToObject(msg, "username", username);
    send_json(sockfd, msg);
    cJSON_Delete(msg);
}

void send_move(int sockfd, int sx, int sy, int tx, int ty){
    cJSON *msg = cJSON_CreateObject();
    
    cJSON_AddStringToObject(msg, "type", "move");
    cJSON_AddStringToObject(msg, "username", name);
    cJSON_AddNumberToObject(msg, "sx", sx);
    cJSON_AddNumberToObject(msg, "sy", sy);
    cJSON_AddNumberToObject(msg, "tx", tx);
    cJSON_AddNumberToObject(msg, "ty", ty);
    send_json(sockfd, msg);
    cJSON_Delete(msg);
}

int in_board(int x) {
    return 0 < x && x < 9;
}

int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};
typedef struct {
    int sx, sy, tx, ty;
} Move;

void generate_move(int sockfd, const char board[8][8], char c) {
    Move moves[1024];
    int move_count = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == c) {
                for (int m = 1; m <= 2; m++) {
                    for (int d = 0; d < 8; d++) {
                        int ni = i + dy[d] * m;
                        int nj = j + dx[d] * m;
                        if (in_board(ni+1) && in_board(nj+1) && board[ni][nj] == '.') {
                            moves[move_count++] = (Move){i + 1, j + 1, ni + 1, nj + 1};
                        }
                    }
                }
            }
        }
    }

    if (move_count == 0) {
        send_move(sockfd, 0, 0, 0, 0);
    } else {
        srand(time(NULL));
        int idx = rand() % move_count;
        send_move(sockfd, moves[idx].sx, moves[idx].sy, moves[idx].tx, moves[idx].ty);
    }
}

//void *handle_socket(void *arg){
void handle_socket(int sockfd){
    //int sockfd = (intptr_t)arg;
    char buffer[1024], board_local[8][8]; size_t len = 0; char *p; ssize_t n;
    int exit = 1;
    char c = 0;
    while (exit) {
        n = recv(sockfd, buffer + len, sizeof(buffer) - len - 1, 0);
        if (n <= 0) {
            printf("server disconnected");
            return;
        }
        len += n;
        buffer[len] = '\0';

        while ((p = strchr(buffer, '\n')) != NULL) {
            *p = '\0';
            cJSON *msg = cJSON_Parse(buffer);
            if (msg) {
                char *dump = cJSON_PrintUnformatted(msg);
                printf("Received JSON from server:\n%s\n", dump);
                free(dump);
                const cJSON *type = cJSON_GetObjectItem(msg, "type");
                const cJSON *board = cJSON_GetObjectItem(msg, "board");
                if (cJSON_IsArray(board)){
                    printf("Current board:\n");
                    int board_size = cJSON_GetArraySize(board);
                    for (int i = 0; i < board_size; i++) {
                        const cJSON *row = cJSON_GetArrayItem(board, i);
                        if (cJSON_IsString(row) && row->valuestring != NULL) {
                            printf("%s\n", row->valuestring);
                            memcpy(board_local[i], row->valuestring, 8);
                        }
                    }
                }
                if (!cJSON_IsString(type)){
                    printf("server message corrupted\n");
                } else if (strcmp(type->valuestring, "game_over") == 0) {
                    // print results
                    const cJSON *scores = cJSON_GetObjectItem(msg, "scores");
                    if (scores) {
                        cJSON *entry = NULL;
                        cJSON_ArrayForEach(entry, scores) {
                            const char *uname = entry->string;
                            int score = entry->valueint;
                            printf("%s: %d points\n", uname, score);
                        }
                    }
                    exit = 0;
                    break;
                    //return NULL;
                } else if  (strcmp(type->valuestring, "register_ack") == 0) {
                    printf("[client] registered\n");
                } else if (strcmp(type->valuestring, "register_nack") == 0) {
                    printf("[client] register failed\n");
                    cJSON_Delete(msg);
                    exit = 0;
                    break;
                    //return NULL;
                } else if (strcmp(type->valuestring, "game_start") == 0) {
                    printf("[client] game started\n");
                    const cJSON *first_player = cJSON_GetObjectItem(msg, "first_player");
                    if (cJSON_IsString(first_player) && strcmp(first_player->valuestring, name) == 0)
                        c = 'R';
                    else
                        c = 'B';
                } else if (strcmp(type->valuestring, "your_turn") == 0 || strcmp(type->valuestring, "invalid_move") == 0) {
                    generate_move(sockfd, board_local, c);
                }
                
            }
            // move remaining data forward
            len -= (p - buffer + 1);
            memmove(buffer, p + 1, len);
            buffer[len] = '\0';
        }
    }
    //return NULL;
}

// for debug
void game_start(int sockfd){
    while (1){
        int invalid = 0, move[4];
        char movec[4][50], *endptr;
        scanf("%s %s %s %s", movec[0], movec[1], movec[2], movec[3]);
        for (int i=0;i<4;i++){
            int num = (int)strtol(movec[i], &endptr, 10);
            
            if (*endptr != '\0') {
                printf("[error] invalid move");
                invalid = 1;
                break;
            }
            move[i] = num;
        }
        if (invalid) continue;
        send_move(sockfd , move[0], move[1], move[2], move[3]);
    }
}

// not used
void await_game_start(int sockfd) {
    char buffer[1024];
    size_t len = 0;
    char *p;
    while (1) {
        ssize_t n = recv(sockfd, buffer + len, sizeof(buffer) - len - 1, 0);
        if (n <= 0) break;
        len += n;
        buffer[len] = '\0';

        while ((p = strchr(buffer, '\n')) != NULL) {
            *p = '\0';
            cJSON *msg = cJSON_Parse(buffer);
            if (msg) {
                printf("Received JSON from server:\n%s\n", cJSON_PrintUnformatted(msg));
                const cJSON *type = cJSON_GetObjectItem(msg, "type");
                if (cJSON_IsString(type)) {
                    if (strcmp(type->valuestring, "register_ack") == 0) {
                        printf("[client] registered\n");
                    } else if (strcmp(type->valuestring, "register_nack") == 0) {
                        printf("[client] register failed\n");
                        cJSON_Delete(msg);
                        return;
                    } else if (strcmp(type->valuestring, "game_start") == 0) {
                        printf("[client] game started\n");
                        cJSON_Delete(msg);

                        //game loop
                        game_start(sockfd);
                        return;

                    }
                }
                cJSON_Delete(msg);
            }

            // move remaining data forward
            len -= (p - buffer + 1);
            memmove(buffer, p + 1, len);
            buffer[len] = '\0';
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 7 && strcmp(argv[1], "-ip") == 0 && strcmp(argv[3], "-port") == 0 && strcmp(argv[5], "-username") == 0) {
        name =  malloc(strlen(argv[6]) + 1);
        strcpy(name, argv[6]);
        
        int sockfd = connect_to_server(argv[2], argv[4]);
        if (sockfd <= 0){
            printf("[error] unable to connect to server\n");
        }else {
            send_register(sockfd, argv[6]);
            //pthread_t tid;
            //pthread_create(&tid, NULL, handle_socket, (void *)(intptr_t)sockfd);
            //pthread_detach(tid);
            //game_start(sockfd);
            handle_socket(sockfd);
        }
        close(sockfd);
        free(name);
    } else {
        fprintf(stderr, "Usage: %s -ip <ip_address> -username <name>\n", argv[0]);
        return 1;
    }
    return 0;
}