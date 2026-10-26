/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "sdmmc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* WARNING: LBA 4096 is not reserved space. It could contain filesystem data.
 * Only run with a disposable microSD card whose contents can be lost.
 * Change SD_TEST_ENABLE_WRITES to 0 to test card detection/read only.
 */
#define SD_TEST_LBA            4096U
#define SD_BLOCK_SIZE          512U
#define SD_TIMEOUT_MS          3000U
#define SD_TEST_ENABLE_WRITES  0U

/* ST LSM6DSV320X low-g accel + gyro test on I2C5. */
#define IMU_I2C_ADDR        (0x6AU << 1) /* HAL expects shifted 7-bit addr */
#define IMU_WHO_AM_I       0x0FU
#define IMU_EXPECTED_ID    0x73U
#define IMU_CTRL1          0x10U
#define IMU_CTRL2          0x11U
#define IMU_CTRL3          0x12U
#define IMU_CTRL6          0x15U
#define IMU_CTRL8          0x17U
#define IMU_OUTX_L_G       0x22U
#define IMU_POLL_MS        250U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USB CDC device handle defined in usb_device.c. */
extern USBD_HandleTypeDef hUsbDeviceHS;
static uint8_t usb_initialized = 0;
static uint8_t usb_connected = 0;
static uint8_t boot_message_sent = 0;
static uint32_t last_tx_ms = 0;
/* CDC sends asynchronously. Do not overwrite until TxState becomes zero. */
static uint8_t usb_tx_buffer[128];

/* Existing DotStar LED blink: all five use the user's RGB values. */
static uint32_t last_led_toggle_ms = 0;
static uint8_t led_on = 0;

/* Existing OLED test. */
static uint16_t oled_addr = 0;   /* STM32 HAL uses 7-bit address << 1. */
static uint8_t oled_ready = 0;

/* Latch the six-position robot-ID DIP switch once at startup. */
static uint8_t robot_id = 0;

/* EXTI ISR -> main-loop button event flags. */
static volatile uint8_t button_pending[3] = {0};
static volatile uint32_t button_last_ms[3] = {0};
static volatile uint8_t button_seen[3] = {0};

/* Static storage avoids consuming the small main stack. */
static uint8_t sd_original[SD_BLOCK_SIZE] __attribute__((aligned(32)));
static uint8_t sd_pattern[SD_BLOCK_SIZE] __attribute__((aligned(32)));
static uint8_t sd_verify[SD_BLOCK_SIZE] __attribute__((aligned(32)));
static uint8_t sd_test_done = 0;
static uint8_t sd_report_pending = 0;
static char sd_report[112];

/* IMU: nonfatal init, samples every 250 ms, reports over existing CDC. */
static uint8_t imu_ready = 0U;
static uint8_t imu_id = 0U;
static uint8_t imu_status_pending = 1U;
static uint8_t imu_report_pending = 0U;
static uint32_t imu_last_poll_ms = 0U;
static char imu_report[128];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
static uint8_t OLED_ShowHello(void);
static uint8_t ReadRobotID(void);
static void LED_SetColors(uint8_t on);
static bool SD_WaitReady(uint32_t timeout_ms);
static void SD_RunTest(void);
static uint8_t IMU_Init(void);
static uint8_t IMU_Read(int32_t accel_mg[3], int32_t gyro_mdps[3]);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* SW3: DIP0..DIP5 = PF7, PF8, PF9, PF10, PC0, PC1.
 * Each DIP input has an external 100k pull-up. ON = active-low (0).
 * Return a six-bit ID with ON mapped to a set bit.
 * Call exactly once after MX_GPIO_Init(), not from the main loop.
 */
static uint8_t ReadRobotID(void)
{
    uint8_t id = 0;
    id |= (uint8_t)((HAL_GPIO_ReadPin(DIP0_GPIO_Port, DIP0_Pin) == GPIO_PIN_RESET) << 0);
    id |= (uint8_t)((HAL_GPIO_ReadPin(DIP1_GPIO_Port, DIP1_Pin) == GPIO_PIN_RESET) << 1);
    id |= (uint8_t)((HAL_GPIO_ReadPin(DIP2_GPIO_Port, DIP2_Pin) == GPIO_PIN_RESET) << 2);
    id |= (uint8_t)((HAL_GPIO_ReadPin(DIP3_GPIO_Port, DIP3_Pin) == GPIO_PIN_RESET) << 3);
    id |= (uint8_t)((HAL_GPIO_ReadPin(DIP4_GPIO_Port, DIP4_Pin) == GPIO_PIN_RESET) << 4);
    id |= (uint8_t)((HAL_GPIO_ReadPin(DIP5_GPIO_Port, DIP5_Pin) == GPIO_PIN_RESET) << 5);
    return id;
}

