// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include "App/LazyClass.hpp"
#include "MemTrace.hpp"

#include <string>


#define LAZY_STR_ALLOC MemStats(1, 48)
#define LAZY_NAME_ALLOC MemStats(1, 40)

// Must be longer than 15 otherwise std::string won't allocate on heap
#define VERY_LONG_STRING "TESTTESTTESTTEST"
#define VERY_LONG_STRING_SIZE 16
#define VERY_LONG_STRING_ALLOC MemStats(1, VERY_LONG_STRING_SIZE + 1)


using LazyString = Lazy<std::string>;

class FakeMappedName
{
public:
    FakeMappedName() = default;
    FakeMappedName(const LazyString& name)
        : data(name)
    {}

    FakeMappedName(const FakeMappedName& other) = default;
    FakeMappedName& operator=(const FakeMappedName& other) = default;

    FakeMappedName(FakeMappedName&& other) = default;
    FakeMappedName& operator=(FakeMappedName&& other) = default;

    // example const function
    std::string GetName() const
    {
        return data.asConst();
    }

    // example non-const function
    void Append(const std::string& postfix)
    {
        data.asMutable() += postfix;
    }

private:
    LazyString data;
    int otherstuff = 0;
};

using LazyName = Lazy<FakeMappedName>;


/*---------------------------------------------------------------------------------------------------------------*/

TEST(LazyClass, basicAllocTest)
{
    RESET_MEM();

    int* ptr = new int[5];
    EXPECT_MEM(MemStats(1, 5 * sizeof(int)));

    delete[] ptr;
    EXPECT_MEM(MemStats(0, 0));
}

TEST(LazyClass, stringAllocTest)
{
    RESET_MEM();

    {
        std::string test = VERY_LONG_STRING;
        EXPECT_ALLOC(VERY_LONG_STRING_ALLOC);
    }

    EXPECT_DEALLOC(VERY_LONG_STRING_ALLOC);
}

TEST(LazyClass, stringCopy)
{
    RESET_MEM();

    std::string test = VERY_LONG_STRING;
    EXPECT_ALLOC(VERY_LONG_STRING_ALLOC);

    // Copy = alloc

    std::string test2 = test;
    EXPECT_ALLOC(VERY_LONG_STRING_ALLOC);

    std::string test3 = test;
    EXPECT_ALLOC(VERY_LONG_STRING_ALLOC);
}

TEST(LazyClass, lazyStringCopy)
{
    RESET_MEM();

    LazyString test("TEST");// sizeof("TEST") = 5  ==>  string will not allocate on heap
    EXPECT_ALLOC(LAZY_STR_ALLOC);

    // lazy copy = no alloc, only refcount increment

    LazyString test2 = test;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test2.isUnshared());

    LazyString test3 = test;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test3.isUnshared());

    // create a copy: another shared_ptr allocation
    test2.ensureUnshared();
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test2.isUnshared());

    // access object without modification: no new allocations
    EXPECT_EQ(test3.asConst().size(), 4);
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test3.isUnshared());

    // modify object: copy gets created, another shared_ptr allocation
    test3.asMutable() += "ANOTHER";
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test3.isUnshared());
    EXPECT_EQ(test3.asConst(), "TESTANOTHER");
}


TEST(LazyClass, lazyStringCopyLongString)
{
    RESET_MEM();

    LazyString test(VERY_LONG_STRING);
    EXPECT_ALLOC(LAZY_STR_ALLOC + VERY_LONG_STRING_ALLOC);

    LazyString test2 = test;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test2.isUnshared());

    LazyString test3 = test;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test3.isUnshared());

    // create a copy: new string gets allocated, with consequent shared_ptr allocation
    test2.ensureUnshared();
    EXPECT_ALLOC(LAZY_STR_ALLOC + VERY_LONG_STRING_ALLOC);
    EXPECT_TRUE(test2.isUnshared());

    // access object without modification: no new allocations
    EXPECT_EQ(test3.asConst().size(), VERY_LONG_STRING_SIZE);
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test3.isUnshared());

    // modify object:
    // copy gets created: +1 shared_ptr, +1 string alloc
    // +1 string allocation to expand storage
    // -1 dealloc of previous VERY_LONG_STRING_ALLOC
    test3.asMutable() += "ANOTHER";
    EXPECT_ALLOC(LAZY_STR_ALLOC
                 + MemStats(1, VERY_LONG_STRING_SIZE + 17));// 17 = overprovisioning?? magic!
    EXPECT_TRUE(test3.isUnshared());
    EXPECT_EQ(test3.asConst(), std::string(VERY_LONG_STRING) + "ANOTHER");
}


