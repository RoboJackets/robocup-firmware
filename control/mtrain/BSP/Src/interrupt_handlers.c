#include "bsp.h"

extern void osSystickHandler(void);

void SysTick_Handler(void)
{
    HAL_IncTick();
    osSystickHandler();
}

extern PCD_HandleTypeDef hpcd;

/*  */
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
    HAL_PCD_IRQHandler(&hpcd);
}

extern QSPI_HandleTypeDef QSPIHandle;

void QUADSPI_IRQHandler(void)
{
    HAL_QSPI_IRQHandler(&QSPIHandle);
}