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
#include "aplogger.h"

int main(int argc, const char *argv[]) {
    char name[64] = "1111";
    char s[3308]  = "this is a test!";
    aplogger_ctrl_msg_interface(1, name, s, strlen(s));
    return 0;
}
