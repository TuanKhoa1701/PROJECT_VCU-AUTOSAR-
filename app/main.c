#include "Os.h"
#include "Rte.h"
/* RTE scheduler 10ms */
extern void Rte_Tick10ms(void);

int main(void)
{
    /* Bring-up RTE (không bật timing vội) */
    (void)Rte_Start();
    Rte_Init_Core();

    /* Gắn callback 10ms: OS sẽ gọi Rte_Tick10ms mỗi kỳ */
    Os_ConnectAlarm(ALARM_10MS, Rte_Tick10ms);

    /* Bật OS: SysTick 1ms + ALARM_10MS */
    (void)StartOS(OSDEFAULTAPPMODE);

    /* Cho phép timing event của RTE */
    Rte_StartTiming();

    for (;;)
        ;
    //__WFI(); /* Idle; mọi việc chạy theo ngắt */
}
