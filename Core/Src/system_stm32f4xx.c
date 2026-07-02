/**
 ******************************************************************************
 * @file    system_stm32f4xx.c
 * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
 *          Provides system clock configuration (called from startup)
 ******************************************************************************
 */

#include "stm32f4xx.h"

/* VDD = 3.3V, Scale1 mode - SYSCLK up to 168MHz */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)8000000)
#endif

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000)
#endif

#define VECT_TAB_OFFSET  0x00

uint32_t SystemCoreClock = 168000000;
const uint8_t  AHBPrescTable[16] = {0,0,0,0,0,0,0,0,1,2,3,4,6,7,8,9};
const uint8_t  APBPrescTable[8]  = {0,0,0,0,1,2,3,4};

void SystemInit(void)
{
  /* FPU settings - enable CP10 and CP11 full access */
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));

  /* Reset the RCC clock configuration to default reset state */
  RCC->CR |= (uint32_t)0x00000001;
  RCC->CFGR = 0x00000000;
  RCC->CR &= (uint32_t)0xFEF6FFFF;
  RCC->PLLCFGR = 0x24003010;
  RCC->CR &= (uint32_t)0xFFFBFFFF;
  RCC->CIR = 0x00000000;

  /* Configure the Vector Table location */
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;
}

void SystemCoreClockUpdate(void)
{
  uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp)
  {
    case 0x00:  /* HSI used as system clock source */
      SystemCoreClock = HSI_VALUE;
      break;
    case 0x04:  /* HSE used as system clock source */
      SystemCoreClock = HSE_VALUE;
      break;
    case 0x08:  /* PLL used as system clock source */
      pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
      pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

      if (pllsource != 0)
        pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
      else
        pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);

      pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
      SystemCoreClock = pllvco/pllp;
      break;
    default:
      SystemCoreClock = HSI_VALUE;
      break;
  }

  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
  SystemCoreClock >>= tmp;
}
