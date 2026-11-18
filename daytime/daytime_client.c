// daytime/daytime_client.c — клиент UDP daytime (RFC 867)
// Отправляем пустую датаграмму на сервер, получаем строку с датой/временем.
// Использование: ./daytime_client <host> [port] (порт по умолчанию 13)
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

#define BUF_SIZE    1024
#define TIMEOUT_SEC 5

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <host> [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *host = argv[1];
    const char *port = (argc == 3) ? argv[2] : "13";

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    // Таймаут на приём, чтобы программа не зависла, если сервер молчит
    struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Отправляем пустую датаграмму — по RFC 867
    const char msg = '\0';
    if (sendto(sockfd, &msg, 0, 0, res->ai_addr, res->ai_addrlen) < 0) {
        perror("sendto");
        close(sockfd);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }

    // Принимаем ответ; sizeof(buf)-1 оставляем под '\0'
    char buf[BUF_SIZE];
    struct sockaddr_storage from;
    socklen_t fromlen = sizeof(from);
    ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&from, &fromlen);
    if (n < 0) {
        perror("recvfrom");
        close(sockfd);
        freeaddrinfo(res);
        return EXIT_FAILURE;
    }
    buf[n] = '\0';

    printf("%s", buf);
    if (n == 0 || buf[n - 1] != '\n')
        printf("\n");

    close(sockfd);
    freeaddrinfo(res);
    return EXIT_SUCCESS;
}
