#include <gtest/gtest.h>
#include "skip_list.h"
#include <vector>

TEST(SkipListTest, BasicFunctionality) {
    skip_list<int> sl = {3, 1, 4, 5, 2};

    ASSERT_EQ(sl.size(), 5);

    ASSERT_NE(sl.find(1), sl.end());
    ASSERT_NE(sl.find(3), sl.end());
    ASSERT_NE(sl.find(4), sl.end());
    ASSERT_NE(sl.find(5), sl.end());

    std::vector<int> elements;
    for (int x : sl) {
        elements.push_back(x);
    }
    std::sort(elements.begin(), elements.end());
    ASSERT_EQ(elements, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(SkipListTest, InsertAndErase) {
    skip_list<int> sl;

    auto [it1, inserted1] = sl.insert(5);
    ASSERT_TRUE(inserted1);
    ASSERT_EQ(*it1, 5);

    auto [it2, inserted2] = sl.insert(2);
    ASSERT_TRUE(inserted2);
    ASSERT_EQ(*it2, 2);

    auto [it3, inserted3] = sl.insert(5);
    ASSERT_FALSE(inserted3);
    ASSERT_EQ(*it3, 5);

    ASSERT_EQ(sl.erase(5), 1);
    ASSERT_EQ(sl.erase(5), 0);
}

TEST(SkipListTest, CopyAndMove) {
    skip_list<int> sl1 = {1, 2, 3};

    skip_list<int> sl2 = sl1;
    ASSERT_EQ(sl1.size(), 3);
    ASSERT_EQ(sl2.size(), 3);

    skip_list<int> sl3 = std::move(sl1);
    ASSERT_EQ(sl1.size(), 0);
    ASSERT_EQ(sl3.size(), 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
