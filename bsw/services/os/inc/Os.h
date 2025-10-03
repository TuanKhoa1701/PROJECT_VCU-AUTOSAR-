/**********************************************************
 * @file    Os.h
 * @brief   API công khai của lớp OS (subset AUTOSAR/OSEK) cho STM32F103
 * @details Hỗ trợ vận hành thời gian thực nhẹ cho Cortex-M3:
 *          - SysTick 1ms (tăng TickType, alarm/schedule)
 *          - PendSV cho chuyển ngữ cảnh
 *          - PSP cho THREAD mode, MSP cho HANDLER mode
 *
 *          Nhóm API:
 *            1) Lifecycle: StartOS, ShutdownOS
 *            2) Task     : ActivateTask, TerminateTask, OS_Yield, OS_Delay
 *            3) Event    : WaitEvent, SetEvent, GetEvent, ClearEvent (Extended Task)
 *            4) Alarm    : SetRelAlarm/CancelAlarm (ms)
 *            5) Counter  : OS_TickCount()
 *            6) IOC demo : hàng đợi byte vòng (SR queued tối giản)
 *            7) Resource : mutex đơn giản (không có ceiling protocol)
 *            8) Schedule : ScheduleTable “lite”
 *            9) Arch     : glue phụ thuộc kiến trúc (SysTick/PendSV/bootstrap)
 *
 * @version  1.0
 * @date     2025-09-10
 * @author   HALA Academy
 **********************************************************/
#ifndef OS_H
#define OS_H

#include "Os_Types.h"
#include "Os_Cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif
/* =========================================================
 * Macro định nghĩa thân TASK theo phong cách AUTOSAR
 *  - AUTOSAR chuẩn:  #define TASK(Name)  FUNC(void, OS_CODE) Name(void)
 *  - Bản này:        TASK(Name) mở rộng thành “void Name(void)”.
 *  - Ứng dụng định nghĩa:
 *
 * ======================================================= */
