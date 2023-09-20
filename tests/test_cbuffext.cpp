#include "../include/circular_buffer.h"

#include <gtest/gtest.h>

TEST(CBufferTestExtSuite, EmptyTest) {
    CircularBufferExt<int> buff;
    ASSERT_TRUE(buff.empty());
}

TEST(CBufferTestExtSuite, CapacityTest) {
    CircularBufferExt<int> buff1(5);
    CircularBufferExt<int> buff2(10);
    CircularBufferExt<int> buff3(11);
    CircularBufferExt<int> buff4(1);
    CircularBufferExt<int> buff5(12345);
    CircularBufferExt<int> buff6;
    
    ASSERT_TRUE(buff1.capacity() == 5);
    ASSERT_TRUE(buff2.capacity() == 10);
    ASSERT_TRUE(buff3.capacity() == 11);
    ASSERT_TRUE(buff4.capacity() == 1);
    ASSERT_TRUE(buff5.capacity() == 12345);
    ASSERT_TRUE(buff6.capacity() == 0);
}

TEST(CBufferTestExtSuite, SizeTest) {
    CircularBufferExt<int> buff1(5);
    CircularBufferExt<int> buff2(10);
    CircularBufferExt<int> buff3(11);
    CircularBufferExt<int> buff4(1);
    CircularBufferExt<int> buff5(12345);
    CircularBufferExt<int> buff6;
    
    ASSERT_TRUE(buff1.size() == 5);
    ASSERT_TRUE(buff2.size() == 10);
    ASSERT_TRUE(buff3.size() == 11);
    ASSERT_TRUE(buff4.size() == 1);
    ASSERT_TRUE(buff5.size() == 12345);
    ASSERT_TRUE(buff6.size() == 0);
}

TEST(CBufferTestExtSuite, EqualityTest) {
    CircularBufferExt<int> buff1(5);
    CircularBufferExt<int> buff2(buff1);

    ASSERT_TRUE(buff1 == buff2);

    buff2.push_back(23456);

    ASSERT_FALSE(buff1 == buff2);

    buff1 = {1, 2, 3};
    buff2 = {3, 2, 1};

    ASSERT_FALSE(buff1 == buff2);
    sort(buff2.begin(), buff2.end());
    ASSERT_TRUE(buff1 == buff2);
}

TEST(CBufferTestExtSuite, InitializerListTest) {
    CircularBufferExt<int> buff({213, 123, 213});

    ASSERT_TRUE(buff[0] == 213);
    ASSERT_TRUE(buff[1] == 123);
    ASSERT_TRUE(buff[2] == 213);
    ASSERT_TRUE(buff.size() == 3);
}

TEST(CBufferTestExtSuite, InitializerListOperatorTest) {
    CircularBufferExt<int> buff;

    ASSERT_TRUE(buff.size() == 0);
    ASSERT_TRUE(buff.empty());

    buff = {213, 123, 213};

    ASSERT_TRUE(buff[0] == 213);
    ASSERT_TRUE(buff[1] == 123);
    ASSERT_TRUE(buff[2] == 213);
    ASSERT_TRUE(buff.size() == 3);
}

TEST(CBufferTestExtSuite, IteratorTest) {
    CircularBufferExt<int> buff({123, 3, 4, 5});

    ASSERT_TRUE(*buff.begin() == 123);
    ASSERT_TRUE(*(buff.end() - 1) == 5);
    ASSERT_TRUE(*(buff.begin() + 1) == 3);

    buff = {0, 1, 2, 3};

    ASSERT_TRUE(*buff.begin() == 0);
    ASSERT_TRUE(*(buff.begin() + 1) == 1);
    ASSERT_TRUE(*(buff.begin() + 2) == 2);
    ASSERT_TRUE(*(buff.begin() + 3) == 3);
}

TEST(CBufferTestExtSuite, SwapTest) {
    CircularBufferExt<int> base1({1, 2, 3});
    CircularBufferExt<int> base2({3, 2, 1});

    CircularBufferExt<int> a = base1;
    CircularBufferExt<int> b = base2;

    ASSERT_TRUE(a == base1 && b == base2);
    swap(a, b);
    ASSERT_TRUE(a == base2 && b == base1);
}

