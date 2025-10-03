/**********************************************************
 * @file Adc.h
 * @brief Khai báo các hàm và kiểu dữ liệu liên quan đến điều khiển ADC
 * @details File này cung cấp các khai báo và định nghĩa cần thiết cho việc khởi tạo,
 *          chuyển đổi, đọc kết quả và quản lý trạng thái của ADC trên vi điều khiển.
 * @version 1.0
 * @date 2024-10-03
 * @author HALA Academy
 **********************************************************/

#ifndef ADC_H
#define ADC_H

#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "Std_Types.h"

/* ============================= */
/* ========== MACRO =========== */
/* ============================= */

#define ADC_MAX_HW 1
#define ADC_MAX_GROUPS 2
#define ADC_BUFFER_SIZE_GROUP_1 4
#define ADC_BUFFER_SIZE_GROUP_2 4

#define ADC_GROUP_1 0
#define ADC_GROUP_2 1

/* ============================= */
/* ==== ENUM & STRUCT TYPE ==== */
/* ============================= */

typedef uint32_t Adc_ConversionTimeType;
typedef uint32_t Adc_ChannelType;
typedef uint32_t Adc_GroupType;
typedef uint16_t Adc_ValueGroupType;
typedef uint8_t Adc_ResolutionType;
typedef uint8_t Adc_GroupPriorityType;
typedef uint8_t Adc_StreamNumberSampleType;
typedef uint8_t Adc_SamplingTimeType;

typedef enum
{
    ADC_GROUP_REPL_ABORT_RESTART = 0,
    ADC_GROUP_REPL_SUSPEND_RESUME = 1,
} Adc_GroupReplacementType;

typedef enum
{
    ADC_HW_TRIG_RISING_EDGE = 0,
    ADC_HW_TRIG_FALLING_EDGE = 1,
    ADC_HW_TRIG_BOTH_EDGES = 2
} Adc_HwTriggerSignalType;

typedef enum
{
    ADC_GROUP_ACCESS_MODE_SINGLE = 0,
    ADC_GROUP_ACCESS_MODE_STREAMING = 1
} Adc_GroupAccessModeType;
typedef enum
{
    ADC_NOTIFICATION_DISABLED = 0,
    ADC_NOTIFICATION_ENABLED = 1
} Adc_NotificationType;
typedef enum
{
    DMA_ADC_NOTIFICATION_DISABLED = 0,
    DMA_ADC_NOTIFICATION_ENABLED = 1
} Dma_Adc_NotificationType;
typedef enum
{
    ADC_TRIGGER_SOFTWARE = 0,
    ADC_TRIGGER_HARDWARE = 1
} Adc_TriggerSourceType;

typedef enum
{
    ADC_CONV_MODE_SINGLE = 0,
    ADC_CONV_MODE_CONTINUOUS = 1
} Adc_GroupConvModeType;

typedef enum
{
    ADC_RESULT_ALIGNMENT_RIGHT = 0,
    ADC_RESULT_ALIGNMENT_LEFT = 1
} Adc_ResultAlignmentType;

typedef enum
{
    ADC_INSTANCE_1 = 0,
    ADC_INSTANCE_2 = 1
} Adc_InstanceType;

typedef enum
{
    ADC_IDLE = 0,
    ADC_BUSY,
    ADC_COMPLETED,
    ADC_ERROR
} Adc_StatusType;

typedef enum
{
    ADC_STREAM_BUFFER_LINEAR = 0x00,
    ADC_STREAM_BUFFER_CIRCULAR = 0x01
} Adc_StreamBufferModeType;

typedef struct
{
    Adc_ChannelType Channel;
    Adc_SamplingTimeType SamplingTime;
    uint8_t Rank;
} Adc_ChannelConfigType;

typedef struct
{
    uint32_t ClockPrescaler;
    Adc_ResolutionType Resolution;
    Adc_GroupConvModeType ConversionMode;
    Adc_TriggerSourceType TriggerSource;
    Adc_NotificationType NotificationEnabled;
    uint8_t NumChannels;
    Adc_InstanceType AdcInstance;
    Adc_ChannelConfigType Channels[16];
    Adc_ResultAlignmentType ResultAlignment;
    void (*InitCallback)(void);
} Adc_ConfigType;

typedef struct
{
    Adc_GroupType id;                              /**< ID của nhóm kênh ADC */
    Adc_InstanceType AdcInstance;                  /**< ID cơ bản của mô-đun ADC */
    Adc_ChannelType Channels[16];                  /**< Danh sách kênh trong nhóm */
    Adc_GroupPriorityType Priority;                /**< Mức độ ưu tiên của nhóm */
    uint8_t NumChannels;                           /**< Số kênh trong nhóm */
    Adc_StatusType Status;                         /**< Trạng thái hiện tại của nhóm */
    Adc_ValueGroupType *Result;                    /**< Con trỏ buffer kết quả */
    uint8_t Adc_StreamEnableType;                  /**< Có bật DMA hay không (1 = ENABLE) */
    uint8_t Adc_StreamBufferSize;                  /**< Kích thước bộ đệm */
    Adc_StreamBufferModeType Adc_StreamBufferMode; /**< Kiểu hoạt động stream */
    Dma_Adc_NotificationType Dma_Notification;     /**< Có bật thông báo DMA hay không */
} Adc_GroupDefType;

