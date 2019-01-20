#include <errno.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define CCCI_MAX_LENGTH (3308)
#define PAYLOAD_MAX_LENGTH (CCCI_MAX_LENGTH - sizeof(aplogger_ctrl_msg_header_t))

#define NETLINK_TEST 30
#define MAX_PLOAD 125

typedef struct aplogger_ctrl_msg_header {
    unsigned int magic_num;
    unsigned int ver;
    int          src_module;
    int          dest_module;
    char         module_name[64];
    int          msg_id;
    int          msg_type;
    char         msg_name[64];
    int          fragment_cnt;
    int          reserverd[2];
    int          data_len;
} aplogger_ctrl_msg_header_t;

typedef struct aplogger_ctrl_msg {
    aplogger_ctrl_msg_header_t header;
    char                       data[PAYLOAD_MAX_LENGTH];
} aplogger_ctrl_msg_t;

typedef struct _user_msg_info {
    struct nlmsghdr     hdr;
    aplogger_ctrl_msg_t msg;
} user_msg_info;

#define MAX_CONNECT_NUM 2
#define APLOGGER_CTRL_MSG_FILE "socket_file"
int                 g_ctrl_msg_socket_fd  = 0;
int                 g_ctrl_msg_netlink_fd = 0;
aplogger_ctrl_msg_t msg;
user_msg_info       u_info;

int aplogger_ctrl_msg_socket_init() {
    int                server_fd;
    struct sockaddr_un server_addr;
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("Request socket failed!\n");
        return -1;
    }
    server_addr.sun_family = AF_UNIX;
    unlink(APLOGGER_CTRL_MSG_FILE);
    strcpy(server_addr.sun_path, APLOGGER_CTRL_MSG_FILE);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind failed!\n");
        return -1;
    }
    if (listen(server_fd, MAX_CONNECT_NUM) < 0) {
        printf("listen failed!\n");
        return -1;
    }
    g_ctrl_msg_socket_fd = server_fd;
}

void *aplogger_ctrl_msg_pthread(void *arg) {
    pthread_detach(pthread_self());
    int    client_fd;
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(g_ctrl_msg_socket_fd, &readset);
    FD_SET(g_ctrl_msg_netlink_fd, &readset);
    int max_fd = (g_ctrl_msg_socket_fd > g_ctrl_msg_netlink_fd ? g_ctrl_msg_socket_fd : g_ctrl_msg_netlink_fd);
    while (1) {
        int ret = select(max_fd + 1, &readset, NULL, NULL, NULL);
        if (ret > 0) {
            if (FD_ISSET(g_ctrl_msg_socket_fd, &readset)) {
                client_fd = accept(g_ctrl_msg_socket_fd, NULL, NULL);
                if (client_fd < 0) {
                    printf("accept failed\n");
                    return NULL;
                }
                int ret = recv(client_fd, &msg, sizeof(aplogger_ctrl_msg_t), 0);
                if (ret < 0) { printf("recv failed\n"); }
                printf("data is %s\n", msg.data);
                printf("msg_name is %s\n", msg.header.module_name);
                close(client_fd);
            } else if (FD_ISSET(g_ctrl_msg_netlink_fd, &readset)) {
                struct sockaddr_nl daddr;
                socklen_t          len = sizeof(struct sockaddr_nl);
                ret =
                    recvfrom(g_ctrl_msg_netlink_fd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
                if (!ret) {
                    perror("recv form kernel error\n");
                    close(g_ctrl_msg_netlink_fd);
                    exit(-1);
                }

                printf("from kernel:%s\n", u_info.msg.data);
            } else {
                continue;
            }
        } else if (0 == ret) {
            printf("timeout\n");
        } else {
            printf("error\n");
        }
    }
    close(g_ctrl_msg_socket_fd);
    close(g_ctrl_msg_netlink_fd);
}

int aplogger_ctrl_msg_thread_create(void) {
    pthread_t thread;
    int       ret;
    ret = pthread_create(&thread, NULL, aplogger_ctrl_msg_pthread, NULL);
    if (ret != 0) {
        printf("Create thread error!\n");
        return -1;
    }
    return 0;
}
int aplogger_ctrl_msg_netlink_init() {
    int socket_fd;
    int ret;

    struct nlmsghdr *  nlh = NULL;
    struct sockaddr_nl saddr;

    /* 创建NETLINK socket */
    socket_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if (socket_fd == -1) {
        perror("create socket error\n");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK;  // AF_NETLINK
    saddr.nl_pid    = 100;         //端口号(port ID)
    saddr.nl_groups = 0;
    if (bind(socket_fd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        perror("bind() error\n");
        close(socket_fd);
        return -1;
    }
    memset(&u_info, 0, sizeof(u_info));
    g_ctrl_msg_netlink_fd = socket_fd;
    return 0;
}

int main(int argc, const char *argv[]) {
    aplogger_ctrl_msg_socket_init();
    aplogger_ctrl_msg_netlink_init();
    aplogger_ctrl_msg_thread_create();

    while (1)
        ;
}
