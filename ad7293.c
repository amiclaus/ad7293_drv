// SPDX-License-Identifier: GPL-2.0-only
/*
 * AD7293 driver
 *
 * Copyright 2021 Analog Devices Inc.
 */

#include <linux/bitfield.h>
#include <linux/bits.h>
#include <linux/device.h>
#include <linux/iio/iio.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/spi/spi.h>

/* AD7293 Register Map Common */
#define AD7293_REG_NO_OP			0x00
#define AD7293_REG_PAGE_SELECT			0x01
#define AD7293_REG_CONV_CMD			0x02
#define AD7293_REG_RESULT			0x03
#define AD7293_REG_DAC_EN			0x04
#define AD7293_REG_DEVICE_ID			0x0C
#define AD7293_REG_SOFT_RESET			0x0F

/* AD7293 Register Map Page 0x00 */
#define AD7293_REG_VIN0				0x10
#define AD7293_REG_VIN1				0x11
#define AD7293_REG_VIN2				0x12
#define AD7293_REG_VIN3				0x13
#define AD7293_REG_TSENSE_INT			0x20
#define AD7293_REG_TSENSE_D0			0x21
#define AD7293_REG_TSENSE_D1			0x22
#define AD7293_REG_ISENSE_0			0x28
#define AD7293_REG_ISENSE_1			0x29
#define AD7293_REG_ISENSE_2			0x2A
#define AD7293_REG_ISENSE_3			0x2B
#define AD7293_REG_UNI_VOUT0			0x30
#define AD7293_REG_UNI_VOUT1			0x31
#define AD7293_REG_UNI_VOUT2			0x32
#define AD7293_REG_UNI_VOUT3			0x33
#define AD7293_REG_BI_VOUT0			0x34
#define AD7293_REG_BI_VOUT1			0x35
#define AD7293_REG_BI_VOUT2			0x36
#define AD7293_REG_BI_VOUT3			0x37

/* AD7293 Register Map Page 0x01 */
#define AD7293_REG_AVDD				0x10
#define AD7293_REG_DACVDD_UNI			0x11
#define AD7293_REG_DACVDD_BI			0x12
#define AD7293_REG_AVSS				0x13
#define AD7293_REG_BI_VOUT0_MON			0x14
#define AD7293_REG_BI_VIOU1_MON			0x15
#define AD7293_REG_BI_VOUT2_MON			0x16
#define AD7293_REG_BI_VOUT3_MON			0x17
#define AD7293_REG_RS0_MON			0x28
#define AD7293_REG_RS1_MON			0x29
#define AD7293_REG_RS2_MON			0x2A
#define AD7293_REG_RS3_MON			0x2B

/* AD7293 Register Map Page 0x02 */
#define AD7293_REG_DIGITAL_OUT_EN		0x11
#define AD7293_REG_DIGITAL_INOUT_FUNC		0x12
#define AD7293_REG_DIGITAL_FUNC_POL		0x13
#define AD7293_REG_GENERAL			0x14
#define AD7293_REG_VINX_RANGE0			0x15
#define AD7293_REG_VINX_RANGE1			0x16
#define AD7293_REG_VINX_DIFF_SE			0x17
#define AD7293_REG_VINX_FILTER			0x18
#define AD7293_REG_BG_EN			0x19
#define AD7293_REG_CONV_DELAY			0x1A
#define AD7293_REG_TSENSE_BG_EN			0x1B
#define AD7293_REG_ISENSE_BG_EN			0x1C
#define AD7293_REG_ISENSE_GAIN			0x1D
#define AD7293_REG_DAC_SNOOZE_O			0x1F
#define AD7293_REG_DAC_SNOOZE_1			0x20
#define AD7293_REG_RSX_MON_BG_EN		0x23
#define AD7293_REG_INTEGR_CL			0x28
#define AD7293_REG_PA_ON_CTRL			0x29
#define AD7293_REG_RAMP_TIME_0			0x2A
#define AD7293_REG_RAMP_TIME_1			0x2B
#define AD7293_REG_RAMP_TIME_2			0x2C
#define AD7293_REG_RAMP_TIME_3			0x2D
#define AD7293_REG_CL_FR_IT			0x2E
#define AD7293_REG_INTX_AVSS_AVDD		0x2F

