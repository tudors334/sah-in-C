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

int coloana(char c) 
{
    return toupper(c) - 'A';
}

int randul(char c) {
    return '8' - c;
}

void parseaza_mutare(const char *input, int *fromRow, int *fromCol, int *toRow, int *toCol)
 {
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

void ruleaza_joc(Board *board, int este_server) 
{
    char input[20];
    int remiza_propusa = 0;
    int rand = 1; // 1 - alb, 0 - negru
    int joc_terminat = 0;

    afiseaza_tabla(*board);

    while (!joc_terminat) 
    {
        printf("Este randul %s\n", rand ? "albului" : "negrului");

        if (regele_in_sah(*board, rand))
            {
            if (este_sah_mat(*board, rand)) 
            {
                printf("Șah mat! %s a pierdut.\n", rand ? "Alb" : "Negru");
                break;
            } 
            else 
            {
                printf("%s este în șah!\n", rand ? "Alb" : "Negru");
            }
        } 
        else if (este_pat(*board, rand)) 
        {
            printf("Remiză prin pat!\n");
            break;
        }

        printf("Introdu mutarea (ex: E2 E4 sau 'draw'): ");
        if (este_server)
            fgets(input, sizeof(input), stdin);
        else
            recv(socket_id, input, sizeof(input), 0); 

        input[strcspn(input, "\n")] = 0; 

        if (strcmp(input, "draw") == 0)
        {
            if (remiza_propusa) 
            {
                printf("Remiză acceptată. Jocul s-a încheiat.\n");
                break;
            }
            else
                {
                printf("Remiză propusă. Dacă oponentul scrie 'draw', jocul se încheie remiză.\n");
                remiza_propusa = 1;
                rand = !rand;
                continue;
            }
        }
        else
        {
            remiza_propusa = 0;
        }

        int x1, y1, x2, y2;
        if (sscanf(input, "%c%d %c%d", &y1, &x1, &y2, &x2) != 4) {
            printf("Format invalid. Folosește formatul A2 A4 sau 'draw'.\n");
            continue;
        }

        x1 = 8 - x1;
        y1 = y1 - 'A';
        x2 = 8 - x2;
        y2 = y2 - 'A';

        if (!mutare_legala(*board, x1, y1, x2, y2, rand)) {
            printf("Mutare ilegală.\n");
            continue;
        }

        if (pion_promovat(board, x1, y1, x2, rand)) 
        {
            printf("Promovare! Introdu piesa dorită (Q, R, B, N): ");
            char promovare;
            if (este_server)
                scanf(" %c", &promovare);
            else
                recv(socket_id, &promovare, 1, 0); // exemplu generic

            promoveaza_pion(board, x2, y2, promovare, rand);
        }
        else 
        {
            muta_piesa(board, x1, y1, x2, y2);
        }

        afiseaza_tabla(*board);
        rand = !rand;
    }
}

int server_mode() 
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("socket");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Aștept un jucător la portul %d...\n", PORT);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) 
    {
        perror("accept");
        exit(1);
    }

    printf("Conectat la client!\n");
    close(server_fd);
    return client_fd;
}

int client_mode(const char *ip) 
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("socket");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        printf("Adresă IP invalidă\n");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
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
    getchar();

    int sockfd;
    if (opt == 1) 
    {
        sockfd = server_mode();
        ruleaza_joc(sockfd, 1); // server = alb
    }
    else
        {
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