#ifndef TASK
#define TASK(Name) void Name(void)
#endif
    /* =========================================================
     * 1) LIFECYCLE
     * =======================================================*/
    /**
     * @brief  Khởi động OS.
     * @param  appMode  AppMode (OSDEFAULTAPPMODE nếu không dùng đa mode)
     * @return E_OK nếu khởi động thành công (thực tế thường không trả về)
     *
     * @details
     *  - Khởi tạo phần phụ thuộc kiến trúc: OS_Arch_Init(), OS_Arch_SystickConfig(...).
     *  - Gọi StartupHook() (nếu có).
     *  - Kích hoạt autostart task (ví dụ InitTask).
     *  - Chuyển quyền điều khiển sang Scheduler (OS_Arch_StartFirstTask()).
     *  - Thông thường không quay lại; kết thúc bằng ShutdownOS().
     *
     * @note   Gọi từ trạng thái reset/main trước khi có task nào chạy.
     */
    StatusType StartOS(AppModeType appMode);
    /**
     * @brief  Tắt OS, kết thúc hệ thống.
     * @param  error  Mã kết thúc (nguyên nhân)
     * @note   Gọi ShutdownHook(e) (nếu có), dừng SysTick, vô hiệu lịch, và quay về main().
     */
    void ShutdownOS(StatusType error);
    /* =========================================================
     * 2) TASK API
     * =======================================================*/
    /**
     * @brief  Kích hoạt một Task theo ID.
     * @param  tid  ID Task (xem Os_Cfg.h)
     * @return E_OK | E_OS_ID | E_OS_LIMIT | E_OS_STATE
     *
     * @details
     *  - Nếu Task đang SUSPENDED → chuyển về READY, gán entry/stack/priority.
     *  - Nếu đã RUNNING/READY và không cấu hình multi-activation → E_OS_LIMIT.
     *  - Có thể được gọi từ TASK hoặc ISR (Cat2) tùy hiện thực.
     */
    StatusType ActivateTask(TaskType tid);
    /**
     * @brief  Kết thúc Task hiện tại.
     * @return E_OK | E_OS_STATE (nếu gọi khi không ở RUNNING)
     *
     * @details
     *  - Chuyển Task về SUSPENDED (Basic) hoặc READY (nếu cấu hình khác).
     *  - Gọi PostTaskHook() trước khi nhường CPU.
     */
    StatusType TerminateTask(void);
    /**
     * @brief  Nhường CPU tự nguyện (cooperative yield).
     * @note   Không thay đổi trạng thái (vẫn READY); chỉ kích PendSV để chuyển ngữ cảnh.
     */
    void OS_Yield(void);
    /**
     * @brief  Treo Task hiện tại một khoảng thời gian (ms).
     * @param  ms  Thời gian trì hoãn (mili-giây)
     * @note   Chỉ gọi ở ngữ cảnh TASK; không gọi từ ISR. Task chuyển sang WAITING.
     */
    void OS_Delay(uint32_t ms);
    /* =========================================================
     * 3) EVENT API (Extended Task)
     * =======================================================*/
    /**
     * @brief  Chờ tới khi bất kỳ bit trong mask được set.
     * @param  mask  Mặt nạ event mong đợi
     * @return E_OK | E_OS_STATE | E_OS_TIMEOUT (nếu có cơ chế timeout đi kèm)
     *
     * @details
     *  - Chỉ dùng cho Extended Task.
     *  - Không gọi từ ISR. Task chuyển sang WAITING cho tới khi (events & m) != 0.
     */
    StatusType WaitEvent(EventMaskType mask);
    /**
     * @brief  Set event cho Task t và đánh thức nếu Task đang WaitEvent().
     * @param  tid  Task ID
     * @param  mask  Event mask cần set
     * @return E_OK | E_OS_ID
     *
     * @details
     *  - Có thể gọi từ TASK hoặc ISR (Cat2).
     *  - OR bit vào events của Task t; nếu đang WAITING và trùng mask → chuyển READY.
     */
    StatusType SetEvent(TaskType tid, EventMaskType mask);
    /**
     * @brief  Đọc mặt nạ event hiện tại của Task t.
     * @param  tid   Task ID
     * @param  mask   [out] con trỏ nhận giá trị mask
     * @return E_OK | E_OS_ID
     */
    StatusType GetEvent(TaskType tid, EventMaskType *mask);
    /**
     * @brief  Xoá (clear) các bit event đã xử lý của Task hiện tại.
     * @param  mask  Mặt nạ cần xoá
     * @return E_OK | E_OS_STATE
     *
     * @note   Chỉ gọi bởi chính Task đang RUNNING (Extended Task).
     */
    StatusType ClearEvent(EventMaskType mask);
    /* =========================================================
     * 4) ALARM API (ms)
     * =======================================================*/
    /**
     * @brief  Đặt alarm tương đối (ms).
     * @param  alarm     Alarm ID
     * @param  offset    Trễ trước lần kích đầu tiên
     * @param  cycle     Chu kỳ kích tiếp theo (0 = one-shot)
     * @return E_OK | E_OS_ID | E_OS_STATE | E_OS_LIMIT
     *
     * @details
     *  - Mỗi tick (SysTick) OS cập nhật bộ đếm; tới hạn → kích hoạt Task map với Alarm.
     *  - Mapping Alarm→Task do phần triển khai OS/ cấu hình xác định.
     */
    StatusType SetRelAlarm(AlarmType alarm, TickType offset, TickType cycle);
    /***************************************************
     * @brief Đặt alarm tuyệt đối (ms).
     * @param  alarm     Alarm ID [0..ALARM_COUNT-1]
     * @param  start     Thời điểm kích lần đầu tiên (tính từ 0)
     * @param  cycle     Chu kỳ kích sau đó (0 = one-shot)
     * @return E_OK | E_OS_ID | E_OS_STATE | E_OS_LIMIT
     *****************************************************/
    StatusType SetAbsAlarm(AlarmType alarm, TickType start, TickType cycle);
    /**
     * @brief  Huỷ một alarm nếu đang hoạt động.
     * @param  alarm  Alarm ID
     * @return E_OK | E_OS_ID | E_OS_STATE
     */
    StatusType CancelAlarm(AlarmType alarm);
    /* =========================================================
     * 5) COUNTER API
     * =======================================================*/
    /**
     * @brief  Lấy giá trị tick (ms) kể từ khi StartOS.
     * @return TickType (tràn theo modulo 2^width)
     */
    TickType OS_TickCount(CounterTypeId cid);
    StatusType IncrementCounter(CounterType counter);
    StatusType GetCounterValue(CounterType counter, TickRefType value);
    /* =========================================================
     * 6) SCHEDULE TABLE API
     * =======================================================*/
    StatusType StartScheduleTableRel(uint8_t table_id, TickType offset);
    StatusType StartScheduleTableAbs(uint8_t table_id, TickType start);
    StatusType StopScheduleTable(uint8_t table_id);
    StatusType SyncScheduleTable(uint8_t table_id, TickType new_start_offset);
    void ScheduleTableTick(CounterType cid);
    /* =========================================================
     * 7) IOC API
     * =======================================================*/
    uint8_t IocSend(uint8_t ch, void *data);
    uint8_t IocReceive(uint8_t ch, void *data, TaskType receiver);
    uint8_t IocReceiveGroup(uint8_t ch, void *data, uint8_t num);
    uint8_t IocHasNewData(uint8_t ch, TaskType receiver);
    /* =========================================================
     * 8) HOOK API
     * =======================================================*/
    void StartupHook(void);
    void ShutdownHook(StatusType error);
    void ErrorHook(StatusType error);
    void PreTaskHook(void);
    void PostTaskHook(void);
    /* =========================================================
     * 9) Resource API
     * =======================================================*/
    /** @brief Lấy resource (bận-chờ hoặc vô hiệu ngắt để bảo vệ, tuỳ hiện thực). */
    void GetResource(ResourceControlBlock *r);
    /** @brief Nhả resource. */
    void ReleaseResource(ResourceControlBlock *r);

        void Os_TickISR(void);

    StatusType Os_ConnectAlarm(AlarmType alarm, void (*cb)(void));
    StatusType Os_DisconnectAlarm(AlarmType alarm);

    void DisableAllInterrupts(void);
    void EnableAllInterrupts(void);

#ifdef __cplusplus
}
#endif
#endif /* OS_H */
