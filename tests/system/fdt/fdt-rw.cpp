#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "libfdt/libfdt.h"
}

class DeviceTreeRWTest : public ::testing::Test {
protected:
	void SetUp() {
	}

	void TearDown() {
	}
};

TEST_F(DeviceTreeRWTest, insert_node){
	char buf[512];
	char buf2[512];
	int val = 1234;
	EXPECT_EQ(fdt_create(buf, sizeof(buf)), 0);
	EXPECT_EQ(fdt_finish_reservemap(buf), 0);
	EXPECT_EQ(fdt_begin_node(buf, ""), 0);
	EXPECT_EQ(fdt_property_u32(buf, "prop_test", (uint32_t)val), 0);
	EXPECT_EQ(fdt_begin_node(buf, "subnode"), 0);
	EXPECT_EQ(fdt_end_node(buf), 0);
	EXPECT_EQ(fdt_end_node(buf), 0);
	EXPECT_EQ(fdt_finish(buf), 0);
	EXPECT_EQ(fdt_totalsize(buf), 122);

	EXPECT_EQ(fdt_open_into(buf, buf2, sizeof(buf)), 0);

	int first = fdt_first_subnode(buf2, 0);
	EXPECT_EQ(first, 24);
	int node = fdt_path_offset(buf, "/subnode");
	EXPECT_GE(node, 0);

	//printf("error: %s\n", fdt_strerror(fdt_appendprop_u32(buf, node, "newprop", 123)));

	EXPECT_EQ(fdt_appendprop_u32(buf2, node, "newprop", 123), 0);
	EXPECT_EQ(fdt_pack(buf2), 0);

	EXPECT_EQ(fdt_totalsize(buf2), 138);

	EXPECT_EQ(fdt_get_int_or_default(buf2, node, "newprop", -1), 123);
}

int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