TEST(LazyClass, LazyConstructors)
{
    RESET_MEM();

    // default constructor
    LazyString test;
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test.isUnshared());

    // can use any of the underlying type constructors
    LazyString test2("TEST");
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test2.isUnshared());

    LazyString test3 = "TEST";
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test3.isUnshared());

    LazyString test4(std::string("TEST"));
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test4.isUnshared());

    const LazyString const_test5 = std::string("TEST");
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(const_test5.isUnshared());

    // non const copy
    LazyString test6(test4);
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test6.isUnshared());

    LazyString test7 = test4;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test7.isUnshared());

    // const copy
    LazyString test8(const_test5);
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test8.isUnshared());

    LazyString test9 = const_test5;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(test9.isUnshared());

    // move
    LazyString test10(LazyString("TEST"));
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test10.isUnshared());

    LazyString test11 = LazyString("TEST");
    EXPECT_ALLOC(LAZY_STR_ALLOC);
    EXPECT_TRUE(test11.isUnshared());
}

TEST(LazyClass, LazyMappedName)
{
    RESET_MEM();

    LazyName name("TEST");
    // 1 ptr for MappedName and 1 ptr for string
    EXPECT_ALLOC(LAZY_STR_ALLOC + LAZY_NAME_ALLOC);
    EXPECT_TRUE(name.isUnshared());
    EXPECT_EQ(name.asConst().GetName(), "TEST");

    LazyName name2 = name;
    EXPECT_NO_ALLOC();
    EXPECT_FALSE(name2.isUnshared());
    EXPECT_EQ(name2.asConst().GetName(), "TEST");

    name2.ensureUnshared();
    // ptr allocation to copy wrapped mappedname
    // during copy lazystring's copy constructor gets called, incrementing refcount
    EXPECT_ALLOC(LAZY_NAME_ALLOC);// caused by Lazy<FakeMappedName>

    name2.asMutable().Append("POSTFIX");
    // to append to lazystring asMutable needs to be called on it, creating a copy.
    EXPECT_ALLOC(LAZY_STR_ALLOC);// caused by Lazy<std::string>
    EXPECT_TRUE(name2.isUnshared());
    EXPECT_EQ(name2.asConst().GetName(), "TESTPOSTFIX");

    // original name remains unchanged
    EXPECT_EQ(name.asConst().GetName(), "TEST");
}

TEST(LazyClass, modifyFirstCopy)
{
    RESET_MEM();

    LazyString test("TEST");
    EXPECT_TRUE(test.isUnshared());

    LazyString test2 = test;
    EXPECT_FALSE(test2.isUnshared());

    LazyString test3 = test;
    EXPECT_FALSE(test3.isUnshared());

    EXPECT_EQ(test.asConst(), "TEST");
    EXPECT_EQ(test2.asConst(), "TEST");
    EXPECT_EQ(test3.asConst(), "TEST");

    test2.asMutable() += "another";

    EXPECT_EQ(test.asConst(), "TEST");
    EXPECT_EQ(test2.asConst(), "TESTanother");
    EXPECT_EQ(test3.asConst(), "TEST");

    test.asMutable() += "azz";// modify first owner here

    EXPECT_EQ(test.asConst(), "TESTazz");
    EXPECT_EQ(test2.asConst(), "TESTanother");
    EXPECT_EQ(test3.asConst(), "TEST");
}
