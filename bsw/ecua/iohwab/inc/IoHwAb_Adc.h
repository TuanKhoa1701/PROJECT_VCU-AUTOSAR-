/*************************************
 * @file: IoHwAb_Adc.h
 * @brief: IoHwAb ADC Driver Header File (AUTOSAR)
 * @details: Khai báo các hàm và kiểu dữ liệu cho IoHwAb ADC
 * @version: 1.0
 * @date: 09-09-2025
 * @author: Nguyễn Tuấn Khoa
 **************************************/
#ifndef __IOHWAB_ADC_H__
#define __IOHWAB_ADC_H__

#include "IoHwAb_Types.h"
#include "IoHwAb_Adc_Cfg.h"

/************************************
 * @typedef void
 * @brief: Hàm khởi tạo IoHwAb ADC
 * @param[in] ConfigPtr: Con trỏ tới cấu hình IoHwAb0_ConfigType
 * @return: Không trả về giá trị
 *************************************/
void IoHwAb_Init0(const IoHwAb0_ConfigType *ConfigPtr);

/************************************
 * @typedef Std_ReturnType
 * @brief: Hàm đọc giá trị ADC thô
 * @param[in] Value: Con trỏ tới biến lưu giá trị ADC
 * @return: Giá trị trả về kiểu Std_ReturnType
 ****************************************/
Std_ReturnType IoHwAb_ReadRaw_0(uint16_t *value);

/*************************************
 * @typedef Std_ReturnType
 * @brief: Hàm đọc giá trị ADC đã hiệu chỉnh
 * @param[in] Value: Con trỏ tới biến lưu giá trị ADC
 * @return: Giá trị trả về kiểu Std_ReturnType
 *************************************/
Std_ReturnType IoHwAb_ReadScaleValue_0(uint16_t *temperature);

#endif /* __IOHWAB_ADC_H__ */