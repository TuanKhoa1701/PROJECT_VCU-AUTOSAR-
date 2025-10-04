/**********************************************
 * @file   : IoHwAb_Adc_Cfg.h
 * @brief  : IoHwAb ADC Driver Configuration Header File (AUTOSAR)
 * @details: Khai báo kiểu cấu hình cho ADC trong IoHwAb.
 * @version: 1.0
 * @date   : 2024-06-27
 * @author : Nguyễn Tuấn Khoa
 *********************************************/
#ifndef __IOHWAB_ADC_CFG_H__
#define __IOHWAB_ADC_CFG_H__
#include "Port_cfg.h"
#include "Adc_Cfg.h"
#include "Pwm_Lcfg.h"

extern Port_ConfigType portConfig;
typedef struct
{
    Port_ConfigType *portConfig;
} IoHwAb1_ConfigType;

const IoHwAb1_ConfigType IoHwAb1_Config = {
    .portConfig = &portConfig,

};
#endif /* __IOHWAB_ADC_CFG_H__ */