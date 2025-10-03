/*************************************
 * @file IoHwAb_Adc.c
 * @brief IoHwAb ADC Driver Source File (AUTOSAR)
 * @details Cài đặt các hàm cho IoHwAb ADC
 * @version 1.0
 * @date 09-09-2025
 * @author Nguyễn Tuấn Khoa
 **************************************/
#include "IoHwAb_adc.h"

static uint16_t Temperature_RawValue = 0;
static uint16_t Temperature_ScaledValue = 0;
Adc_ValueGroupType ADC_Group_Buffer[ADC_MAX_GROUPS];

void IoHwAb_Init0(const IoHwAb0_ConfigType *cfg)
{
    if (cfg == NULL)
    {
        // Handle null pointer error
        return;
    }
    // khởi tạo Port
    Port_Init(cfg->portConfig);
    // Khởi tạo Adc
    Adc_Init(cfg->adcConfig);
    //   Adc_EnableGroupNotification(0);
}

Std_ReturnType IoHwAb_ReadRaw_0(uint16_t *value)
{
    if (value == NULL)
    {
        return E_NOT_OK;
    }
    Adc_GroupType grp = Adc_GroupConfigs->id;
    Adc_StartGroupConversion(0);
    Adc_ReadGroup(0, &ADC_Group_Buffer[0]);
    *value = ADC_Group_Buffer[0];
    return E_OK;
}
Std_ReturnType IoHwAb_ReadScaleValue_0(uint16_t *temperature)
{
    if (temperature == NULL)
    {
        return E_NOT_OK;
    }
    // Giả sử giá trị thô từ ADC là từ 0 đến 1023 (10-bit ADC)
    // và nhiệt độ tương ứng là từ -40 đến 125 độ C
    Temperature_ScaledValue = (Temperature_RawValue * 165) / 1023 - 40;
    *temperature = Temperature_ScaledValue;
    return E_OK;
}