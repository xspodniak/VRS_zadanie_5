#ifndef HTS221_H_
#define HTS221_H_

#include <stdint.h>

#define HTS221_DEVICE_ADDRESS		0xBEU

#define HTS221_WHO_AM_I_ADDRESS		0x0FU
#define HTS221_AV_CONF				0x10U
#define HTS221_CTRL_REG1			0x20U
#define HTS221_CTRL_REG2			0x21U
#define HTS221_CTRL_REG3			0x22U
#define HTS221_STATUS_REG			0x27U
#define HTS221_HUMIDITY_OUT_L		0x28U
#define HTS221_HUMIDITY_OUT_H		0x29U
#define HTS221_TEMP_OUT_L			0x2AU
#define HTS221_TEMP_OUT_H			0x2BU
#define HTS221_CALIB_START			0x30U
#define HTS221_CALIB_LENGTH			16U

#define HTS221_WHO_AM_I_VALUE		0xBCU

#define HTS221_AUTO_INCREMENT		0x80U

#define HTS221_AVGT_2				(0x0U << 3)
#define HTS221_AVGT_4				(0x1U << 3)
#define HTS221_AVGT_8				(0x2U << 3)
#define HTS221_AVGT_16				(0x3U << 3)
#define HTS221_AVGT_32				(0x4U << 3)
#define HTS221_AVGT_64				(0x5U << 3)
#define HTS221_AVGT_128				(0x6U << 3)
#define HTS221_AVGT_256				(0x7U << 3)
#define HTS221_AVGH_4				0x0U
#define HTS221_AVGH_8				0x1U
#define HTS221_AVGH_16				0x2U
#define HTS221_AVGH_32				0x3U
#define HTS221_AVGH_64				0x4U
#define HTS221_AVGH_128				0x5U
#define HTS221_AVGH_256				0x6U
#define HTS221_AVGH_512				0x7U

#define HTS221_CTRL_REG1_PD			0x80U
#define HTS221_CTRL_REG1_BDU		0x04U
#define HTS221_ODR_ONE_SHOT			0x00U
#define HTS221_ODR_1HZ				0x01U
#define HTS221_ODR_7HZ				0x02U
#define HTS221_ODR_12_5HZ			0x03U

#define HTS221_CTRL_REG2_BOOT		0x80U
#define HTS221_CTRL_REG2_HEATER		0x02U
#define HTS221_CTRL_REG2_ONE_SHOT	0x01U

#define HTS221_STATUS_H_DA			0x02U
#define HTS221_STATUS_T_DA			0x01U

#define HTS221_OK					1U
#define HTS221_ERROR				0U

typedef uint8_t (*hts221_read_callback_t)(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);
typedef uint8_t (*hts221_write_callback_t)(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);

void hts221_register_callbacks(hts221_read_callback_t read_callback, hts221_write_callback_t write_callback);

uint8_t hts221_init(void);

uint8_t hts221_read_registers(uint8_t register_address, uint8_t *data, uint8_t length);
uint8_t hts221_write_registers(uint8_t register_address, uint8_t *data, uint8_t length);
uint8_t hts221_read_register(uint8_t register_address, uint8_t *value);
uint8_t hts221_write_register(uint8_t register_address, uint8_t value);

uint8_t hts221_get_temperature(float *temperature);
uint8_t hts221_get_humidity(float *humidity);

#endif /* HTS221_H_ */
