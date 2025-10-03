/* rte/core/src/Rte.c */
#include "Rte.h"
#include "Rte_Types.h"

/* Kéo vào các Application Headers để bảo đảm prototype khớp */
#include "Rte_Swc_PedalAcq.h"
#include "Rte_Swc_BrakeAcq.h"
#include "Rte_Swc_GearSelector.h"
#include "Rte_Swc_DriveModeMgr.h"
#include "Rte_Swc_SafetyManager.h"
#include "Rte_Swc_CmdComposer.h"

/* Forward tới IoHwAb / CanIf (Client-Server) */
#include "IoHwAb_Adc.h"     /* Std_ReturnType IoHwAb_Adc_ReadChannel(uint8, uint16*) */
#include "IoHwAb_Digital.h" /* Std_ReturnType IoHwAb_Digital_ReadChannel(uint8, boolean*);
                                 Std_ReturnType IoHwAb_Digital_GetDebounced(uint8, boolean*); */
#include "CanIf.h"          /* Std_ReturnType CanIf_Transmit_VcuCommand(uint8,uint8,uint8,boolean,uint8) */

#ifdef __cplusplus
extern "C"
{
#endif

    /* =======================================================
     *                   RTE INTERNAL BUFFERS
     *  (SR non-queued explicit + IsUpdated flags + Mode value)
     * ======================================================= */

    /* PedalAcq -> PedalOut.PedalPct (uint8) */
    static uint8_t Rte_Buffer_PedalOut_PedalPct = 0u;
    static boolean Rte_IsUpdated_PedalOut_PedalPct_Flag = FALSE;

    /* BrakeAcq -> BrakeOut.BrakePressed (boolean) */
    static boolean Rte_Buffer_BrakeOut_BrakePressed = FALSE;
    static boolean Rte_IsUpdated_BrakeOut_BrakePressed_Flag = FALSE;

    /* GearSelector -> GearOut.Gear (Gear_e) */
    static Gear_e Rte_Buffer_GearOut_Gear = GEAR_P;
    static boolean Rte_IsUpdated_GearOut_Gear_Flag = FALSE;

    /* SafetyManager -> SafeOut.Cmd (Safe_s) */
    static Safe_s Rte_Buffer_SafeOut_Cmd = {0};
    static boolean Rte_IsUpdated_SafeOut_Cmd_Flag = FALSE;

    /* Mode port: DriveMode */
    static DriveMode_e Rte_Mode_DriveMode_Value = MODE_ECO;
    static boolean Rte_Mode_DriveMode_SwitchPendingAck = FALSE;

    /* Lifecycle state (đơn giản) */
    static boolean Rte_Core_Started = FALSE;
    static boolean Rte_Timing_Activated = FALSE;

    /* =======================================================
     *                     LIFECYCLE API
     * ======================================================= */
    Std_ReturnType Rte_Start(void)
    {
        /* Reset buffers & flags về trạng thái an toàn */
        Rte_Buffer_PedalOut_PedalPct = 0u;
        Rte_IsUpdated_PedalOut_PedalPct_Flag = FALSE;

        Rte_Buffer_BrakeOut_BrakePressed = FALSE;
        Rte_IsUpdated_BrakeOut_BrakePressed_Flag = FALSE;

        Rte_Buffer_GearOut_Gear = GEAR_P;
        Rte_IsUpdated_GearOut_Gear_Flag = FALSE;

        Rte_Buffer_SafeOut_Cmd.ThrottleReq_pct = 0u;
        Rte_Buffer_SafeOut_Cmd.Gear = GEAR_P;
        Rte_Buffer_SafeOut_Cmd.Mode = MODE_ECO;
        Rte_Buffer_SafeOut_Cmd.Flags = 0u;
        Rte_Buffer_SafeOut_Cmd.BrakeActive = FALSE;
        Rte_IsUpdated_SafeOut_Cmd_Flag = FALSE;

        Rte_Mode_DriveMode_Value = MODE_ECO;
        Rte_Mode_DriveMode_SwitchPendingAck = FALSE;

        Rte_Core_Started = TRUE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Stop(void)
    {
        Rte_Timing_Activated = FALSE;
        Rte_Core_Started = FALSE;
        return RTE_E_OK;
    }
    void Rte_Tick10ms(void)
    {
    }
    void Rte_Init_Core(void)
    {
        /* Nếu có initialization runnables, gọi ở đây theo thứ tự mong muốn.
           Dự án tối giản hiện không có, nên giữ trống. */
    }

    void Rte_StartTiming(void)
    {
        /* Cho phép OS/Scheduler kích runnable theo tick; RTE bản tối giản không tự lập lịch */
        Rte_Timing_Activated = TRUE;
        (void)Rte_Timing_Activated; /* tránh cảnh báo nếu chưa dùng */
    }

    /* =======================================================
     *               SENDER-RECEIVER — WRITE (Provide)
     * ======================================================= */
    /* PedalAcq -> PedalOut.PedalPct */
    Std_ReturnType Rte_Write_Swc_PedalAcq_PedalOut_PedalPct(uint8_t data)
    {
        Rte_Buffer_PedalOut_PedalPct = data;
        Rte_IsUpdated_PedalOut_PedalPct_Flag = TRUE;
        return RTE_E_OK;
    }

    /* BrakeAcq -> BrakeOut.BracePressed */
    Std_ReturnType Rte_Write_Swc_BrakeAcq_BrakeOut_BrakePressed(boolean data)
    {
        Rte_Buffer_BrakeOut_BrakePressed = data;
        Rte_IsUpdated_BrakeOut_BrakePressed_Flag = TRUE;
        return RTE_E_OK;
    }

    /* GearSelector -> GearOut.Gear */
    Std_ReturnType Rte_Write_Swc_GearSelector_GearOut_Gear(Gear_e data)
    {
        Rte_Buffer_GearOut_Gear = data;
        Rte_IsUpdated_GearOut_Gear_Flag = TRUE;
        return RTE_E_OK;
    }

    /* SafetyManager -> SafeOut.Cmd */
    Std_ReturnType Rte_Write_Swc_SafetyManager_SafeOut_Cmd(const Safe_s *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        Rte_Buffer_SafeOut_Cmd = *data;
        Rte_IsUpdated_SafeOut_Cmd_Flag = TRUE;
        return RTE_E_OK;
    }

    /* =======================================================
     *           SENDER-RECEIVER — READ (Require)
     * (clear IsUpdated flag khi đọc để báo đã tiêu thụ)
     * ======================================================= */
    Std_ReturnType Rte_Read_Swc_SafetyManager_PedalOut_PedalPct(uint8_t *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_PedalOut_PedalPct;
        Rte_IsUpdated_PedalOut_PedalPct_Flag = FALSE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Read_Swc_SafetyManager_BrakeOut_BrakePressed(boolean *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_BrakeOut_BrakePressed;
        Rte_IsUpdated_BrakeOut_BrakePressed_Flag = FALSE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Read_Swc_SafetyManager_GearOut_Gear(Gear_e *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_GearOut_Gear;
        Rte_IsUpdated_GearOut_Gear_Flag = FALSE;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_Read_Swc_CmdComposer_SafeOut_Cmd(Safe_s *data)
    {
        if (data == NULL)
        {
            return RTE_E_INVALID;
        }
        *data = Rte_Buffer_SafeOut_Cmd;
        Rte_IsUpdated_SafeOut_Cmd_Flag = FALSE;
        return RTE_E_OK;
    }

    /* =======================================================
     *            SENDER-RECEIVER — IsUpdated helpers
     * ======================================================= */
    Std_ReturnType Rte_IsUpdated_Swc_PedalAcq_PedalOut_PedalPct(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_PedalOut_PedalPct_Flag;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_IsUpdated_Swc_BrakeAcq_BrakeOut_BrakePressed(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_BrakeOut_BrakePressed_Flag;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_IsUpdated_Swc_GearSelector_GearOut_Gear(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_GearOut_Gear_Flag;
        return RTE_E_OK;
    }

    Std_ReturnType Rte_IsUpdated_Swc_SafetyManager_SafeOut_Cmd(boolean *updated)
    {
        if (updated == NULL)
        {
            return RTE_E_INVALID;
        }
        *updated = Rte_IsUpdated_SafeOut_Cmd_Flag;
        return RTE_E_OK;
    }

    /* =======================================================
     *               MODE MANAGEMENT — DriveMode
     * ======================================================= */
    Std_ReturnType Rte_Switch_Swc_DriveModeMgr_DriveMode_Mode(DriveMode_e mode)
    {
        /* Validate input (tối giản) */
        if (mode != MODE_ECO && mode != MODE_NORMAL)
        {
            return RTE_E_LIMIT;
        }
        Rte_Mode_DriveMode_Value = mode;
        Rte_Mode_DriveMode_SwitchPendingAck = TRUE;
        return RTE_E_OK;
    }

    /* Hai hàm Mode_Read cho 2 SWC consumer khác nhau (cùng trả một giá trị) */
    DriveMode_e Rte_Mode_Swc_DriveModeMgr_DriveMode_Mode(void)
    {
        return Rte_Mode_DriveMode_Value;
    }

    DriveMode_e Rte_Mode_Swc_SafetyManager_DriveMode_Mode(void)
    {
        return Rte_Mode_DriveMode_Value;
    }

    Std_ReturnType Rte_SwitchAck_Swc_DriveModeMgr_DriveMode_Mode(void)
    {
        if (Rte_Mode_DriveMode_SwitchPendingAck == TRUE)
        {
            Rte_Mode_DriveMode_SwitchPendingAck = FALSE;
            return RTE_E_OK;
        }
        return RTE_E_NO_DATA; /* không có switch đang chờ ACK */
    }

    /* =======================================================
     *          CLIENT–SERVER FORWARDING (IoHwAb / CanIf)
     * ======================================================= */
    Std_ReturnType Rte_Call_Swc_PedalAcq_AdcIf_ReadChannel(uint8_t channel, uint16_t *value)
    {
        return IoHwAb_Adc_ReadChannel(channel, value);
    }

    Std_ReturnType Rte_Call_Swc_BrakeAcq_DigIf_ReadChannel(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_ReadChannel(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_BrakeAcq_DigIf_GetDebounced(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_GetDebounced(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_GearSelector_DigIf_ReadChannel(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_ReadChannel(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_DriveModeMgr_DigIf_ReadChannel(uint8_t channel, boolean *level)
    {
        return IoHwAb_Digital_ReadChannel(channel, level);
    }

    Std_ReturnType Rte_Call_Swc_CmdComposer_CanIf_TransmitVcuCommand(uint8_t throttle_pct,
                                                                     uint8_t gear,
                                                                     uint8_t mode,
                                                                     boolean brake,
                                                                     uint8_t alive)
    {
        return CanIf_Transmit_VcuCommand(throttle_pct, gear, mode, brake, alive);
    }

    /* =======================================================
     *         EXCLUSIVE AREA (SafetyManager) – tối giản
     * ======================================================= */
    void Rte_Enter_Swc_SafetyManager_EA(void)
    {
        /* No-op trong bản tối giản; có thể disable IRQ hoặc lock nếu cần */
    }

    void Rte_Exit_Swc_SafetyManager_EA(void)
    {
        /* No-op */
    }

#ifdef __cplusplus
}
#endif
