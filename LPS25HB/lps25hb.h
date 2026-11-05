#ifndef LPS25HB_H_
#define LPS25HB_H_

#include <stdint.h>

#define LPS25HB_DEVICE_ADDRESS		0xBAU

#define LPS25HB_REF_P_XL			0x08U
#define LPS25HB_REF_P_L				0x09U
#define LPS25HB_REF_P_H				0x0AU
#define LPS25HB_WHO_AM_I_ADDRESS	0x0FU
#define LPS25HB_RES_CONF			0x10U
#define LPS25HB_CTRL_REG1			0x20U
#define LPS25HB_CTRL_REG2			0x21U
#define LPS25HB_CTRL_REG3			0x22U
#define LPS25HB_CTRL_REG4			0x23U
#define LPS25HB_INTERRUPT_CFG		0x24U
#define LPS25HB_INT_SOURCE			0x25U
#define LPS25HB_STATUS_REG			0x27U
#define LPS25HB_PRESS_OUT_XL		0x28U
#define LPS25HB_PRESS_OUT_L			0x29U
#define LPS25HB_PRESS_OUT_H			0x2AU
#define LPS25HB_TEMP_OUT_L			0x2BU
#define LPS25HB_TEMP_OUT_H			0x2CU
#define LPS25HB_FIFO_CTRL			0x2EU
#define LPS25HB_FIFO_STATUS			0x2FU
#define LPS25HB_THS_P_L				0x30U
#define LPS25HB_THS_P_H				0x31U
#define LPS25HB_RPDS_L				0x39U
#define LPS25HB_RPDS_H				0x3AU

#define LPS25HB_WHO_AM_I_VALUE		0xBDU

#define LPS25HB_AUTO_INCREMENT		0x80U

#define LPS25HB_AVGT_8				(0x0U << 2)
#define LPS25HB_AVGT_16				(0x1U << 2)
#define LPS25HB_AVGT_32				(0x2U << 2)
#define LPS25HB_AVGT_64				(0x3U << 2)
#define LPS25HB_AVGP_8				0x0U
#define LPS25HB_AVGP_32				0x1U
#define LPS25HB_AVGP_128			0x2U
#define LPS25HB_AVGP_512			0x3U

#define LPS25HB_CTRL_REG1_PD		0x80U
#define LPS25HB_ODR_ONE_SHOT		(0x0U << 4)
#define LPS25HB_ODR_1HZ				(0x1U << 4)
#define LPS25HB_ODR_7HZ				(0x2U << 4)
#define LPS25HB_ODR_12_5HZ			(0x3U << 4)
#define LPS25HB_ODR_25HZ			(0x4U << 4)
#define LPS25HB_CTRL_REG1_DIFF_EN	0x08U
#define LPS25HB_CTRL_REG1_BDU		0x04U
#define LPS25HB_CTRL_REG1_RESET_AZ	0x02U
#define LPS25HB_CTRL_REG1_SIM		0x01U

#define LPS25HB_CTRL_REG2_BOOT		0x80U
#define LPS25HB_CTRL_REG2_FIFO_EN	0x40U
#define LPS25HB_CTRL_REG2_SWRESET	0x04U
#define LPS25HB_CTRL_REG2_ONE_SHOT	0x01U

#define LPS25HB_FIFO_MODE_MEAN		(0x6U << 5)
#define LPS25HB_FIFO_MEAN_2			0x01U
#define LPS25HB_FIFO_MEAN_4			0x03U
#define LPS25HB_FIFO_MEAN_8			0x07U
#define LPS25HB_FIFO_MEAN_16		0x0FU
#define LPS25HB_FIFO_MEAN_32		0x1FU

#define LPS25HB_STATUS_P_DA			0x02U
#define LPS25HB_STATUS_T_DA			0x01U

#define LPS25HB_PRESSURE_SENSITIVITY	4096.0f
#define LPS25HB_TEMP_SENSITIVITY		480.0f
#define LPS25HB_TEMP_OFFSET				42.5f

#define LPS25HB_OK					1U
#define LPS25HB_ERROR				0U

typedef uint8_t (*lps25hb_read_callback_t)(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);
typedef uint8_t (*lps25hb_write_callback_t)(uint8_t slave_address, uint8_t register_address, uint8_t *data, uint8_t length);

void lps25hb_register_callbacks(lps25hb_read_callback_t read_callback, lps25hb_write_callback_t write_callback);

uint8_t lps25hb_init(void);

uint8_t lps25hb_read_registers(uint8_t register_address, uint8_t *data, uint8_t length);
uint8_t lps25hb_write_registers(uint8_t register_address, uint8_t *data, uint8_t length);
uint8_t lps25hb_read_register(uint8_t register_address, uint8_t *value);
uint8_t lps25hb_write_register(uint8_t register_address, uint8_t value);

uint8_t lps25hb_is_pressure_ready(void);

uint8_t lps25hb_get_pressure(float *pressure);
uint8_t lps25hb_get_temperature(float *temperature);

float lps25hb_calculate_altitude(float pressure, float reference_pressure);

#endif /* LPS25HB_H_ */
