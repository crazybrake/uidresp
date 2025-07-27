#include <gtest/gtest.h>
#include "uidresp.h"

TEST(UidResponderTest, MatchExactLeftAndRight)
{
    EXPECT_TRUE(UidResponder::matches("12", "123456"));
    EXPECT_TRUE(UidResponder::matches("1256", "123456"));
    EXPECT_TRUE(UidResponder::matches("12bc56", "12abc56"));
    EXPECT_FALSE(UidResponder::matches("13", "123456"));
    EXPECT_FALSE(UidResponder::matches("1257", "123456"));
}

TEST(UidResponderTest, MatchEmptyInput)
{
    EXPECT_FALSE(UidResponder::matches("", "123456"));
}

TEST(UidResponderTest, MatchInputLongerThanUid)
{
    EXPECT_FALSE(UidResponder::matches("123456789", "123"));
}

TEST(UidResponderTest, CollisionLengthLimit)
{
    std::vector<std::string> uids = {
        "ABCDEF1234567890ZZZ",
        "XYZ1234567890QWERTY",
        "1112223334445556667"
    };

    std::string result = UidResponder::generateCollision(uids, 10);
    EXPECT_EQ(result.size(), 10u);
}

TEST(UidResponderTest, CollisionFromSingleUid)
{
    std::vector<std::string> uids = { "ABCDEF" };
    std::string result = UidResponder::generateCollision(uids, 6);
    EXPECT_EQ(result, "ABCDEF");
}

