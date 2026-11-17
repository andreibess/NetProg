// echo/echo_client.c — клиент TCP echo (RFC 862)
// Читаем строки из stdin, отправляем серверу, читаем ровно столько же обратно.
// Использование: ./echo_client <host> [port] (порт по умолчанию 7)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <host> [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    const char *port = (argc == 3) ? argv[2] : "7";

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    /* Пробуем подключиться по адресам из списка, пока не получится */
    int sockfd = -1;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);
    if (sockfd < 0) {
        perror("connect");
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        size_t len = strlen(buf);

        ssize_t sent = send(sockfd, buf, len, 0);
        if (sent < 0) {
            perror("send");
            break;
        }

        /* TCP — поток байт: recv может вернуть меньше, поэтому читаем в цикле */
        size_t total = 0;
        while (total < (size_t)sent) {
            ssize_t n = recv(sockfd, buf + total, (size_t)sent - total, 0);
            if (n <= 0) {
                if (n < 0) perror("recv");
                else       fprintf(stderr, "Соединение закрыто сервером\n");
                close(sockfd);
                return EXIT_FAILURE;
            }
            total += (size_t)n;
        }

        fwrite(buf, 1, total, stdout);
        fflush(stdout);
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
