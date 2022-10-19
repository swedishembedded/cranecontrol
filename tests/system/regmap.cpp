#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "regmap.h"
void regmap_ko();

int printk(const char *fmt, ...)
{
	static char buf[80];

	va_list argptr;
	va_start(argptr, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, argptr);
	va_end(argptr);

	printf("%s", buf);

	return len;
}
}

class RegmapTest : public ::testing::Test {
    protected:
	virtual void SetUp()
	{
	}
};

struct test_obj {
	struct regmap_range range;
	char mem[64];
};

ssize_t _regmap_read(regmap_range_t range, uint32_t id, regmap_value_type_t type, void *value,
		     size_t size)
{
	struct test_obj *self = container_of(range, struct test_obj, range.ops);
	printf("read: %02x\n", id);
	switch (id) {
	case 0x10ff: {
		memcpy(value, self->mem, size);
		return size;
	}
	}
	return -EINVAL;
}

ssize_t _regmap_write(regmap_range_t range, uint32_t id, regmap_value_type_t type,
		      const void *value, size_t size)
{
	struct test_obj *self = container_of(range, struct test_obj, range.ops);
	printf("write: %02x\n", id);
	switch (id) {
	case 0x10ff: {
		memcpy(self->mem, value, size);
		return size;
	}
	}
	return -EINVAL;
}

TEST_F(RegmapTest, init)
{
	struct regmap *regmap = regmap_new();

	EXPECT_NE(regmap, nullptr);

	struct test_obj obj;
	static struct regmap_range_ops ops = { .read = _regmap_read, .write = _regmap_write };

	regmap_range_init(&obj.range, 0x1000, 0x2000, &ops);
	EXPECT_EQ(regmap_add_range(regmap, &obj.range), 0);

	// test read and write
	EXPECT_EQ(regmap_write_u32(regmap, 0x10ff, 1234), 4);
	uint32_t val = 0;
	EXPECT_EQ(regmap_read_u32(regmap, 0x10ff, &val), 4);
	EXPECT_EQ(val, 1234);

	regmap_delete(regmap);
}

TEST_F(RegmapTest, init_driver)
{
	char dtb[512];

	// a simple device tree to instantiate the driver
	fdt_create(dtb, sizeof(dtb));
	fdt_begin_node(dtb, "");
	fdt_begin_node(dtb, "regmap");
	fdt_property_string(dtb, "compatible", "fw,regmap");
	fdt_end_node(dtb);
	fdt_end_node(dtb);
	fdt_finish(dtb);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
