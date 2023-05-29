// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include <App/Application.h>
#include "App/ElementMap.h"

// NOLINTBEGIN(readability-magic-numbers)

class LessComplexPart
{
    // this is a "holder" class used for simpler testing of ElementMap in the context
    // of a class
public:
    mutable long Tag;
    Data::ElementMapPtr elementMapPtr;
};

class ElementMapTest: public ::testing::Test
{
protected:
    static void SetUpTestSuite() {
        int argc = 1;
        char *argv[] = { "FreeCAD" };
        App::Application::Config()["ExeName"] = "FreeCAD";
        App::Application::init(argc, argv);
    }
    void SetUp() override {
        App::GetApplication().newDocument("test", "testUser");
        sids = &_sid;
    }
    // void TearDown() override {}
    void populateFakePart(LessComplexPart& part, long Tag) {
        // object also have Vertexes etc and the face count varies; but that is not important
        // here since we are not testing a real model
        // the "MappedName" is left blank for now
        part.elementMapPtr = std::make_shared<Data::ElementMap>();
        auto face1 = Data::IndexedName("Face", 1);
        auto face2 = Data::IndexedName("Face", 2);
        auto face3 = Data::IndexedName("Face", 3);
        auto face4 = Data::IndexedName("Face", 4);
        auto face5 = Data::IndexedName("Face", 5);
        auto face6 = Data::IndexedName("Face", 6);
        part.elementMapPtr->setElementName(face1, Data::MappedName(face1), Tag);
        part.elementMapPtr->setElementName(face2, Data::MappedName(face2), Tag);
        part.elementMapPtr->setElementName(face3, Data::MappedName(face3), Tag);
        part.elementMapPtr->setElementName(face4, Data::MappedName(face4), Tag);
        part.elementMapPtr->setElementName(face5, Data::MappedName(face5), Tag);
        part.elementMapPtr->setElementName(face6, Data::MappedName(face6), Tag);
        part.Tag = Tag;
    }
    Data::ElementIDRefs _sid;
    QVector<App::StringIDRef>* sids;
};

TEST_F(ElementMapTest, defaultConstruction)
{
    // Act
    Data::ElementMap elementMap = Data::ElementMap();

    // Assert
    EXPECT_EQ(elementMap.size(), 0);
}

TEST_F(ElementMapTest, setElementNameDefaults)
{
    // Arrange
    Data::MappedName mappedName("TEST");
    Data::IndexedName element("Edge", 1);
    Data::MappedName expectedName("TEST");
    Data::ElementMap elementMap;

    // Act
    auto resultName = elementMap.setElementName(element, mappedName, 0, sids);

    // Assert
    EXPECT_EQ(resultName, expectedName);
    auto mappedToElement = elementMap.find(element);
    EXPECT_EQ(mappedToElement, mappedName);
}

TEST_F(ElementMapTest, setElementNameWithHashing)
{
    // Arrange
    Data::MappedName mappedName("TEST");
    Data::IndexedName element("Edge", 1);
    Data::MappedName expectedName("TEST");
    Data::ElementMap elementMap;

    // Act
    auto resultName = elementMap.setElementName(element, mappedName, 0, sids);

    // Assert
    EXPECT_EQ(resultName, expectedName);
    auto mappedToElement = elementMap.find(element);
    EXPECT_EQ(mappedToElement, mappedName);
}

TEST_F(ElementMapTest, mimicOnePart)
{
    // Arrange
    //   pattern: new doc, create Cube
    //   for a single part, there is no "naming algo" to speak of
    std::ostringstream ss;
    auto docName = "Unnamed";
    auto cube = LessComplexPart();
    auto cubeName = Data::MappedName("Box");
    cube.elementMapPtr = std::make_shared<Data::ElementMap>();
    cube.Tag = 1;
    auto finalMappedName = Data::MappedName();
    auto face1 = Data::IndexedName("Face", 1);
    auto face2 = Data::IndexedName("Face", 2);
    auto face3 = Data::IndexedName("Face", 3);
    auto face4 = Data::IndexedName("Face", 4);
    auto face5 = Data::IndexedName("Face", 5);
    auto face6 = Data::IndexedName("Face", 6);
    auto face6NameHolder = Data::MappedName(face6);

    // Act
    cube.elementMapPtr->setElementName(face1, Data::MappedName(face1), 0); //  build a box with faces
    cube.elementMapPtr->setElementName(face2, Data::MappedName(face2), 0);
    cube.elementMapPtr->setElementName(face3, Data::MappedName(face3), 0);
    cube.elementMapPtr->setElementName(face4, Data::MappedName(face4), 0);
    cube.elementMapPtr->setElementName(face5, Data::MappedName(face5), 0);
    cube.elementMapPtr->encodeElementName(  // we will invoke the encoder anyway for face 6; should make no difference
        face6.getType()[0],
        face6NameHolder,
        ss,
        nullptr,
        cube.Tag,
        nullptr,
        cube.Tag);  // naming against itself
    cube.elementMapPtr->setElementName(face6, face6NameHolder, 0);
    auto children = cube.elementMapPtr->getAll();
    ss << docName << "#" << cubeName << "." << cube.elementMapPtr->find(face6);

    // Assert
    EXPECT_EQ(children.size(), 6);
    EXPECT_EQ(children[0].index.toString(), "Face1");
    EXPECT_EQ(children[0].name.toString(), "Face1");
    EXPECT_EQ(children[1].index.toString(), "Face2");
    EXPECT_EQ(children[1].name.toString(), "Face2");
    EXPECT_EQ(children[2].index.toString(), "Face3");
    EXPECT_EQ(children[2].name.toString(), "Face3");
    EXPECT_EQ(children[3].index.toString(), "Face4");
    EXPECT_EQ(children[3].name.toString(), "Face4");
    EXPECT_EQ(children[4].index.toString(), "Face5");
    EXPECT_EQ(children[4].name.toString(), "Face5");
    EXPECT_EQ(children[5].index.toString(), "Face6");
    EXPECT_EQ(children[5].name.toString(), "Face6");
    auto serializedResult = ss.str();
    EXPECT_EQ(serializedResult, "Unnamed#Box.Face6");
}

