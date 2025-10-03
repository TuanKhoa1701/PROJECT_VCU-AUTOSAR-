#ifndef ECUM_H
#define ECUM_H

#include "Std_Types.h"

/* Cấu hình tối giản cho EcuM pha khởi động sớm.
   - vectorTableBase: địa chỉ bảng vector muốn dùng (VD: 0x08000000 Flash, hoặc RAM khi bootloader).
   - relocateVector  : TRUE → ghi SCB->VTOR về vectorTableBase (rõ ràng, tránh phụ thuộc mặc định).
   - earlyPinSafeInitEnable: TRUE → gọi EcuM_EarlyPinSafeInit() để đưa chân về trạng thái an toàn sớm.
*/
typedef struct
{
   uint32_t vectorTableBase;
   boolean relocateVector;
   boolean earlyPinSafeInitEnable;
} EcuM_ConfigType;

/* Cấu hình mặc định (định nghĩa trong EcuM.c). Có thể override bằng cách link đối tượng khác. */
extern const EcuM_ConfigType EcuM_Config;

/* Pha 1: tải cấu hình/chuẩn bị khởi động (rút gọn, đặt trạng thái & optional pin-safe). */
void EcuM_Init(void);

/* Pha 2: khởi tạo mức thấp (SystemInit: clock/flash/vector), tuỳ chọn relocate VTOR. */
void EcuM_StartupTwo(void);

/* Hooks yếu (có thể override tại dự án):
   - EcuM_EarlyPinSafeInit(): đưa IO về trạng thái an toàn trước khi các driver hoạt động.
   - EcuM_StartupTwo_Hook(): thao tác bổ sung sau SystemInit/VTOR (ví dụ bật SysTick source HCLK sớm, nếu cần).
*/
void EcuM_EarlyPinSafeInit(void);
void EcuM_StartupTwo_Hook(void);

#endif /* ECUM_H */
