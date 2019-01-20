#include <errno.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NETLINK_TEST 30
#define MAX_PLOAD 125

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

typedef struct _user_msg_info {
    struct nlmsghdr     hdr;
    aplogger_ctrl_msg_t msg;
} user_msg_info;

int main(int argc, char **argv) {
    int                skfd;
    int                ret;
    user_msg_info      u_info;
    socklen_t          len;
    struct nlmsghdr *  nlh = NULL;
    struct sockaddr_nl saddr, daddr;
    char *             umsg = "hello netlink!!";

    /* 创建NETLINK socket */
    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if (skfd == -1) {
        perror("create socket error\n");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK;  // AF_NETLINK
    saddr.nl_pid    = 100;         //端口号(port ID)
    saddr.nl_groups = 0;
    if (bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        perror("bind() error\n");
        close(skfd);
        return -1;
    }

    memset(&u_info, 0, sizeof(u_info));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
    if (!ret) {
        perror("recv form kernel error\n");
        close(skfd);
        exit(-1);
    }

    printf("from kernel:%s\n", u_info.msg.data);
    close(skfd);
    // free((void *)nlh);
    return 0;
}
