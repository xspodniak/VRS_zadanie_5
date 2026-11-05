#include "lps25hb.h"
#include <math.h>

static lps25hb_read_callback_t lps25hb_read = 0;
static lps25hb_write_callback_t lps25hb_write = 0;

static uint8_t lps25hb_initialized = 0;


void lps25hb_register_callbacks(lps25hb_read_callback_t read_callback, lps25hb_write_callback_t write_callback)
{
	lps25hb_read = read_callback;
	lps25hb_write = write_callback;
}

uint8_t lps25hb_read_registers(uint8_t register_address, uint8_t *data, uint8_t length)
{
	if(lps25hb_read == 0) return LPS25HB_ERROR;

	if(length > 1U) register_address |= LPS25HB_AUTO_INCREMENT;

	return lps25hb_read(LPS25HB_DEVICE_ADDRESS, register_address, data, length);
}

uint8_t lps25hb_write_registers(uint8_t register_address, uint8_t *data, uint8_t length)
{
	if(lps25hb_write == 0) return LPS25HB_ERROR;

	if(length > 1U) register_address |= LPS25HB_AUTO_INCREMENT;

	return lps25hb_write(LPS25HB_DEVICE_ADDRESS, register_address, data, length);
}

uint8_t lps25hb_read_register(uint8_t register_address, uint8_t *value)
{
	return lps25hb_read_registers(register_address, value, 1U);
}

uint8_t lps25hb_write_register(uint8_t register_address, uint8_t value)
{
	return lps25hb_write_registers(register_address, &value, 1U);
}

uint8_t lps25hb_init(void)
{
	uint8_t who_am_i = 0;

	lps25hb_initialized = 0;

	if(!lps25hb_read_register(LPS25HB_WHO_AM_I_ADDRESS, &who_am_i)) return LPS25HB_ERROR;
	if(who_am_i != LPS25HB_WHO_AM_I_VALUE) return LPS25HB_ERROR;

	if(!lps25hb_write_register(LPS25HB_CTRL_REG1, 0x00U)) return LPS25HB_ERROR;
	if(!lps25hb_write_register(LPS25HB_RES_CONF, LPS25HB_AVGT_16 | LPS25HB_AVGP_32)) return LPS25HB_ERROR;
	if(!lps25hb_write_register(LPS25HB_CTRL_REG2, LPS25HB_CTRL_REG2_FIFO_EN)) return LPS25HB_ERROR;
	if(!lps25hb_write_register(LPS25HB_FIFO_CTRL, LPS25HB_FIFO_MODE_MEAN | LPS25HB_FIFO_MEAN_8)) return LPS25HB_ERROR;
	if(!lps25hb_write_register(LPS25HB_CTRL_REG1, LPS25HB_CTRL_REG1_PD | LPS25HB_ODR_12_5HZ | LPS25HB_CTRL_REG1_BDU)) return LPS25HB_ERROR;

	lps25hb_initialized = 1;
	return LPS25HB_OK;
}

uint8_t lps25hb_is_pressure_ready(void)
{
	uint8_t status = 0;

	if(!lps25hb_read_register(LPS25HB_STATUS_REG, &status)) return 0;

	return (status & LPS25HB_STATUS_P_DA) ? 1U : 0U;
}

uint8_t lps25hb_get_pressure(float *pressure)
{
	uint8_t data[3];
	int32_t raw;

	if(!lps25hb_initialized || (pressure == 0)) return LPS25HB_ERROR;

	if(!lps25hb_read_registers(LPS25HB_PRESS_OUT_XL, data, 3U)) return LPS25HB_ERROR;

	raw = (int32_t)(((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | data[0]);
	if(raw & 0x00800000) raw |= (int32_t)0xFF000000;

	*pressure = (float)raw / LPS25HB_PRESSURE_SENSITIVITY;
	return LPS25HB_OK;
}

uint8_t lps25hb_get_temperature(float *temperature)
{
	uint8_t data[2];
	int16_t raw;

	if(!lps25hb_initialized || (temperature == 0)) return LPS25HB_ERROR;

	if(!lps25hb_read_registers(LPS25HB_TEMP_OUT_L, data, 2U)) return LPS25HB_ERROR;

	raw = (int16_t)(((uint16_t)data[1] << 8) | data[0]);

	*temperature = LPS25HB_TEMP_OFFSET + (float)raw / LPS25HB_TEMP_SENSITIVITY;
	return LPS25HB_OK;
}

float lps25hb_calculate_altitude(float pressure, float reference_pressure)
{
	if((pressure <= 0.0f) || (reference_pressure <= 0.0f)) return 0.0f;

	return 44330.0f * (1.0f - powf(pressure / reference_pressure, 1.0f / 5.255f));
}
