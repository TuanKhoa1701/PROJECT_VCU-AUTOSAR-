#ifndef RTE_H
#define RTE_H

/* ================================
 * AUTOSAR RTE – Core Public Header
 * - MUST include Std_Types.h
 * - SHOULD include ComStack_Types.h (nếu dùng COM/PduR/CanIf sau này)
 * ================================ */
#include "Std_Types.h"
// #include "ComStack_Types.h"   /* Nếu chưa dùng COM stack, vẫn có thể để sẵn include này. */
#include "Rte_Types.h" /* Kiểu dùng chung cho mọi SWC */

#ifdef __cplusplus
extern "C"
{
#endif

/* ================================
 * Version & Identification (gợi ý)
 * ================================ */
#define RTE_VENDOR_ID (0u)
#define RTE_MODULE_ID (1u)
#define RTE_AR_RELEASE_MAJOR_VERSION (4u)
#define RTE_AR_RELEASE_MINOR_VERSION (9u)
#define RTE_AR_RELEASE_REVISION_VERSION (0u)
#define RTE_SW_MAJOR_VERSION (0u)
#define RTE_SW_MINOR_VERSION (1u)
#define RTE_SW_PATCH_VERSION (0u)

/* ================================
 * Generic RTE status codes (tối giản)
 * ================================ */
#ifndef RTE_E_OK
#define RTE_E_OK ((Std_ReturnType)E_OK)
#endif
#ifndef RTE_E_LIMIT
#define RTE_E_LIMIT ((Std_ReturnType)3u) /* out of range / queue full */
#endif
#ifndef RTE_E_NO_DATA
#define RTE_E_NO_DATA ((Std_ReturnType)4u) /* nothing available */
#endif
#ifndef RTE_E_INVALID
#define RTE_E_INVALID ((Std_ReturnType)5u) /* invalid handle/port */
#endif
#ifndef RTE_E_TIMEOUT
#define RTE_E_TIMEOUT ((Std_ReturnType)6u)
#endif

/* ================================
 * Inline helper (option)
 * ================================ */
#ifndef RTE_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define RTE_INLINE static inline
#else
#define RTE_INLINE static
#endif
#endif

/* ================================
 * Exclusive Area hooks (tối giản)
 * - Thực tế, các API này sẽ được sinh riêng cho từng Exclusive Area của SWC
 * - Ở header chung chỉ gợi ý macro fallback (no-op)
 * ================================ */
#ifndef RTE_ENTER_EXCLUSIVE
#define RTE_ENTER_EXCLUSIVE() ((void)0)
#endif
#ifndef RTE_EXIT_EXCLUSIVE
#define RTE_EXIT_EXCLUSIVE() ((void)0)
#endif

/* ================================
 * Lifecycle prototypes nằm trong Rte_Main.h
 * (được "available via Rte.h")
 * ================================ */
#include "Rte_Main.h"

    /* ================================
     * Ghi chú:
     * - Không khai báo API port-specific (Rte_Read/Write/Call/Mode...) trong Rte.h.
     *   Chúng được khai báo ở các Application Header: Rte_Swc_*.h
     * ================================ */

#ifdef __cplusplus
}
#endif

#endif /* RTE_H */