/* AD7293 Register Map Page 0x03 */
#define AD7293_REG_VINX_SEQ			0x10
#define AD7293_REG_ISENSEX_TSENSEX_SEQ		0x11
#define AD7293_REG_RSX_MON_BI_VOUTX_SEQ		0x12

/* AD7293 Register Map Page 0x04 */
#define AD7293_REG_VIN0_HL			0x10
#define AD7293_REG_VIN1_HL			0x11
#define AD7293_REG_VIN2_HL			0x12
#define AD7293_REG_VIN3_HL			0x13
#define AD7293_REG_TSENSE_INT_HL		0x20
#define AD7293_REG_TSENSE_D0_HL			0x21
#define AD7293_REG_TSENSE_D1_HL			0x22
#define AD7293_REG_ISENSE0_HL			0x28
#define AD7293_REG_ISENSE1_HL			0x29
#define AD7293_REG_ISENSE2_HL			0x2A
#define AD7293_REG_ISENSE3_HL			0x2B

/* AD7293 Register Map Page 0x05 */
#define AD7293_REG_AVDD_HL			0x10
#define AD7293_REG_DACVDD_UNI_HL		0x11
#define AD7293_REG_DACVDD_BI_HL			0x12
#define AD7293_REG_AVSS_HL			0x13
#define AD7293_REG_BI_VOUT0_MON_HL		0x14
#define AD7293_REG_BI_VOUT1_MON_HL		0x15
#define AD7293_REG_BI_VOUT2_MON_HL		0x16
#define AD7293_REG_BI_VOUT3_MON_HL		0x17
#define AD7293_REG_RS0_MON_HL			0x28
#define AD7293_REG_RS1_MON_HL			0x29
#define AD7293_REG_RS2_MON			0x2A
#define AD7293_REG_RS3_MON			0x2B

/* AD7293 Register Map Page 0x06 */
#define AD7293_REG_VIN0_LL			0x10
#define AD7293_REG_VIN1_LL			0x11
#define AD7293_REG_VIN2_LL			0x12
#define AD7293_REG_VIN3_LL			0x13
#define AD7293_REG_TSENSE_D0_LL			0x20
#define AD7293_REG_TSENSE_D1_LL			0x21
#define AD7293_REG_TSENSE_D2_LL			0x22
#define AD7293_REG_ISENSE0_LL			0x28
#define AD7293_REG_ISENSE1_LL			0x29
#define AD7293_REG_ISENSE2_LL			0x2A
#define AD7293_REG_ISENSE3_LL			0x2B

/* AD7293 Register Map Page 0x07 */
#define AD7293_REG_AVDD_LL			0x10
#define AD7293_REG_DACVDD_UNI_LL		0x11
#define AD7293_REG_DACVDD_BI_LL			0x12
#define AD7293_REG_AVSS_LL			0x13
#define AD7293_REG_BI_VOUT0_MON_LL		0x14
#define AD7293_REG_BI_VOUT1_MON_LL		0x15
#define AD7293_REG_BI_VOUT2_MON_LL		0x16
#define AD7293_REG_BI_VOUT3_MON_LL		0x17
#define AD7293_REG_RS0_MON_LL			0x28
#define AD7293_REG_RS1_MON_LL			0x29
#define AD7293_REG_RS2_MON_LL			0x2A
#define AD7293_REG_RS3_MON_LL			0x2B

/* AD7293 Register Map Page 0x08 */
#define AD7293_REG_VIN0_HYS			0x10
#define AD7293_REG_VIN1_HYS			0x11
#define AD7293_REG_VIN2_HYS			0x12
#define AD7293_REG_VIN3_HYS			0x13
#define AD7293_REG_TSENSE_INT_HYS		0x20
#define AD7293_REG_TSENSE_D0_HYS		0x20
#define AD7293_REG_TSENSE_D1_HYS		0x20
#define AD7293_REG_ISENSE0_HYS			0x28
#define AD7293_REG_ISENSE1_HYS			0x29
#define AD7293_REG_ISENSE2_HYS			0x2A
#define AD7293_REG_ISENSE3_HYS			0x2B

