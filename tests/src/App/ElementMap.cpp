// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include <App/Application.h>
#include "App/ElementMap.h"

// NOLINTBEGIN(readability-magic-numbers)

class ElementMapTest: public ::testing::Test
{
protected:
    void SetUp() override {
        int argc = 1;
        char *argv[] = { "FreeCAD" };
        App::Application::Config()["ExeName"] = "FreeCAD";
        App::Application::init(argc, argv);
        App::GetApplication().newDocument("test", "testUser");
    }
    // void TearDown() override {}
};

TEST_F(ElementMapTest, defaultConstruction)
{
    // Act
    Data::ElementMap elementMap = Data::ElementMap();

    // Assert
    EXPECT_EQ(elementMap.size(), 0);
}

//TEST_F(ElementMapTest, setElementNameDefaults)
//{
//    // Arrange
//    Data::ElementMapPtr currentElementMap = std::make_shared<Data::ElementMap>();
//    Data::MappedName mappedName("TEST");
//    Data::IndexedName element("Edge", 1);
//    Data::ElementIDRefs _sid;
//    auto sids = &_sid;
//    Data::MappedName expectedName("TEST");
//
//    // Act
//    auto newName = elementMap.setElementName(element, mappedName, currentElementMap, 0);
//
//    // Assert
//    EXPECT_EQ(newName, expectedName);
//}

// NOLINTEND(readability-magic-numbers)
