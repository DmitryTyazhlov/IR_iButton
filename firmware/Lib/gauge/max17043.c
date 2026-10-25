#include "max17043.h"

#include "main.h"

#define ADDR_DEFAULT 0x36
#define ADDR_ALTERNATIVE 0x76 /* seen answering on one real board */
#define REG_VCELL 0x02
#define REG_SOC 0x04
#define REG_VERSION 0x08
#define TIMEOUT_MS 20

/* 100 kHz with a 32 MHz I2C clock: PRESC 7, SCLDEL 4, SCLH 15, SCLL 19 */
#define I2C_TIMING 0x70400F13U

static I2C_HandleTypeDef hi2c1;

void HAL_I2C_MspInit(I2C_HandleTypeDef *handle) {
  if (handle->Instance == I2C1) __HAL_RCC_I2C1_CLK_ENABLE();
}

static uint32_t last_error;
static uint8_t address = ADDR_DEFAULT;
static uint16_t version_reg;
static uint16_t vcell_raw;

uint16_t max17043_vcell_raw(void) { return vcell_raw; }

uint8_t max17043_address(void) { return address; }
uint16_t max17043_version(void) { return version_reg; }

uint32_t max17043_last_error(void) { return last_error; }

uint8_t max17043_bus_levels(void) {
  uint8_t levels = 0;
  if (HAL_GPIO_ReadPin(BAT_SCL_GPIO_Port, BAT_SCL_Pin)) levels |= 1;
  if (HAL_GPIO_ReadPin(BAT_SDA_GPIO_Port, BAT_SDA_Pin)) levels |= 2;
  return levels;
}

uint8_t max17043_scan(uint8_t *found, uint8_t max) {
  uint8_t n = 0;
  for (uint8_t addr = 0x08; addr < 0x78 && n < max; addr++) {
    if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 5) == HAL_OK) {
      found[n++] = addr;
    }
  }
  return n;
}

static bool read_reg(uint8_t reg, uint16_t *value) {
  uint8_t buf[2];
  if (HAL_I2C_Mem_Read(&hi2c1, address << 1, reg, I2C_MEMADD_SIZE_8BIT, buf, 2,
                       TIMEOUT_MS) != HAL_OK) {
    last_error = HAL_I2C_GetError(&hi2c1);
    return false;
  }
  *value = ((uint16_t)buf[0] << 8) | buf[1];
  return true;
}

bool max17043_init(void) {
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = I2C_TIMING;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  last_error = 0;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
    last_error = 0x100;
    return false;
  }

  static const uint8_t candidates[] = {ADDR_DEFAULT, ADDR_ALTERNATIVE};
  for (uint8_t i = 0; i < sizeof(candidates); i++) {
    address = candidates[i];
    if (read_reg(REG_VERSION, &version_reg)) return true;
  }
  address = ADDR_DEFAULT;
  return false;
}

bool max17043_read_mv(uint16_t *mv) {
  uint16_t raw;
  if (!read_reg(REG_VCELL, &raw)) return false;
  vcell_raw = raw;
  /* 12 bit value in the upper bits, 1.25 mV per step */
  *mv = (uint16_t)(((uint32_t)(raw >> 4) * 5) / 4);
  return true;
}

bool max17043_read_percent(uint8_t *percent) {
  uint16_t raw;
  if (!read_reg(REG_SOC, &raw)) return false;
  uint8_t soc = raw >> 8;
  *percent = soc > 100 ? 100 : soc;
  return true;
}
