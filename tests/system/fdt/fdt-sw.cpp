#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "libfdt/libfdt.h"
}

class DeviceTreeSWTest : public ::testing::Test {
    protected:
	void SetUp()
	{
	}

	void TearDown()
	{
	}
};

TEST_F(DeviceTreeSWTest, create_device_tree)
{
	char buf[512];
	int val = 1234;
	EXPECT_EQ(fdt_create(buf, sizeof(buf)), 0);
	EXPECT_EQ(fdt_begin_node(buf, ""), 0);
	EXPECT_EQ(fdt_property_u32(buf, "prop_test", (uint32_t)val), 0);
	EXPECT_EQ(fdt_end_node(buf), 0);
	EXPECT_EQ(fdt_finish(buf), 0);
	EXPECT_EQ(fdt_get_int_or_default(buf, 0, "prop_test", -1), val);
	EXPECT_EQ(fdt_totalsize(buf), 90);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
