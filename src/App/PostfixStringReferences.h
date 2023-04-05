#pragma once

#include <string>


namespace Data
{

/// Special prefix to mark the beginning of a mapped sub-element name
static constexpr std::string ELEMENT_MAP_PREFIX     = ";";

/// Special prefix to mark a missing element
static constexpr std::string MISSING_PREFIX         = "?";

/// Special postfix to mark the following tag
static constexpr std::string POSTFIX_TAG            = ELEMENT_MAP_PREFIX + ":H";
static constexpr std::string POSTFIX_DECIMAL_TAG    = ELEMENT_MAP_PREFIX + ":T";
static constexpr std::string POSTFIX_EXTERNAL_TAG   = ELEMENT_MAP_PREFIX + ":X";
static constexpr std::string POSTFIX_CHILD          = ELEMENT_MAP_PREFIX + ":C";

/// Special postfix to mark the index of an array element
static constexpr std::string POSTFIX_INDEX          = ELEMENT_MAP_PREFIX + ":I";
static constexpr std::string POSTFIX_UPPER          = ELEMENT_MAP_PREFIX + ":U";
static constexpr std::string POSTFIX_LOWER          = ELEMENT_MAP_PREFIX + ":L";
static constexpr std::string POSTFIX_MOD            = ELEMENT_MAP_PREFIX + ":M";
static constexpr std::string POSTFIX_GEN            = ELEMENT_MAP_PREFIX + ":G";
static constexpr std::string POSTFIX_MODGEN         = ELEMENT_MAP_PREFIX + ":MG";
static constexpr std::string POSTFIX_DUPLICATE      = ELEMENT_MAP_PREFIX + "D";


}// namespace Data