/* Write one device register and check transaction result. */
static HAL_StatusTypeDef IMU_WriteReg(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c5, IMU_I2C_ADDR, reg,
                             I2C_MEMADD_SIZE_8BIT, &value, 1U, 100U);
}

/* Verify this is the LSM6DSV320X (not the pin-compatible older IMU).
 * Low-g accel: +/-2g, gyro: +/-250 dps, both 120 Hz high performance.
 * CTRL3=0x44 sets BDU=1 and auto-increment=1 for coherent burst reads.
 */
static uint8_t IMU_Init(void)
{
    uint8_t reg3 = 0U;

    HAL_Delay(10U);
    if (HAL_I2C_Mem_Read(&hi2c5, IMU_I2C_ADDR, IMU_WHO_AM_I,
                         I2C_MEMADD_SIZE_8BIT, &imu_id, 1U, 100U) != HAL_OK ||
        imu_id != IMU_EXPECTED_ID)
        return 0U;

    /* Power down sensors before changing full-scale/rate settings. */
    if (IMU_WriteReg(IMU_CTRL1, 0x00U) != HAL_OK ||
        IMU_WriteReg(IMU_CTRL2, 0x00U) != HAL_OK ||
        IMU_WriteReg(IMU_CTRL3, 0x44U) != HAL_OK ||
        IMU_WriteReg(IMU_CTRL8, 0x00U) != HAL_OK || /* XL +/-2g */
        IMU_WriteReg(IMU_CTRL6, 0x09U) != HAL_OK || /* G +/-250 dps, reserved bit3=1 */
        IMU_WriteReg(IMU_CTRL1, 0x06U) != HAL_OK || /* XL 120 Hz HP */
        IMU_WriteReg(IMU_CTRL2, 0x06U) != HAL_OK)   /* G 120 Hz HP */
        return 0U;

    /* Read back the critical interface configuration. */
    if (HAL_I2C_Mem_Read(&hi2c5, IMU_I2C_ADDR, IMU_CTRL3,
                         I2C_MEMADD_SIZE_8BIT, &reg3, 1U, 100U) != HAL_OK ||
        (reg3 & 0x44U) != 0x44U)
        return 0U;

    HAL_Delay(50U); /* Let first low-g and gyro samples become available. */
    return 1U;
}

/* Read gyro x/y/z (0x22..0x27), then low-g XL x/y/z (0x28..0x2D).
 * Raw values are 16-bit little-endian two's-complement.
 * ST sensitivities at chosen ranges:
 *   +/-2g:   0.061 mg/LSB
 *   +/-250dps: 8.75 mdps/LSB
 * Integer conversion avoids pulling in printf floating-point support.
 */
static uint8_t IMU_Read(int32_t accel_mg[3], int32_t gyro_mdps[3])
{
    uint8_t data[12];
    if (HAL_I2C_Mem_Read(&hi2c5, IMU_I2C_ADDR, IMU_OUTX_L_G,
                         I2C_MEMADD_SIZE_8BIT, data, sizeof(data), 100U)
        != HAL_OK)
        return 0U;

    for (uint8_t i = 0U; i < 3U; i++)
    {
        int16_t g = (int16_t)((uint16_t)data[2U*i] |
                     ((uint16_t)data[2U*i + 1U] << 8));
        int16_t a = (int16_t)((uint16_t)data[6U + 2U*i] |
                     ((uint16_t)data[7U + 2U*i] << 8));
        gyro_mdps[i] = ((int32_t)g * 875) / 100; /* 8.75 mdps/LSB */
        accel_mg[i] = ((int32_t)a * 61) / 1000; /* 0.061 mg/LSB */
    }
    return 1U;
}

