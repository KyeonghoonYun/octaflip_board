// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h> 
#include <stdint.h>   // for uint64_t
#include <limits.h>   // for INT_MIN, INT_MAX
#include "cJSON.h"
#include "board.h"


/**
 * Build two 64-bit masks from the 8×8 char board:
 *   - red_mask  : all indices (0..63) where board[r][c] == 'R'
 *   - blue_mask : all indices where board[r][c] == 'B'
 *
 * Empty squares ('.') are implicitly where neither mask has a bit set.
 */
void board_to_bitboards(const char board[8][8],
                        uint64_t *red_mask,
                        uint64_t *blue_mask)
{
    *red_mask  = 0ULL;
    *blue_mask = 0ULL;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int idx = r * 8 + c;
            uint64_t bit = 1ULL << idx;
            if (board[r][c] == 'R') {
                *red_mask |= bit;
            } else if (board[r][c] == 'B') {
                *blue_mask |= bit;
            }
            // else '.' → do nothing
        }
    }
}

/**
 * Apply one move on bitboards. 'from' and 'to' are indices 0..63.
 *   - If |dr| or |dc| == 2, it’s a jump: remove the bit at 'from'.
 *   - Otherwise (distance==1), it’s a clone: keep the bit at 'from'.
 * Then place your bit at 'to' and flip any adjacent opponent bits.
 *
 * @param my_mask      current player's bitboard
 * @param opp_mask     opponent's bitboard
 * @param from         source index (0..63)
 * @param to           destination index (0..63)
 * @param new_my_mask  [out] updated player's bitboard
 * @param new_opp_mask [out] updated opponent's bitboard
 */
void apply_move_bitboard(uint64_t my_mask,
                         uint64_t opp_mask,
                         int from,
                         int to,
                         uint64_t *new_my_mask,
                         uint64_t *new_opp_mask)
{
    uint64_t bit_from = 1ULL << from;
    uint64_t bit_to   = 1ULL << to;
    int fr = from / 8, fc = from % 8;
    int tr = to   / 8, tc = to   % 8;

    uint64_t my_new  = my_mask;
    uint64_t opp_new = opp_mask;

    // If jump (distance 2), remove original
    int dr = (tr > fr) ? (tr - fr) : (fr - tr);
    int dc = (tc > fc) ? (tc - fc) : (fc - tc);
    if (dr == 2 || dc == 2) {
        my_new &= ~bit_from;
    }
    // Place at 'to'
    my_new |= bit_to;

    // Flip any opponent bits adjacent to 'to'
    static const int dr8[8] = {-1,-1, 0,+1,+1,+1, 0,-1};
    static const int dc8[8] = { 0,+1,+1,+1, 0,-1,-1,-1};
    for (int d = 0; d < 8; d++) {
        int nr = tr + dr8[d];
        int nc = tc + dc8[d];
        if (nr < 0 || nr > 7 || nc < 0 || nc > 7) continue;
        int nidx = nr*8 + nc;
        uint64_t bit_n = 1ULL << nidx;
        if (opp_new & bit_n) {
            opp_new &= ~bit_n;
            my_new &= ~bit_n;
            my_new |= bit_n;
        }
    }

    *new_my_mask  = my_new;
    *new_opp_mask = opp_new;
}


/**
 * Evaluate (my_mask vs. opp_mask) as popcount(my) - popcount(opp).
 */
static inline int evaluate_board(uint64_t my_mask, uint64_t opp_mask)
{
    return __builtin_popcountll(my_mask)
         - __builtin_popcountll(opp_mask);
}
/**
 * List every legal move for 'my_mask' given opponent's bits in 'opp_mask'.
 * No walls exist, so occupancy = my_mask | opp_mask.
 *
 * @param my_mask
 * @param opp_mask
 * @param from_list  array of size ≥1024; write source indexes (0..63)
 * @param to_list    parallel array; write destination indexes
 * @return number of moves found
 */
