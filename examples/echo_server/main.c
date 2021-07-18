#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <glib.h>

#include <cucoroutine/all.h>

struct serve_args {
    int32_t fd;
    uint32_t socket_len;
    struct sockaddr_in addr;
};

void serve(void *arg_) {
    struct serve_args *args = arg_;
    cu_add_fd(args->fd, CU_IN | CU_OUT, default_reactor);
    char *addr = inet_ntoa(args->addr.sin_addr);
    int32_t port = args->addr.sin_port;
    for (int i = 0; i < 10; ++i) {
        char client_message[128];
        int s_len = cu_read(args->fd, client_message, 127, default_reactor);
        client_message[s_len] = 0;
        cu_write(args->fd, client_message, s_len, default_reactor);
        printf("serve fd: %d, %s:%d read: %s", args->fd, addr, port, client_message);
    }
    cu_close(args->fd, default_reactor);
    free(args);
}

void server(void *arg) {
    int ss = (int)arg;
    cu_add_fd(ss, CU_IN, default_reactor);
    for (int i = 0; i < 5; ++i) {
        struct sockaddr_in client_addr;
        uint32_t socket_len;
        int client_fd = cu_accept(ss, (struct sockaddr *)&client_addr, &socket_len, default_reactor);
        int flags = fcntl(client_fd, F_GETFL);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        printf("accepted\n");
        struct serve_args *args = malloc(sizeof(struct serve_args));
        args->socket_len = socket_len;
        args->addr = client_addr;
        args->fd = client_fd;
        cu_reactor_make_coro(default_reactor, serve, args);
    }
}

int main() {
    int ss = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    struct sockaddr_in local;
    inet_aton("127.0.0.1", &local.sin_addr);
    local.sin_port = htons(1234);
    local.sin_family = AF_INET;
    g_assert(bind(ss, (struct sockaddr *)&local, sizeof(local)) == 0);
    listen(ss, 100);

    cu_reactor_init(&default_reactor);
    cu_reactor_make_coro(default_reactor, server, (void *)ss);
    cu_reactor_run(default_reactor);
    return 0;
}