/* SDMMC1 is intentionally NOT initialized by CubeMX at startup:
 * an absent card must not prevent USB CDC from enumerating.
 * HAL_SD_Init() calls HAL_SD_MspInit(), configures card setup at the
 * slow initialization clock, and then switches to 4-bit transfer mode.
 */
static bool SD_WaitReady(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    do
    {
        if (HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER)
            return true;
        HAL_Delay(1);
    } while ((uint32_t)(HAL_GetTick() - start) < timeout_ms);
    return false;
}

static void SD_RunTest(void)
{
    HAL_SD_CardInfoTypeDef card = {0};
    const char *result = "PASS (written/read/verified/restored)";
    bool restore_needed = false;
    bool restore_ok = true;

    /* Matches the CubeMX-generated configuration without Error_Handler(). */
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 1;

    if (HAL_SD_Init(&hsd1) != HAL_OK)
    {
        result = "INIT FAILED (card absent or SDMMC fault)";
        goto report;
    }
    if (HAL_SD_GetCardInfo(&hsd1, &card) != HAL_OK)
    {
        result = "CARD INFO FAILED";
        goto report;
    }
    if (card.LogBlockSize != SD_BLOCK_SIZE ||
        card.LogBlockNbr <= SD_TEST_LBA)
    {
        result = "INVALID CARD SIZE / LBA";
        goto report;
    }
    if (!SD_WaitReady(SD_TIMEOUT_MS))
    {
        result = "CARD NOT READY";
        goto report;
    }

    /* Preserve the original sector before modifying it. */
    if (HAL_SD_ReadBlocks(&hsd1, sd_original, SD_TEST_LBA, 1,
                          SD_TIMEOUT_MS) != HAL_OK ||
        !SD_WaitReady(SD_TIMEOUT_MS))
    {
        result = "INITIAL READ FAILED";
        goto report;
    }

#if SD_TEST_ENABLE_WRITES
    for (uint32_t i = 0; i < SD_BLOCK_SIZE; i++)
        sd_pattern[i] = (uint8_t)(i ^ 0xA5U);

    static const char message[] = "RJ RoboCup SDMMC read/write test";
    memcpy(sd_pattern, message, sizeof(message));

    /* Mark for restoration BEFORE attempting any modification. */
    restore_needed = true;
    if (HAL_SD_WriteBlocks(&hsd1, sd_pattern, SD_TEST_LBA, 1,
                           SD_TIMEOUT_MS) != HAL_OK ||
        !SD_WaitReady(SD_TIMEOUT_MS))
    {
        result = "TEST WRITE FAILED";
        goto restore;
    }

    memset(sd_verify, 0, sizeof(sd_verify));
    if (HAL_SD_ReadBlocks(&hsd1, sd_verify, SD_TEST_LBA, 1,
                          SD_TIMEOUT_MS) != HAL_OK ||
        !SD_WaitReady(SD_TIMEOUT_MS))
    {
        result = "TEST READBACK FAILED";
        goto restore;
    }
    if (memcmp(sd_pattern, sd_verify, SD_BLOCK_SIZE) != 0)
    {
        result = "DATA MISMATCH";
        goto restore;
    }
#else
    result = "READ PASS (writes disabled)";
#endif

restore:
    if (restore_needed)
    {
        /* Do not claim PASS unless original data was restored and verified. */
        restore_ok = SD_WaitReady(SD_TIMEOUT_MS) &&
            (HAL_SD_WriteBlocks(&hsd1, sd_original, SD_TEST_LBA, 1,
                                SD_TIMEOUT_MS) == HAL_OK) &&
            SD_WaitReady(SD_TIMEOUT_MS) &&
            (HAL_SD_ReadBlocks(&hsd1, sd_verify, SD_TEST_LBA, 1,
                               SD_TIMEOUT_MS) == HAL_OK) &&
            SD_WaitReady(SD_TIMEOUT_MS) &&
            (memcmp(sd_original, sd_verify, SD_BLOCK_SIZE) == 0);
        if (!restore_ok)
            result = "RESTORE FAILED - CHECK CARD!";
    }

report:
    (void)snprintf(sd_report, sizeof(sd_report),
                   "[SD] %s, LBA=%lu", result,
                   (unsigned long)SD_TEST_LBA);
}


/*
 * Minimal SSD1306 128x64 OLED bring-up test.
 * I2C1: PB8 SCL, PB9 SDA.
 * Uses blocking I2C writes; no external library.
 */