static int generate_moves_bitboard(uint64_t my_mask,
                                   uint64_t opp_mask,
                                   int *from_list,
                                   int *to_list)
{
    uint64_t occupancy = my_mask | opp_mask;
    int move_count = 0;

    static const int dr8[8] = {-1,-1, 0,+1,+1,+1, 0,-1};
    static const int dc8[8] = { 0,+1,+1,+1, 0,-1,-1,-1};

    uint64_t tmp = my_mask;
    while (tmp) {
        int from_idx = __builtin_ctzll(tmp);
        tmp &= tmp - 1ULL;
        int r = from_idx / 8, c = from_idx % 8;

        // Clone (m=1) and jump (m=2)
        for (int m = 1; m <= 2; m++) {
            for (int d = 0; d < 8; d++) {
                int nr = r + dr8[d]*m;
                int nc = c + dc8[d]*m;
                if (nr < 0 || nr > 7 || nc < 0 || nc > 7) continue;
                int to_idx = nr*8 + nc;
                if ((occupancy & (1ULL << to_idx)) == 0ULL) {
                    from_list[move_count] = from_idx;
                    to_list[move_count]   = to_idx;
                    move_count++;
                }
            }
        }
    }
    return move_count;
}
/**
 * @param my_mask   current player's bits
 * @param opp_mask  opponent's bits
 * @param depth     how many plies left
 * @param alpha
 * @param beta
 *
 * Returns best score from “my” perspective.
 */
int minimax_bitboard(uint64_t my_mask,
                     uint64_t opp_mask,
                     int depth,
                     int alpha,
                     int beta)
{
    if (depth == 0) {
        return evaluate_board(my_mask, opp_mask);
    }

    int from_list[1024], to_list[1024];
    int move_count = generate_moves_bitboard(my_mask, opp_mask,
                                             from_list, to_list);
    if (move_count == 0) {
        return evaluate_board(my_mask, opp_mask);
    }

    int best = INT_MIN;
    for (int i = 0; i < move_count; i++) {
        uint64_t nm, no;
        apply_move_bitboard(my_mask, opp_mask,
                            from_list[i], to_list[i],
                            &nm, &no);
        int score = -minimax_bitboard(no, nm,
                                      depth - 1,
                                      -beta, -alpha);
        if (score > best) {
            best = score;
        }
        if (score > alpha) {
            alpha = score;
            if (alpha >= beta) {
                break;  // cutoff
            }
        }
    }
    return best;
}


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
/**
 * Should replace your old generate_move.
 *   - Converts 8×8 array to (red_mask, blue_mask)
 *   - Chooses my_mask vs. opp_mask based on 'c'
 *   - Generates all root moves; if too many, use depth=5, else depth=6
 *   - Runs minimax on each root move to pick the best
 *   - Sends that move via send_move(...)
 */
void generate_move(int sockfd, const char board[8][8], char c) {
    uint64_t red_mask, blue_mask;
    board_to_bitboards(board, &red_mask, &blue_mask);

    // Determine which is “my” bitboard
    uint64_t my_mask  = (c == 'R') ? red_mask  : blue_mask;
    uint64_t opp_mask = (c == 'R') ? blue_mask : red_mask;

    int from_root[1024], to_root[1024];
    int root_moves = generate_moves_bitboard(my_mask, opp_mask,
                                             from_root, to_root);

    if (root_moves == 0) {
        send_move(sockfd, 0, 0, 0, 0);
        return;
    }

    int max_depth = (root_moves > 60) ? 6 : 7;
    int best_from = -1, best_to = -1;
    int alpha = INT_MIN, beta = INT_MAX;

    for (int i = 0; i < root_moves; i++) {
        uint64_t nm, no;
        apply_move_bitboard(my_mask, opp_mask,
                            from_root[i], to_root[i],
                            &nm, &no);
        int score = -minimax_bitboard(no, nm,
                                      max_depth - 1,
                                      -beta, -alpha);
        if (score > alpha) {
            alpha = score;
            best_from = from_root[i];
            best_to   = to_root[i];
        }
    }

    int fr = best_from / 8, fc = best_from % 8;
    int tr = best_to   / 8, tc = best_to   % 8;
    send_move(sockfd,
              fr + 1, fc + 1,
              tr + 1, tc + 1);
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
            led_clear();
            led_delete();
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
                    if (!led_initialize()) {
                        fprintf(stderr, "Failed to initialize LED panel\n");
                        exit = 0;
                        break;
                    }
                } else if (strcmp(type->valuestring, "your_turn") == 0 || strcmp(type->valuestring, "invalid_move") == 0) {
                    draw_board(board_local);
                    generate_move(sockfd, board_local, c);
                }
                
            }
            // move remaining data forward
            len -= (p - buffer + 1);
            memmove(buffer, p + 1, len);
            buffer[len] = '\0';
        }
    }
    led_clear();
    led_delete();
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