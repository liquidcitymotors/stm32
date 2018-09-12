/**
 ******************************************************************************
 * @file    tsl_time.h
 * @author  MCD Application Team
 * @version V2.2.0
 * @date    01-february-2016
 * @brief   This file contains external declarations of the tsl_time.c file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TSL_TIME_H
#define __TSL_TIME_H

/* Includes ------------------------------------------------------------------*/
#include "tsl_conf.h"

/* Exported functions ------------------------------------------------------- */

void TSL_tim_ProcessIT(void);
TSL_Status_enum_T TSL_tim_CheckDelay_ms(TSL_tTick_ms_T delay_ms,
		__IO TSL_tTick_ms_T *last_tick);
TSL_Status_enum_T TSL_tim_CheckDelay_sec(TSL_tTick_sec_T delay_sec,
		__IO TSL_tTick_sec_T *last_tick);
void TSL_CallBack_TimerTick(void);

#endif /* __TSL_TIME_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
