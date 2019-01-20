

#include <linux/module.h>

int aplogger_ctrl_msg_interface(int msg_id, char *module_name, char *data, int msg_len);

#define CCCI_MAX_LENGTH (3308)
#define PAYLOAD_MAX_LENGTH (CCCI_MAX_LENGTH - sizeof(aplogger_ctrl_msg_header_t))
typedef struct aplogger_ctrl_msg_header
{
    unsigned int magic_num;
    unsigned int ver;
    int src_module;
    int dest_module;
    char module_name[64];
    int msg_id;
    int msg_type;
    char msg_name[64];
    int fragment_cnt;
    int reserverd[2];
    int data_len;
} aplogger_ctrl_msg_header_t;

typedef struct aplogger_ctrl_msg
{
    aplogger_ctrl_msg_header_t header;
    char data[PAYLOAD_MAX_LENGTH];
} aplogger_ctrl_msg_t;

static int test_init(void)
{
    printk("test init!!!\n");
    char cc[64] = "this is a test!";
    aplogger_ctrl_msg_interface(1, "1111", cc, strlen(cc));
    return 0;
}

static void test_exit(void)
{
    printk("test exit!!!\n");
}

MODULE_LICENSE("GPL");
module_init(test_init);
module_exit(test_exit);
