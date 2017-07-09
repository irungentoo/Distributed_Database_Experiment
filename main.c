
#include <uv.h>
#include <stdlib.h>
#include <string.h>

#define TCP_PORT 6177
#define TCP_BACKLOG 128

static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    if (nread < 0) {
        uv_close((uv_handle_t *) stream, NULL);
    }

    if (nread > 0) {
        printf("%u, read: %zd\n", buf->base[0], nread);
    }

    free(buf->base);
}

void connection_cb(uv_stream_t* server, int status)
{
    if (status) {
        fprintf(stderr, "connection error: %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(server->loop, client);
    int ret = uv_accept(server, (uv_stream_t *) client);

    if (ret) {
        fprintf(stderr, "uv_accept error: %s\n", uv_strerror(ret));
        return;
    }

    uv_read_start((uv_stream_t *) client, alloc_cb, read_cb);
}

int main(int argc, char **argv)
{
    uv_loop_t *loop = uv_default_loop();

    uv_tcp_t server4;
    uv_tcp_init(loop, &server4);

    struct sockaddr_in bind_addr4 = {};
    uv_ip4_addr("0.0.0.0", 6177, &bind_addr4);
    uv_tcp_bind(&server4, (const struct sockaddr*) &bind_addr4, 0);

    int ret = uv_listen((uv_stream_t *)&server4, TCP_BACKLOG, connection_cb);

    if (ret) {
        fprintf(stderr, "uv_listen ipv4 error: %s\n", uv_strerror(ret));
        return 1;
    }

    uv_tcp_t server6;
    uv_tcp_init(loop, &server6);

    struct sockaddr_in6 bind_addr6 = {};
    uv_ip6_addr("::1", 6177, &bind_addr6);
    uv_tcp_bind(&server6, (const struct sockaddr*) &bind_addr6, UV_TCP_IPV6ONLY);

    ret = uv_listen((uv_stream_t *)&server6, TCP_BACKLOG, connection_cb);

    if (ret) {
        fprintf(stderr, "uv_listen ipv6 error: %s\n", uv_strerror(ret));
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
