#include "EcuM.h"
#include "stm32f10x.h"  /* CMSIS: SystemInit(), SCB->VTOR */

/* =========================
 *   C ấ u   h ì n h   m ặ c   đ ị n h
 * =========================
 * Vector table ở Flash base 0x08000000 (F103), chủ động relocate VTOR để rõ ràng,
 * không làm Early Pin Safe mặc định (để dự án tự quyết).
 */
const EcuM_ConfigType EcuM_Config = {
    /* vectorTableBase          */ 0x08000000u,
    /* relocateVector           */ TRUE,
    /* earlyPinSafeInitEnable   */ FALSE
};

/* Trạng thái nội bộ (đơn giản) */
static boolean EcuM_Initialized = FALSE;

/* =========================
 *   H o o k s   y ế u  (có thể override ở dự án)
 * ========================= */
__attribute__((weak)) void EcuM_EarlyPinSafeInit(void)
{
    /* Mặc định: không làm gì.
       Gợi ý override: cấu hình clock GPIO + set pin ở mức an toàn:
         - Tắt driver mạnh (MOSFET/relay), ngắt đầu ra.
         - Kéo các chân input về pull hợp lý để tránh trôi.
       Tránh dùng driver MCAL phức tạp ở đây; chỉ dùng mức thanh ghi/CMSIS tối thiểu nếu cần. */
}

__attribute__((weak)) void EcuM_StartupTwo_Hook(void)
{
    /* Mặc định: không làm gì.
       Có thể bật thêm trap/usage fault, prefetch, hoặc đo tần số, v.v. nếu thật sự cần ở giai đoạn sớm. */
}

/* =========================
 *   P h a   1 :   E c u M _ I n i t
 * ========================= */
void EcuM_Init(void)
{
    /* Đặt trạng thái init; gọi pin-safe sớm nếu cấu hình yêu cầu */
    if (EcuM_Config.earlyPinSafeInitEnable == TRUE) {
        EcuM_EarlyPinSafeInit();
    }
    EcuM_Initialized = TRUE;
}

/* =========================
 *   P h a   2 :   E c u M _ S t a r t u p T w o
 * =========================
 * - Gọi SystemInit() từ CMSIS: cấu hình RCC/PLL/Flash latency theo system_stm32f10x.c
 * - Relocate bảng vector (SCB->VTOR) nếu cấu hình yêu cầu
 * - Gọi hook bổ sung nếu dự án cần
 */
void EcuM_StartupTwo(void)
{
    /* Đảm bảo đã qua pha 1; nếu chưa, thực hiện luôn */
    if (EcuM_Initialized == FALSE) {
        EcuM_Init();
    }

    /* Clock/flash/vector mặc định theo CMSIS */
    SystemInit();

    /* Chủ động đặt VTOR nếu được yêu cầu (rõ ràng khi có bootloader/dual-bank) */
    if (EcuM_Config.relocateVector == TRUE) {
        SCB->VTOR = (uint32_t)EcuM_Config.vectorTableBase;
        __DSB();
        __ISB();
    }

    /* Hook bổ sung cho dự án (nếu cần) */
    EcuM_StartupTwo_Hook();
}
