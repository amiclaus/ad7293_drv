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

struct ad7293_dev {
	struct spi_device	*spi;
	struct regmap		*regmap;
};

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