TEST(CBufferTestExtSuite, ConstructorTest) {
    CircularBufferExt<int> a(5, 10);
    CircularBufferExt<int> check1 = {10, 10, 10, 10, 10};
    CircularBufferExt<int> check2 = {10, 10, 10, 10};

    ASSERT_TRUE(a == check1);
    ASSERT_TRUE(a != check2);

    CircularBufferExt<std::vector<std::string>> v(5, std::vector<std::string>(5, "heh"));
    CircularBufferExt<std::vector<std::string>> check3 = {
        {"heh", "heh", "heh", "heh", "heh",},
        {"heh", "heh", "heh", "heh", "heh",},
        {"heh", "heh", "heh", "heh", "heh",},
        {"heh", "heh", "heh", "heh", "heh",},
        {"heh", "heh", "heh", "heh", "heh",}
    };

    ASSERT_TRUE(v == check3);
}

TEST(CBufferTestExtSuite, InsertTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    a.insert(a.begin() + 1, 5);

    CircularBufferExt<int> b;
    b.reserve(5);
    b.push_back(1);
    b.push_back(2);
    b.push_back(3);
    b.insert(b.begin() + 1, 5);

    CircularBufferExt<int> c;
    c.reserve(5);
    c.insert(c.begin(), 3, 4);

    CircularBufferExt<int> d;
    d.reserve(5);
    d.insert(d.begin(), 5, 4);

    ASSERT_TRUE(a == CircularBufferExt<int>({1, 5, 2, 3, 4, 5}));
    ASSERT_TRUE(b == CircularBufferExt<int>({1, 5, 2, 3}));
    ASSERT_TRUE(c == CircularBufferExt<int>({4, 4, 4}));
    ASSERT_TRUE(d == CircularBufferExt<int>({4, 4, 4, 4, 4}));
}

TEST(CBufferTestExtSuite, EraseTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    auto it = a.erase(a.begin());

    ASSERT_TRUE(a == CircularBufferExt<int>({2, 3, 4, 5}));
    ASSERT_TRUE(*it == 2);

    it = a.erase(a.begin(), a.begin() + 2);

    ASSERT_TRUE(a == CircularBufferExt<int>({4, 5}));
    ASSERT_TRUE(*it == 4);
}

TEST(CBufferTestExtSuite, ClearTest) {
    CircularBufferExt<std::string> v = {"hey", "have", "a", "good", "day"};
    v.clear();

    ASSERT_TRUE(v.empty());
}

TEST(CBufferTestExtSuite, AssignTest) {
    CircularBufferExt<std::string> v = {"hey", "have", "a", "good", "day"};

    ASSERT_TRUE(v.size() == 5);

    v.assign(3, "woyoyoy");

    ASSERT_TRUE(v == CircularBufferExt<std::string>({"woyoyoy", "woyoyoy", "woyoyoy"}));

    std::initializer_list<std::string> temp = {"heh", "heh", "why"};
    v.assign(temp);

    ASSERT_TRUE(v == CircularBufferExt<std::string>(temp));

    v.assign(temp.begin() + 1, temp.end());

    ASSERT_TRUE(v == CircularBufferExt<std::string>({"heh", "why"}));
}

TEST(CBufferTestExtSuite, FrontTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    ASSERT_TRUE(a.front() == 1);
}

TEST(CBufferTestExtSuite, BackTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    ASSERT_TRUE(a.back() == 5);
}

TEST(CBufferTestExtSuite, PushBackTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    
    a.push_back(6);

    ASSERT_TRUE(a == CircularBufferExt<int>({1, 2, 3, 4, 5, 6}));
}

TEST(CBufferTestExtSuite, PushFrontTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    
    a.push_front(6);

    ASSERT_TRUE(a == CircularBufferExt<int>({6, 1, 2, 3, 4, 5}));
}

TEST(CBufferTestExtSuite, PopBackTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    
    a.pop_back();

    ASSERT_TRUE(a == CircularBufferExt<int>({1, 2, 3, 4}));
}

TEST(CBufferTestExtSuite, PopFrontTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    
    a.pop_front();

    ASSERT_TRUE(a == CircularBufferExt<int>({2, 3, 4, 5}));
}

TEST(CBufferTestExtSuite, AccessOperatorTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    
    for (size_t i = 0; i < a.size(); ++i) {
        ASSERT_TRUE(a[i] == i + 1);
    }
}

TEST(CBufferTestExtSuite, AtTest) {
    CircularBufferExt<int> a({1, 2, 3, 4, 5});
    
    for (size_t i = 0; i < a.size(); ++i) {
        ASSERT_TRUE(a.at(i) == i + 1);
    }
}
