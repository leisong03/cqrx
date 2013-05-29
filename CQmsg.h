#include <stdint.h>
#define MAX_CQ_LEN 8
typedef struct {
    uint16_t Seq;
    uint8_t ID;
    uint8_t CQLEN;
    uint8_t symbol[MAX_CQ_LEN];
    uint16_t counter[MAX_CQ_LEN];
} cqmsg;