typedef enum
{
    ADC_POWERSTATE_OFF,
    ADC_POWERSTATE_ON,
    ADC_POWERSTATE_LOWPOWER
} Adc_PowerStateType;
/* ============================= */
/* ========== API ============= */
/* ============================= */

/**********************************************************
 * @brief Khởi tạo mô-đun ADC.
 * @details Hàm này khởi tạo các đơn vị phần cứng ADC và driver theo cấu hình được chỉ định.
 * @param[in] ConfigPtr Con trỏ tới tập cấu hình cho mô-đun ADC.
 * @return None
 **********************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr);

/**********************************************************
 * @brief Thiết lập bộ đệm kết quả cho nhóm ADC.
 * @details Hàm này thiết lập địa chỉ bộ đệm kết quả cho các kênh trong nhóm.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[in] DataBufferPtr Con trỏ tới bộ đệm kết quả.
 * @return Std_ReturnType Trả về E_OK nếu thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

/**********************************************************
 * @brief Hủy khởi tạo mô-đun ADC.
 * @details Đưa tất cả các đơn vị phần cứng ADC về trạng thái chưa khởi tạo.
 * @return None
 **********************************************************/
void Adc_DeInit(void);

/**********************************************************
 * @brief Bắt đầu chuyển đổi nhóm kênh ADC.
 * @details Bắt đầu quá trình chuyển đổi cho tất cả các kênh trong nhóm.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_StartGroupConversion(Adc_GroupType Group);

/**********************************************************
 * @brief Dừng quá trình chuyển đổi nhóm ADC.
 * @details Dừng quá trình chuyển đổi của nhóm kênh ADC được chỉ định.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_StopGroupConversion(Adc_GroupType Group);

/**********************************************************
 * @brief Đọc kết quả chuyển đổi của nhóm ADC.
 * @details Trả về giá trị kết quả của các kênh trong nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[out] DataBufferPtr Con trỏ tới bộ đệm lưu trữ kết quả.
 * @return Std_ReturnType Trả về E_OK nếu đọc thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

/**********************************************************
 * @brief Bật kích hoạt phần cứng cho nhóm ADC.
 * @details Kích hoạt nguồn phần cứng cho nhóm kênh được chỉ định.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**********************************************************
 * @brief Tắt kích hoạt phần cứng cho nhóm ADC.
 * @details Vô hiệu hóa nguồn phần cứng cho nhóm kênh được chỉ định.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**********************************************************
 * @brief Bật thông báo cho nhóm ADC.
 * @details Kích hoạt thông báo nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_EnableGroupNotification(Adc_GroupType Group);

/**********************************************************
 * @brief Tắt thông báo cho nhóm ADC.
 * @details Vô hiệu hóa thông báo nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @return None
 **********************************************************/
void Adc_DisableGroupNotification(Adc_GroupType Group);

/**********************************************************
 * @brief Lấy trạng thái nhóm ADC.
 * @details Trả về trạng thái hiện tại của nhóm ADC (IDLE, BUSY, hoặc COMPLETED).
 * @param[in] Group ID của nhóm kênh ADC.
 * @return Adc_StatusType Trạng thái của nhóm kênh ADC.
 **********************************************************/
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**********************************************************
 * @brief Lấy con trỏ cuối cùng của buffer trong chế độ streaming.
 * @details Trả về địa chỉ của mẫu cuối cùng được ghi vào bộ đệm của nhóm kênh.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[out] PtrToSampleAddress Con trỏ tới địa chỉ của mẫu cuối cùng.
 * @return Std_ReturnType Trả về E_OK nếu thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PtrToSampleAddress);

/**********************************************************
 * @brief Lấy thông tin phiên bản của mô-đun ADC.
 * @details Hàm này trả về thông tin phiên bản của mô-đun ADC.
 * @param[out] VersionInfo Con trỏ tới cấu trúc chứa thông tin phiên bản.
 * @return None
 **********************************************************/

void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/**********************************************************
 * @brief Thiết lập trạng thái nguồn cho nhóm ADC.
 * @details Hàm này thiết lập trạng thái nguồn (bật, tắt, tiết kiệm năng lượng) cho nhóm ADC.
 * @param[in] Group ID của nhóm kênh ADC.
 * @param[in] PowerState Trạng thái nguồn cần thiết lập.
 * @return Std_ReturnType Trả về E_OK nếu thành công, E_NOT_OK nếu thất bại.
 **********************************************************/
Std_ReturnType Adc_SetPowerState(Adc_GroupType Group, Adc_PowerStateType PowerState);

#endif // ADC_H
