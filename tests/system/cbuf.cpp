/** :ms-top-comment
 *  _     ___ ____  _____ ___ ____  __  ____        ___    ____  _____
 * | |   |_ _| __ )|  ___|_ _|  _ \|  \/  \ \      / / \  |  _ \| ____|
 * | |    | ||  _ \| |_   | || |_) | |\/| |\ \ /\ / / _ \ | |_) |  _|
 * | |___ | || |_) |  _|  | ||  _ <| |  | | \ V  V / ___ \|  _ <| |___
 * |_____|___|____/|_|   |___|_| \_\_|  |_|  \_/\_/_/   \_\_| \_\_____|
 *
 * Copyright (c) 2020, Martin K. Schröder, All Rights Reserved
 *
 * This library is distributed under LGPLv2
 *
 * Commercial licensing: http://swedishembedded.com/code
 * Contact: info@swedishembedded.com
 **/
#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "libfirmware/types/cbuf.h"
}

class CbufTest : public ::testing::Test {
protected:
	struct cbuf cbuf;
	char data[16];
	void SetUp() {
		cbuf_init(&cbuf, data, 1, sizeof(data));
	}

	void TearDown() {
	}
};

int main(int argc, char **argv) {
	::testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}

TEST_F(CbufTest, initial_state){
	EXPECT_TRUE(cbuf_empty(&cbuf));
	EXPECT_EQ(cbuf_size(&cbuf), 0);
	EXPECT_EQ(cbuf_capacity(&cbuf), sizeof(data));
}

TEST_F(CbufTest, read_write){
	char ch = 'A';
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	EXPECT_FALSE(cbuf_empty(&cbuf));
	for(unsigned c = 0; c < sizeof(data) - 2; c++){
		ch = 'B' + c;
		EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	}
	EXPECT_FALSE(cbuf_full(&cbuf));
	EXPECT_EQ(cbuf_size(&cbuf), sizeof(data) - 1);
	ch = 'C';
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	EXPECT_TRUE(cbuf_full(&cbuf));
	EXPECT_EQ(cbuf_size(&cbuf), cbuf_capacity(&cbuf));
	// at this point the first element is discarded and overwritten by the new element
	ch = 'D';
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 1);
	EXPECT_TRUE(cbuf_full(&cbuf));
	EXPECT_FALSE(cbuf_empty(&cbuf));
	EXPECT_EQ(cbuf_size(&cbuf), cbuf_capacity(&cbuf));
	// now getting an element should return 'B' since 'A' was overwritten
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'B');
	EXPECT_FALSE(cbuf_full(&cbuf));
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'C');
	EXPECT_FALSE(cbuf_full(&cbuf));
	// now test putting unless full
	ch = 'X';
	EXPECT_EQ(cbuf_put_unless_full(&cbuf, &ch), 0);
	ch = 'Y';
	EXPECT_EQ(cbuf_put_unless_full(&cbuf, &ch), 0);
	// Z will not fit so an error is returned
	ch = 'Z';
	EXPECT_EQ(cbuf_put_unless_full(&cbuf, &ch), -1);
	// now read back all the lements in the buffer
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'D');
	for(unsigned c = 0; c < sizeof(data) - 5; c++){
		EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
		EXPECT_EQ(ch, 'E' + c);
	}
	// these are the C and D that we wrote before
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'C');
	EXPECT_FALSE(cbuf_empty(&cbuf));
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'D');
	// and now come the X and Y
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'X');
	EXPECT_FALSE(cbuf_empty(&cbuf));
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(ch, 'Y');
	// at this point the buffer should be empty
	EXPECT_TRUE(cbuf_empty(&cbuf));
	// getting from an empty buffer should return zero
	EXPECT_EQ(cbuf_get(&cbuf, NULL), 0);
}

TEST_F(CbufTest, write_past_end){
	char ch = 'A';
	// this tests writing past end where head and tail become reversed
	// fill the buffer leaving one character left
	for(unsigned c = 0; c < sizeof(data) - 1; c++){
		EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	}
	EXPECT_EQ(cbuf_size(&cbuf), sizeof(data) - 1);
	// writing one more will wrap head pointer
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	EXPECT_EQ(cbuf_size(&cbuf), sizeof(data));
	EXPECT_TRUE(cbuf_full(&cbuf));
	// get a few characters to move head pointer forward
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(cbuf_get(&cbuf, &ch), 1);
	EXPECT_EQ(cbuf_size(&cbuf), sizeof(data) - 2);
	// check that size still works in this condition
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	EXPECT_EQ(cbuf_size(&cbuf), sizeof(data) - 1);
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	EXPECT_EQ(cbuf_size(&cbuf), sizeof(data));
}

TEST_F(CbufTest, clear){
	char ch = 'A';
	EXPECT_TRUE(cbuf_empty(&cbuf));
	EXPECT_EQ(cbuf_put(&cbuf, &ch), 0);
	EXPECT_FALSE(cbuf_empty(&cbuf));
	cbuf_clear(&cbuf);
	EXPECT_TRUE(cbuf_empty(&cbuf));
}