// Transmit one SSD1306 I2C packet.
// HAL expects the 7-bit address shifted left by one.
static HAL_StatusTypeDef OLED_Send(uint8_t *data, uint16_t len)
{
    return HAL_I2C_Master_Transmit(
        &hi2c1, oled_addr, data, len, 100);
}

static uint8_t OLED_ShowHello(void)
{
    // Allow display power and auto-reset to settle.
    HAL_Delay(100);

    // Probe both documented display addresses.
    if (HAL_I2C_IsDeviceReady(&hi2c1, 0x3C << 1, 2, 20) == HAL_OK)
    {
        oled_addr = 0x3C << 1;
    }
    else if (HAL_I2C_IsDeviceReady(&hi2c1, 0x3D << 1, 2, 20) == HAL_OK)
    {
        oled_addr = 0x3D << 1;
    }
    else
    {
        return 0; // OLED not detected
    }

    // SSD1306 initialization, 128x64, internal charge pump.
    // First byte 0x00 indicates command transmission.
    uint8_t init[] = {
        0x00,
        0xAE,       // Display OFF
        0xD5, 0x80, // Display clock
        0xA8, 0x3F, // Multiplex: 64 rows
        0xD3, 0x00, // Display offset
        0x40,       // Start line = 0
        0x8D, 0x14, // Enable charge pump
        0x20, 0x02, // Page addressing mode
        0xA1,       // Segment remap
        0xC8,       // COM scan direction
        0xDA, 0x12, // COM pins configuration
        0x81, 0x7F, // Contrast
        0xD9, 0xF1, // Pre-charge period
        0xDB, 0x40, // VCOMH deselect level
        0xA4,       // Display from RAM
        0xA6,       // Normal display (not inverted)
        0x2E        // Deactivate scrolling
    };

    if (OLED_Send(init, sizeof(init)) != HAL_OK)
        return 0;

    // Clear all 8 pages (128 columns per page).
    // 0x40 is the I2C control byte for display data.
    uint8_t clear[129] = {0x40};

    for (uint8_t page = 0; page < 8; page++)
    {
        uint8_t pos[] = {
            0x00,
            (uint8_t)(0xB0 | page), // Select page
            0x00,                   // Column low nibble
            0x10                    // Column high nibble
        };

        if (OLED_Send(pos, sizeof(pos)) != HAL_OK)
            return 0;

        if (OLED_Send(clear, sizeof(clear)) != HAL_OK)
            return 0;
    }

    // Position text at column 40, page 3.
    uint8_t text_pos[] = {
        0x00,
        0xB3, // Page 3
        0x08, // Low nibble of column 40
        0x12  // High nibble of column 40
    };

    if (OLED_Send(text_pos, sizeof(text_pos)) != HAL_OK)
        return 0;

    // "HELLO RJ" in a 5x7 bitmap font.
    // Each character occupies 5 columns + 1 blank column.
    uint8_t text[] = {
        0x40, // I2C data control byte

        // H
        0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,

        // E
        0x7F, 0x49, 0x49, 0x49, 0x41, 0x00,

        // L
        0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,

        // L
        0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,

        // O
        0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,

        // Space
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        // R
        0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,

        // J
        0x20, 0x40, 0x41, 0x3F, 0x01, 0x00
    };

    if (OLED_Send(text, sizeof(text)) != HAL_OK)
        return 0;

    // Turn the display on.
    uint8_t display_on[] = {0x00, 0xAF};

    if (OLED_Send(display_on, sizeof(display_on)) != HAL_OK)
        return 0;

    return 1;
}

/* The user's existing LED colors are preserved exactly:
 * five LEDs, brightness 0xE8 (8/31), B=G=R=0x0F when on.
 * The previous function ignored `on`; this one actually blinks.
 */
