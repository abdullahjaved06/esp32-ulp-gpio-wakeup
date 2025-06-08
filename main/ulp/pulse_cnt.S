
/* ULP : rising edge detection and wakeup */

#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "soc/sens_reg.h"

    .bss
    /* Store previous input state */
    .global prev_state
prev_state:
    .long 0

    /* RTC IO number used to sample the input signal. Set by main program. */
    .global io_number
io_number:
    .long 0

    .text
    .global entry
entry:
    /* Load io_number */
    move r3, io_number
    ld r3, r3, 0

    /* Read RTC IO value */
    move r0, r3
    jumpr read_io_high, 16, ge

    READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S, 16)
    rsh r0, r0, r3
    jump read_done

read_io_high:
    READ_RTC_REG(RTC_GPIO_IN_REG, RTC_GPIO_IN_NEXT_S + 16, 2)
    sub r3, r3, 16
    rsh r0, r0, r3

read_done:
    and r0, r0, 1

    /* Load previous state */
    move r2, prev_state
    ld r2, r2, 0

    /* Detect rising edge: prev_state == 0 && r0 == 1 */
    move r1, r2
    add r1, r1, 0
    jump check_r0, eq  /* If prev_state == 0, check r0 */
    jump not_rising    /* Otherwise, not rising */

check_r0:
    move r1, r0
    sub r1, r1, 1
    jump rising, eq    /* If r0 == 1, rising */
    jump not_rising    /* Otherwise, not rising */

rising:
    /* Rising edge detected, update prev_state and wake up */
    move r1, prev_state
    move r2, 1
    st r2, r1, 0
    jump wake_up

not_rising:
    /* Update prev_state */
    move r1, prev_state
    st r0, r1, 0
    halt

    .global wake_up