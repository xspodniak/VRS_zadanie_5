#include "hts221.h"

static hts221_read_callback_t hts221_read = 0;
static hts221_write_callback_t hts221_write = 0;

static struct
{
	float h0_rh;
	float h1_rh;
	float t0_degc;
	float t1_degc;
	int16_t h0_t0_out;
	int16_t h1_t0_out;
	int16_t t0_out;
	int16_t t1_out;
} hts221_calib;

static uint8_t hts221_calibrated = 0;


void hts221_register_callbacks(hts221_read_callback_t read_callback, hts221_write_callback_t write_callback)
{
	hts221_read = read_callback;
	hts221_write = write_callback;
}

uint8_t hts221_read_registers(uint8_t register_address, uint8_t *data, uint8_t length)
{
	if(hts221_read == 0) return HTS221_ERROR;

	if(length > 1U) register_address |= HTS221_AUTO_INCREMENT;

	return hts221_read(HTS221_DEVICE_ADDRESS, register_address, data, length);
}

uint8_t hts221_write_registers(uint8_t register_address, uint8_t *data, uint8_t length)
{
	if(hts221_write == 0) return HTS221_ERROR;

	if(length > 1U) register_address |= HTS221_AUTO_INCREMENT;

	return hts221_write(HTS221_DEVICE_ADDRESS, register_address, data, length);
}

uint8_t hts221_read_register(uint8_t register_address, uint8_t *value)
{
	return hts221_read_registers(register_address, value, 1U);
}

uint8_t hts221_write_register(uint8_t register_address, uint8_t value)
{
	return hts221_write_registers(register_address, &value, 1U);
}

static uint8_t hts221_read_calibration(void)
{
	uint8_t c[HTS221_CALIB_LENGTH];

	if(!hts221_read_registers(HTS221_CALIB_START, c, HTS221_CALIB_LENGTH)) return HTS221_ERROR;

	hts221_calib.h0_rh = c[0x00] / 2.0f;
	hts221_calib.h1_rh = c[0x01] / 2.0f;

	hts221_calib.t0_degc = (float)((((uint16_t)c[0x05] & 0x03U) << 8) | c[0x02]) / 8.0f;
	hts221_calib.t1_degc = (float)((((uint16_t)c[0x05] & 0x0CU) << 6) | c[0x03]) / 8.0f;

	hts221_calib.h0_t0_out = (int16_t)(((uint16_t)c[0x07] << 8) | c[0x06]);
	hts221_calib.h1_t0_out = (int16_t)(((uint16_t)c[0x0B] << 8) | c[0x0A]);
	hts221_calib.t0_out    = (int16_t)(((uint16_t)c[0x0D] << 8) | c[0x0C]);
	hts221_calib.t1_out    = (int16_t)(((uint16_t)c[0x0F] << 8) | c[0x0E]);

	if((hts221_calib.t1_out == hts221_calib.t0_out) || (hts221_calib.h1_t0_out == hts221_calib.h0_t0_out))
	{
		return HTS221_ERROR;
	}

	return HTS221_OK;
}

uint8_t hts221_init(void)
{
	uint8_t who_am_i = 0;

	hts221_calibrated = 0;

	if(!hts221_read_register(HTS221_WHO_AM_I_ADDRESS, &who_am_i)) return HTS221_ERROR;
	if(who_am_i != HTS221_WHO_AM_I_VALUE) return HTS221_ERROR;

	if(!hts221_write_register(HTS221_AV_CONF, HTS221_AVGT_16 | HTS221_AVGH_32)) return HTS221_ERROR;
	if(!hts221_write_register(HTS221_CTRL_REG1, HTS221_CTRL_REG1_PD | HTS221_CTRL_REG1_BDU | HTS221_ODR_12_5HZ)) return HTS221_ERROR;

	if(!hts221_read_calibration()) return HTS221_ERROR;

	hts221_calibrated = 1;
	return HTS221_OK;
}

uint8_t hts221_get_temperature(float *temperature)
{
	uint8_t data[2];
	int16_t t_out;

	if(!hts221_calibrated || (temperature == 0)) return HTS221_ERROR;

	if(!hts221_read_registers(HTS221_TEMP_OUT_L, data, 2U)) return HTS221_ERROR;
	t_out = (int16_t)(((uint16_t)data[1] << 8) | data[0]);

	*temperature = hts221_calib.t0_degc +
				   (float)(t_out - hts221_calib.t0_out) * (hts221_calib.t1_degc - hts221_calib.t0_degc) /
				   (float)(hts221_calib.t1_out - hts221_calib.t0_out);

	return HTS221_OK;
}

uint8_t hts221_get_humidity(float *humidity)
{
	uint8_t data[2];
	int16_t h_out;
	float h;

	if(!hts221_calibrated || (humidity == 0)) return HTS221_ERROR;

	if(!hts221_read_registers(HTS221_HUMIDITY_OUT_L, data, 2U)) return HTS221_ERROR;
	h_out = (int16_t)(((uint16_t)data[1] << 8) | data[0]);

	h = hts221_calib.h0_rh +
		(float)(h_out - hts221_calib.h0_t0_out) * (hts221_calib.h1_rh - hts221_calib.h0_rh) /
		(float)(hts221_calib.h1_t0_out - hts221_calib.h0_t0_out);

	if(h < 0.0f) h = 0.0f;
	if(h > 100.0f) h = 100.0f;

	*humidity = h;
	return HTS221_OK;
}