TEST_F(ElementMapTest, mimicSimpleUnion)
{
    // Arrange
    //   pattern: new doc, create Cube, create Cylinder, Union of both (Cube first)
    std::ostringstream ss;
    std::ostringstream finalSs;
    char* docName = "Unnamed";
    //   arrange "create cube"
    LessComplexPart cube;
    populateFakePart(cube, 1L);
    auto cubeName = Data::MappedName("Box");
    //   arrange "create cylinder"
    LessComplexPart cylinder;
    auto cylinderName = Data::MappedName("Cylinder");
    populateFakePart(cylinder, 2L);
    //   arrange Union (Fusion) operation via the Part Workbench
    LessComplexPart unionPart;
    populateFakePart(unionPart, 3L);
    auto unionName = Data::MappedName("Fusion"); // fusion
    auto uface3 = Data::IndexedName("Face", 3); // we are only going to simulate one face for testing purpose
    auto PartOp = "FUS"; // Part::OpCodes::Fuse;

    // Act
    //   act: simulate a union/fuse operation
    auto parent = cube.elementMapPtr->getAll()[5];
    auto postfixHolder = Data::MappedName(Data::POSTFIX_MOD + "2");
    unionPart.elementMapPtr->encodeElementName(
        postfixHolder[0],
        postfixHolder,
        ss,
        nullptr,
        unionPart.Tag,
        nullptr,
        unionPart.Tag);
    auto postfixStr = postfixHolder.toString() + Data::ELEMENT_MAP_PREFIX + PartOp;
    //   act: with the fuse op, name against the cube's Face6
    auto uface3Holder = Data::MappedName(parent.index);
    unionPart.elementMapPtr->encodeElementName(// we will invoke the encoder for face 3
        uface3Holder[0],
        uface3Holder,
        ss,
        nullptr,
        unionPart.Tag,
        postfixStr.c_str(),
        cube.Tag);
    unionPart.elementMapPtr->setElementName(uface3, uface3Holder, unionPart.Tag, nullptr, true);
    // act: generate a full toponame string for testing  purposes
    finalSs << docName << "#" << unionName;
    finalSs << ".";
    finalSs << Data::ELEMENT_MAP_PREFIX + unionPart.elementMapPtr->find(uface3).toString();
    finalSs << ".";
    finalSs << uface3;

    // Assert
    const char* expectedUnionOpPostfix = ":M2;FUS";
    EXPECT_EQ(postfixStr, std::string(expectedUnionOpPostfix));
    const char* expectedFace3Name = "Face6;:M2;FUS;:H1:8,F";
    EXPECT_EQ(unionPart.elementMapPtr->find(uface3).toString(), expectedFace3Name);
    auto serializedResult = finalSs.str();
    const char* expectedNameOfTopFaceOfCubeSide = "Unnamed#Fusion.;Face6;:M2;FUS;:H1:8,F.Face3";
    EXPECT_EQ(serializedResult, expectedNameOfTopFaceOfCubeSide);

    // explanation of "Fusion.;Face6;:M2;FUS;:H2:3,F.Face3"  toponame
    //    Note: every postfix is prefixed by semicolon
    //    Node: the start/middle/end are separated by periods
    //
    // "Fusion" means that we are on the "Fusion" object.
    // "." we are done with the first part
    // ";Face6" means default inheritance comes from face 6 of the parent (which is a cube)
    // ";:M2" means that a Workbench op has happened. "M" is the "Mod" directory in the source tree?
    // ";FUS" means that a Fusion operation has happened. Notice the lack of a colon.
    // ";:H2" means the subtending object (cylinder) has a tag of 2
    // ":3" means the writing position is 3; literally how far into the current postfix string we are
    // ",F" means are of type "F" which is short for "Face" of Face3 of Fusion.
    // "." we are done with the second part
    // "Face3" is the localized name
}

// NOLINTEND(readability-magic-numbers)
