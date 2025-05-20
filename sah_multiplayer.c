#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "logica_sah.h"

#define PORT 12345
#define BUF_SIZE 64

int coloana(char c) {
    return toupper(c) - 'A';
}

int randul(char c) {
    return '8' - c;
}

void parseaza_mutare(const char *input, int *fromRow, int *fromCol, int *toRow, int *toCol) {
    *fromCol = coloana(input[0]);
    *fromRow = randul(input[1]);
    *toCol = coloana(input[3]);
    *toRow = randul(input[4]);
}

void citeste_mutare(char *buf, int size) 
{
    printf("Introdu mutarea (ex: A2 A4): ");
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0;
}

void trimite(int sockfd, const char *msg) 
{
    send(sockfd, msg, strlen(msg), 0);
}

int primeste(int sockfd, char *buf, int size) 
{
    memset(buf, 0, size);
    return recv(sockfd, buf, size - 1, 0);
}

void ruleaza_joc(int sockfd, int sunt_server) 
{
    int tura_mea = sunt_server; // Serverul = alb (mută primul)
    initBoard();

    while (1) {
        printBoard();
        if (sahmat(sunt_server ? P_WHITE : P_BLACK)) {
            printf("ȘAH MAT! %s câștigă!\n", sunt_server ? "Negrul" : "Alb");
            break;
        }
        if (stalemate(sunt_server ? P_WHITE : P_BLACK, 0)) {
            printf("Pat! Remiză.\n");
            break;
        }

        if (tura_mea) {
            char buf[BUF_SIZE];
            int fr, fc, tr, tc;

            do {
                citeste_mutare(buf, BUF_SIZE);
                parseaza_mutare(buf, &fr, &fc, &tr, &tc);
            } while (!checkIfMoveIsPossible(fc, fr, tc, tr, sunt_server ? P_WHITE : P_BLACK));

            // Execută și trimite mutarea
            board[tr][tc] = board[fr][fc];
            board[fr][fc].type = EMPTY;
            board[fr][fc].color = NONE;
            trimite(sockfd, buf);
        } else {
            char buf[BUF_SIZE];
            printf("Aștept mutarea adversarului...\n");
            primeste(sockfd, buf, BUF_SIZE);
            int fr, fc, tr, tc;
            parseaza_mutare(buf, &fr, &fc, &tr, &tc);

            // Execută mutarea
            board[tr][tc] = board[fr][fc];
            board[fr][fc].type = EMPTY;
            board[fr][fc].color = NONE;
            printf("Mutarea primită: %s\n", buf);
        }
        tura_mea = !tura_mea;
    }
}

int server_mode() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Aștept un jucător la portul %d...\n", PORT);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("accept");
        exit(1);
    }

    printf("Conectat la client!\n");
    close(server_fd);
    return client_fd;
}

int client_mode(const char *ip) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("Adresă IP invalidă\n");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Conectat la server!\n");
    return sock;
}

int main() 
{
    int opt;
    char ip[32];

    printf("=== SAH MULTIPLAYER ===\n");
    printf("1. Server (alb)\n");
    printf("2. Client (negru)\n");
    printf("Alege: ");
    scanf("%d", &opt);
    getchar(); // curăță '\n'

    int sockfd;
    if (opt == 1) {
        sockfd = server_mode();
        ruleaza_joc(sockfd, 1); // server = alb
    } else {
        printf("IP server: ");
        fgets(ip, sizeof(ip), stdin);
        ip[strcspn(ip, "\n")] = 0;
        sockfd = client_mode(ip);
        ruleaza_joc(sockfd, 0); // client = negru
    }

    close(sockfd);
    return 0;
}
//pentru rulare gcc sah_multiplayer_terminal.c logica_sah.c -o sah_multiplayer
//./sah_multiplayer // alege 1 (server) si apoi 2(host) si baga portul
