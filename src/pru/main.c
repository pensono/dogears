#include <stdint.h>
#include <pru_cfg.h>
#include <pru_ctrl.h>
#include "resource_table.h"

volatile register uint32_t __R30;

extern void START(void);

void main(void) {
    START();
}