/* AD7293 Register Map Page 0x09 */
#define AD7293_REG_AVDD_HYS			0x10
#define AD7293_REG_DACVDD_UNI_HYS		0x11
#define AD7293_REG_DACVDD_BI_HYS		0x12
#define AD7293_REG_AVSS_HYS			0x13
#define AD7293_REG_BI_VOUT0_MON_HYS		0x14
#define AD7293_REG_BI_VOUT1_MON_HYS		0x15
#define AD7293_REG_BI_VOUT2_MON_HYS		0x16
#define AD7293_REG_BI_VOUT3_MON_HYS		0x17
#define AD7293_REG_RS0_MON_HYS			0x28
#define AD7293_REG_RS1_MON_HYS			0x29
#define AD7293_REG_RS2_MON_HYS			0x2A
#define AD7293_REG_RS3_MON_HYS			0x2B

/* AD7293 Register Map Page 0x0A */
#define AD7293_REG_VIN0_MIN			0x10
#define AD7293_REG_VIN1_MIN			0x11
#define AD7293_REG_VIN2_MIN			0x12
#define AD7293_REG_VIN3_MIN			0x13
#define AD7293_REG_TSENSE_INT_MIN		0x20
#define AD7293_REG_TSENSE_D0_MIN		0x20
#define AD7293_REG_TSENSE_D1_MIN		0x20
#define AD7293_REG_ISENSE0_MIN			0x28
#define AD7293_REG_ISENSE1_MIN			0x29
#define AD7293_REG_ISENSE2_MIN			0x2A
#define AD7293_REG_ISENSE3_MIN			0x2B

/* AD7293 Register Map Page 0x0B */
#define AD7293_REG_AVDD_MIN			0x10
#define AD7293_REG_DACVDD_UNI_MIN		0x11
#define AD7293_REG_DACVDD_BI_MIN		0x12
#define AD7293_REG_AVSS_MIN			0x13
#define AD7293_REG_BI_VOUT0_MON_MIN		0x14
#define AD7293_REG_BI_VOUT1_MON_MIN		0x15
#define AD7293_REG_BI_VOUT2_MON_MIN		0x16
#define AD7293_REG_BI_VOUT3_MON_MIN		0x17
#define AD7293_REG_RS0_MON_MIN			0x28
#define AD7293_REG_RS1_MON_MIN			0x29
#define AD7293_REG_RS2_MON_MIN			0x2A
#define AD7293_REG_RS3_MON_MIN			0x2B

/* AD7293 Register Map Page 0x0C */
#define AD7293_REG_VIN0_MAX			0x10
#define AD7293_REG_VIN1_MAX			0x11
#define AD7293_REG_VIN2_MAX			0x12
#define AD7293_REG_VIN3_MAX			0x13
#define AD7293_REG_TSENSE_INT_MAX		0x20
#define AD7293_REG_TSENSE_D0_MAX		0x20
#define AD7293_REG_TSENSE_D1_MAX		0x20
#define AD7293_REG_ISENSE0_MAX			0x28
#define AD7293_REG_ISENSE1_MAX			0x29
#define AD7293_REG_ISENSE2_MAX			0x2A
#define AD7293_REG_ISENSE3_MAX			0x2B

/* AD7293 Register Map Page 0x0D */
#define AD7293_REG_AVDD_MAX			0x10
#define AD7293_REG_DACVDD_UNI_MAX		0x11
#define AD7293_REG_DACVDD_BI_MAX		0x12
#define AD7293_REG_AVSS_MAX			0x13
#define AD7293_REG_BI_VOUT0_MON_MAX		0x14
#define AD7293_REG_BI_VOUT1_MON_MAX		0x15
#define AD7293_REG_BI_VOUT2_MON_MAX		0x16
#define AD7293_REG_BI_VOUT3_MON_MAX		0x17
#define AD7293_REG_RS0_MON_MAX			0x28
#define AD7293_REG_RS1_MON_MAX			0x29
#define AD7293_REG_RS2_MON_MAX			0x2A
#define AD7293_REG_RS3_MON_MAX			0x2B

