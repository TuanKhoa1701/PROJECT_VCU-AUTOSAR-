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

extern const Port_ConfigType portConfig;
extern const Adc_ConfigType Adc_Configs[];
typedef struct
{
    const Port_ConfigType *portConfig;
    const Adc_ConfigType *adcConfig;
} IoHwAb0_ConfigType;

const IoHwAb0_ConfigType IoHwAb0_Config = {
    .portConfig = &portConfig,
    .adcConfig = &Adc_Configs[0],
};

#endif /* __IOHWAB_ADC_CFG_H__ */