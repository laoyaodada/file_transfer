#include <linux/init.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <net/sock.h>

#define NETLINK_TEST 30
#define USER_PORT 100

extern struct net init_net;
struct sock *     nlsk = NULL;

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

int aplogger_ctrl_msg_interface(int msg_id, char *module_name, char *data, int msg_len) {
    struct sk_buff *    nl_skb;
    struct nlmsghdr *   nlh;
    aplogger_ctrl_msg_t msg;
    int                 ret;

    /* 创建sk_buff 空间 */
    nl_skb = nlmsg_new(sizeof(aplogger_ctrl_msg_t), GFP_ATOMIC);
    if (!nl_skb) {
        printk("netlink alloc failure\n");
        return -1;
    }

    msg.header.msg_id = msg_id;
    strcpy(msg.header.module_name, module_name);
    msg.header.data_len = msg_len;
    memcpy(msg.data, data, msg_len);
    /* 设置netlink消息头部 */
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, sizeof(aplogger_ctrl_msg_t), 0);
    if (nlh == NULL) {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }

    /* 拷贝数据发送 */
    memcpy(nlmsg_data(nlh), &msg, sizeof(aplogger_ctrl_msg_t));
    // netlink_unicast会自动释放nl_skb
    ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);
    return ret;
}

static int aplogger_netlink_init(void) {
    /* create netlink socket */
    nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_TEST, NULL);
    if (nlsk == NULL) {
        printk("netlink_kernel_create error !\n");
        return -1;
    }
    printk("netlink_init\n");
    return 0;
}

static void aplogger_netlink_exit(void) {
    if (nlsk) {
        netlink_kernel_release(nlsk); /* release ..*/
        nlsk = NULL;
    }
    printk("netlink_exit!\n");
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(aplogger_ctrl_msg_interface);
module_init(aplogger_netlink_init);
module_exit(aplogger_netlink_exit);