/* AD7293 Register Map Page 0x0E */
#define AD7293_REG_VIN0_OFFSET			0x10
#define AD7293_REG_VIN1_OFFSET			0x11
#define AD7293_REG_VIN2_OFFSET			0x12
#define AD7293_REG_VIN3_OFFSET			0x13
#define AD7293_REG_TSENSE_INT_OFFSET		0x20
#define AD7293_REG_TSENSE_D0_OFFSET		0x20
#define AD7293_REG_TSENSE_D1_OFFSET		0x20
#define AD7293_REG_ISENSE0_OFFSET		0x28
#define AD7293_REG_ISENSE1_OFFSET		0x29
#define AD7293_REG_ISENSE2_OFFSET		0x2A
#define AD7293_REG_ISENSE3_OFFSET		0x2B
#define AD7293_REG_UNI_VOUT0_OFFSET		0x30
#define AD7293_REG_UNI_VOUT1_OFFSET		0x31
#define AD7293_REG_UNI_VOUT2_OFFSET		0x32
#define AD7293_REG_UNI_VOUT3_OFFSET		0x33
#define AD7293_REG_BI_VOUT0_OFFSET		0x34
#define AD7293_REG_BI_VOUT1_OFFSET		0x35
#define AD7293_REG_BI_VOUT2_OFFSET		0x36
#define AD7293_REG_BI_VOUT3_OFFSET		0x37

/* AD7293 Register Map Page 0x0F */
#define AD7293_REG_AVDD_OFFSET			0x10
#define AD7293_REG_DACVDD_UNI_OFFSET		0x11
#define AD7293_REG_DACVDD_BI_OFFSET		0x12
#define AD7293_REG_AVSS_OFFSET			0x13
#define AD7293_REG_BI_VOUT0_MON_OFFSET		0x14
#define AD7293_REG_BI_VOUT1_MON_OFFSET		0x15
#define AD7293_REG_BI_VOUT2_MON_OFFSET		0x16
#define AD7293_REG_BI_VOUT3_MON_OFFSET		0x17
#define AD7293_REG_RS0_MON_OFFSET		0x28
#define AD7293_REG_RS1_MON_OFFSET		0x29
#define AD7293_REG_RS2_MON_OFFSET		0x2A
#define AD7293_REG_RS3_MON_OFFSET		0x2B

struct ad7293_dev {
	struct spi_device	*spi;
	struct regmap		*regmap;
};


/* AD7293 Register Map Page 3 */
static const struct regmap_config ad7293_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.read_flag_mask = BIT(7),
	.max_register = 0x12,
};

static int ad7293_init(struct ad7293_dev *dev)
{

}

static int ad7293_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct regmap *regmap;
	struct ad7293_dev *dev;
	int ret;

	regmap = devm_regmap_init_spi(spi, &ad7293_regmap_config);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	indio_dev->dev.parent = &spi->dev;
	indio_dev->info = &ad7293_info;
	indio_dev->name = "ad7293";

	dev = iio_priv(indio_dev);
	dev->regmap = regmap;

	dev->spi = spi;

	ret = ad7293_init(dev);
	if (ret)
		return ret;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static const struct spi_device_id ad7293_id[] = {
	{ "ad7293", 0 },
	{}
};
MODULE_DEVICE_TABLE(spi, ad7293_id);

static const struct of_device_id ad7293_of_match[] = {
	{ .compatible = "adi,ad7293" },
	{}
};
MODULE_DEVICE_TABLE(of, ad7293_of_match);

static struct spi_driver ad7293_driver = {
	.driver = {
		.name = "ad7293",
		.of_match_table = ad7293_of_match,
	},
	.probe = ad7293_probe,
	.id_table = ad7293_id,
};
module_spi_driver(ad7293_driver);

MODULE_AUTHOR("Antoniu Miclaus <antoniu.miclaus@analog.com");
MODULE_DESCRIPTION("Analog Devices AD7293");
MODULE_LICENSE("GPL v2");
