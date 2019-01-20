#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define CCCI_MAX_LENGTH (3308)
#define PAYLOAD_MAX_LENGTH (CCCI_MAX_LENGTH - sizeof(aplogger_ctrl_msg_header_t))

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

#define APLOGGER_CTRL_MSG_FILE "socket_file"

// 返回值： 0:发送成功； -1:发送失败，服务器端没有启动
int aplogger_ctrl_msg_interface(int msg_id, char *module_name, char *data, int msg_len) {
    aplogger_ctrl_msg_t msg;
    struct sockaddr_un  client_addr;
    int                 sock_fd;
    client_addr.sun_family = AF_UNIX;

    // 填充aplogger_ctrl_msg_header_t
    strcpy(client_addr.sun_path, APLOGGER_CTRL_MSG_FILE);
    strcpy(msg.header.module_name, module_name);
    memcpy(msg.data, data, msg_len);
    msg.header.data_len = msg_len;
    msg.header.msg_id   = msg_id;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("aplogger socket create fail\n");
        return -2;
    }
    if (connect(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        printf("server did not start\n");
        return -1;
    }
    send(sock_fd, &msg, sizeof(aplogger_ctrl_msg_t), 0);
    close(sock_fd);
    return 0;
}