static void LED_SetColors(uint8_t on)
{
    uint8_t frame[28] = {0};  /* 4 start + 5*4 LED + 4 end */

    for (uint32_t i = 0; i < 5U; i++)
    {
        uint32_t idx = 4U + i * 4U;
        frame[idx]     = 0xE8;             /* global brightness 8/31 */
        frame[idx + 1] = on ? 0x0F : 0x00; /* Blue */
        frame[idx + 2] = on ? 0x0F : 0x00; /* Green */
        frame[idx + 3] = on ? 0x0F : 0x00; /* Red */
    }
    for (uint32_t i = 24U; i < 28U; i++)
        frame[i] = 0xFF;  /* APA102 end clocks */

    (void)HAL_SPI_Transmit(&hspi2, frame, sizeof(frame), 100);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  MPU_Config();
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize ONLY peripherals needed for these tests.
   * Keep other CubeMX-generated calls disabled in Advanced Settings,
   * particularly SDMMC1 (would halt USB boot when card is missing)
   * and UART4 (powerboard may be absent).
   */
  MX_GPIO_Init();
  MX_SPI2_Init();

  /* USER CODE BEGIN 2 */
  /* Startup-only robot ID: GPIO has been initialized already. */
  HAL_Delay(1); /* Allows the weak external DIP pull-ups to settle. */
  robot_id = ReadRobotID();  /* Latched until MCU reset. */

  LED_SetColors(0);  /* APA102 power-on state is undefined. */
  last_led_toggle_ms = HAL_GetTick();

  MX_I2C1_Init();
  oled_ready = OLED_ShowHello();

  /* IMU is on a separate I2C5 bus (PF1 SCL, PF0 SDA). */
  MX_I2C5_Init();
  imu_ready = IMU_Init(); /* Nonfatal failure: preserve USB/LED/OLED/buttons. */
  imu_last_poll_ms = HAL_GetTick();
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    uint32_t now = HAL_GetTick();
    GPIO_PinState vbus = HAL_GPIO_ReadPin(
        VBUS_SENSE_GPIO_Port, VBUS_SENSE_Pin);

    /* USB device: PA9 is an ordinary GPIO VBUS-sense input. */
    if (vbus == GPIO_PIN_SET && !usb_connected)
    {
        if (!usb_initialized)
        {
            MX_USB_DEVICE_Init();  /* Generated function includes USBD_Start. */
            usb_initialized = 1;
        }
        else if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
        {
            /* Keep other tests alive even if USB restart fails. */
            HAL_Delay(100);
            continue;
        }
        usb_connected = 1;
        boot_message_sent = 0;
        last_tx_ms = now;
    }

    if (vbus == GPIO_PIN_RESET && usb_connected)
    {
        (void)USBD_Stop(&hUsbDeviceHS);
        usb_connected = 0;
    }

    /* Only once per boot, and only AFTER the USB boot message is sent.
     * A missing SD card cannot prevent initial USB enumeration.
     */
    if (usb_connected &&
        hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED &&
        boot_message_sent && !sd_test_done)
    {
        sd_test_done = 1;
        SD_RunTest();       /* Blocking, may pause LED animation briefly. */
        sd_report_pending = 1;
        now = HAL_GetTick();
    }

    /* Poll IMU at 4 Hz; sample generation runs internally at 120 Hz.
     * Store the latest line separately from the asynchronous CDC TX buffer.
     */
    if (imu_ready && (uint32_t)(now - imu_last_poll_ms) >= IMU_POLL_MS)
    {
        int32_t accel_mg[3], gyro_mdps[3];
        imu_last_poll_ms = now;
        if (IMU_Read(accel_mg, gyro_mdps))
        {
            (void)snprintf(imu_report, sizeof(imu_report),
                           "[IMU] A(mg) X=%ld Y=%ld Z=%ld | G(mdps) X=%ld Y=%ld Z=%ld\r\n",
                           (long)accel_mg[0], (long)accel_mg[1], (long)accel_mg[2],
                           (long)gyro_mdps[0], (long)gyro_mdps[1], (long)gyro_mdps[2]);
        }
        else
        {
            (void)snprintf(imu_report, sizeof(imu_report),
                           "[IMU] Read failed (I2C5)\r\n");
            imu_ready = 0U; /* Stop repeated failing transactions. */
        }
        imu_report_pending = 1U;
    }

    /* One CDC transmitter, one persistent buffer, one message per loop.
     * Priority: boot > SD result > IMU status > button > IMU data > hello.
     */
    if (usb_connected &&
        hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED)
    {
        USBD_CDC_HandleTypeDef *cdc =
            (USBD_CDC_HandleTypeDef *)hUsbDeviceHS.pClassData;
        if (cdc != NULL && cdc->TxState == 0U)
        {
            int len = 0;
            int8_t button_idx = -1;
            bool sending_boot = false;
            bool sending_sd = false;
            bool sending_imu_status = false;
            bool sending_imu = false;
            bool sending_hello = false;

            if (!boot_message_sent)
            {
                len = snprintf((char *)usb_tx_buffer,
                               sizeof(usb_tx_buffer),
                               "[RJ RoboCup] ControlBoard2027 booted!\r\n"
                               "[OLED] %s, address: 0x%02X\r\n"
                               "[DIP] Robot ID: %u (0x%02X)\r\n",
                               oled_ready ? "OK" : "NOT DETECTED",
                               (unsigned int)(oled_addr >> 1),
                               (unsigned int)robot_id,
                               (unsigned int)robot_id);
                sending_boot = true;
            }
            else if (sd_report_pending)
            {
                len = snprintf((char *)usb_tx_buffer,
                               sizeof(usb_tx_buffer),
                               "%s\r\n", sd_report);
                sending_sd = true;
            }
            else if (imu_status_pending)
            {
                len = snprintf((char *)usb_tx_buffer,
                               sizeof(usb_tx_buffer),
                               "[IMU] %s, WHO_AM_I=0x%02X\r\n",
                               imu_ready ? "Ready (+/-2g, +/-250dps, 120 Hz)" : "INIT FAILED",
                               (unsigned int)imu_id);
                sending_imu_status = true;
            }
            else
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    if (button_pending[i])
                    {
                        button_idx = (int8_t)i;
                        len = snprintf((char *)usb_tx_buffer,
                                       sizeof(usb_tx_buffer),
                                       "[BUTTON] BTN%u pressed!\r\n",
                                       (unsigned int)i);
                        break;
                    }
                }

                if (button_idx < 0 && imu_report_pending)
                {
                    len = snprintf((char *)usb_tx_buffer,
                                   sizeof(usb_tx_buffer), "%s", imu_report);
                    sending_imu = true;
                }
                else if (button_idx < 0 &&
                         (uint32_t)(now - last_tx_ms) >= 1000U)
                {
                    len = snprintf((char *)usb_tx_buffer,
                                   sizeof(usb_tx_buffer),
                                   "[RJ RoboCup] Hello World! Robot ID: %u, Uptime: %lu seconds\r\n",
                                   (unsigned int)robot_id,
                                   (unsigned long)(now / 1000U));
                    sending_hello = true;
                }
            }

            if (len > 0 && (size_t)len < sizeof(usb_tx_buffer) &&
                CDC_Transmit_HS(usb_tx_buffer, (uint16_t)len) == USBD_OK)
            {
                if (sending_boot)
                    boot_message_sent = 1;
                if (sending_sd)
                    sd_report_pending = 0;
                if (sending_imu_status)
                    imu_status_pending = 0U;
                if (sending_imu)
                    imu_report_pending = 0U;
                if (button_idx >= 0)
                    button_pending[button_idx] = 0;
                if (sending_hello)
                    last_tx_ms = now;
            }
        }
    }

    if ((uint32_t)(now - last_led_toggle_ms) >= 500U)
    {
        led_on = !led_on;
        LED_SetColors(led_on);
        last_led_toggle_ms = now;
    }

    HAL_Delay(10);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 10;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the SYSCFG APB clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB2;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

/* USER CODE BEGIN 4 */

/*
 * Button interrupt callback.
 *
 * BTN0 = PE4  (SW4)
 * BTN1 = PC13 (SW5)
 * BTN2 = PC14 (SW6)
 *
 * Called on falling edge: button pressed.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t index;

    if (GPIO_Pin == USER_EXTI0_Pin)
    {
        index = 0;
    }
    else if (GPIO_Pin == USER_EXTI1_Pin)
    {
        index = 1;
    }
    else if (GPIO_Pin == USER_EXTI2_Pin)
    {
        index = 2;
    }
    else
    {
        return;
    }

    uint32_t now = HAL_GetTick();

    // Ignore subsequent edges within 30 ms.
    // First edge is always accepted.
    if (!button_seen[index] ||
        (uint32_t)(now - button_last_ms[index]) >= 30U)
    {
        button_seen[index] = 1;
        button_last_ms[index] = now;

        // Tell the main loop a button was pressed.
        button_pending[index] = 1;
    }
}


/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
