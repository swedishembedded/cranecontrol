#include <stdio.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "control/motion_profile.h"
#include "linux/linux_uart.h"
#include "linux/linux_serial.h"
}

class LinuxUartTest : public ::testing::Test {
    protected:
	virtual void SetUp()
	{
	}
};

TEST_F(LinuxUartTest, init)
{
	struct linux_uart *self = linux_uart_new();
	EXPECT_NE(self, nullptr);
	printf("uart pts: %s\n", linux_uart_get_ptsname(self));

	// create a serial port that we can use to communicate with the new uart!
	struct linux_serial *serial = linux_serial_new();
	EXPECT_EQ(linux_serial_connect(serial, linux_uart_get_ptsname(self), 115200), 0);

	// write some data and expect to read it back
	uint32_t val1 = 1234;
	uint32_t val2 = 0;
	EXPECT_EQ(linux_serial_write(serial, &val1, sizeof(val1), 100), 4);
	// read it back
	EXPECT_EQ(linux_uart_read(self, &val2, sizeof(val2), 100), 4);

	EXPECT_EQ(val1, val2);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
