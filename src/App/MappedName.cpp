/****************************************************************************
 *   Copyright (c) 2020 Zheng, Lei (realthunder) <realthunder.dev@gmail.com>*
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/

#include "MappedName.h"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

using namespace Data;


MappedName::MappedName(const std::string& name)
{
    if (boost::starts_with(name, ELEMENT_MAP_PREFIX)) {
        data = name.substr(ELEMENT_MAP_PREFIX_SIZE);
    }
    else {
        data = name;
    }
    postfixStartIdx = data.asConst().size();
}

MappedName::MappedName(const char* name, size_t size)
    : MappedName(size != std::string::npos ? std::string(name, size) : std::string(name))
{}

MappedName::MappedName(const IndexedName& element)
    : data(element.getType())
{
    if (element.getIndex() > 0) {
        data.asMutable() += std::to_string(element.getIndex());
    }
    postfixStartIdx = data.asConst().size();
}

MappedName::MappedName(const App::StringIDRef& sid)
    : data(sid.toBytes().constData())
{}

MappedName::MappedName(const MappedName& other, size_t startPosition, size_t size)
{
    append(other, startPosition, size);
}

MappedName::MappedName(const MappedName& other, const char* postfix)
    : data(other.data.asConst() + postfix),
      postfixStartIdx(other.size())
{}

MappedName& MappedName::operator=(const std::string& other)
{
    *this = MappedName(other);
    return *this;
}

MappedName& MappedName::operator=(const char* other)
{
    *this = MappedName(other);
    return *this;
}

bool MappedName::operator==(const MappedName& other) const
{
    return this->data.asConst() == other.data.asConst();
}

bool MappedName::operator!=(const MappedName& other) const
{
    return !(this->operator==(other));
}

MappedName MappedName::operator+(const MappedName& other) const
{
    MappedName res(*this);
    res += other;
    return res;
}

MappedName MappedName::operator+(const char* other) const
{
    MappedName res(*this);
    res += other;
    return res;
}

MappedName MappedName::operator+(const std::string& other) const
{
    MappedName res(*this);
    res += other;
    return res;
}

MappedName& MappedName::operator+=(const char* other)
{
    this->data.asMutable().append(other);
    return *this;
}

MappedName& MappedName::operator+=(const std::string& other)
{
    this->data.asMutable().append(other);
    return *this;
}

MappedName& MappedName::operator+=(const MappedName& other)
{
    append(other);
    return *this;
}

void MappedName::append(const char* dataToAppend, size_t size)
{
    std::string stringToAppend =
        size != std::string::npos ? std::string(dataToAppend, size) : std::string(dataToAppend);
    if (this->data.asConst().size() == 0) {
        postfixStartIdx = stringToAppend.size();
    }
    this->data.asMutable().append(stringToAppend);
}

void MappedName::append(const MappedName& other, size_t startPosition, size_t size)
{
    std::string stringToAppend = other.data.asConst().substr(startPosition, size);
    if (this->data.asConst().size() == 0 && other.postfixStartIdx >= startPosition) {
        postfixStartIdx = other.postfixStartIdx - startPosition;
    }
    this->data.asMutable().append(stringToAppend);
}

const std::string& MappedName::toString() const
{
    return this->data.asConst();
}

const std::string MappedName::name() const
{
    return this->data.asConst().substr(0, postfixStartIdx);
}

const std::string MappedName::postfix() const
{
    return this->data.asConst().substr(postfixStartIdx);
}

IndexedName MappedName::toIndexedName() const
{
    if (this->postfixStartIdx == this->data.asConst().size()) {
        return IndexedName(this->data.asConst().c_str());
    }
    return IndexedName();
}

int MappedName::compare(const MappedName& other) const
{
    auto val = this->data.asConst().compare(other.data.asConst());
    if (val < 0) {
        return -1;
    }
    if (val > 0) {
        return 1;
    }
    return 0;
}

bool MappedName::operator<(const MappedName& other) const
{
    return compare(other) < 0;
}

char MappedName::operator[](size_t index) const
{
    return this->data.asConst()[index];
}

size_t MappedName::size() const
{
    return this->data.asConst().size();
}

bool MappedName::empty() const
{
    return this->data.asConst().empty();
}

MappedName::operator bool() const
{
    return !empty();
}

void MappedName::clear()
{
    this->data.asMutable().clear();
    this->postfixStartIdx = 0;
}

size_t MappedName::find(const char* searchTarget, size_t startPosition) const
{
    return this->data.asConst().find(searchTarget, startPosition);
}

size_t MappedName::find(const std::string& searchTarget, size_t startPosition) const
{
    return this->data.asConst().find(searchTarget, startPosition);
}

size_t MappedName::rfind(const char* searchTarget, size_t startPosition) const
{
    return this->data.asConst().rfind(searchTarget, startPosition);
}

size_t MappedName::rfind(const std::string& searchTarget, size_t startPosition) const
{
    return this->data.asConst().rfind(searchTarget, startPosition);
}

/* These are string builtin in c++20 only, have to use boost I guess */
bool MappedName::endsWith(const char* searchTarget) const
{
    return boost::ends_with(this->data.asConst(), searchTarget);
}

bool MappedName::endsWith(const std::string& searchTarget) const
{
    return boost::ends_with(this->data.asConst(), searchTarget);
}

bool MappedName::startsWith(const char* searchTarget, size_t offset) const
{
    return boost::starts_with(this->data.asConst().substr(offset), searchTarget);
}

bool MappedName::startsWith(const std::string& searchTarget, size_t offset) const
{
    return boost::starts_with(this->data.asConst().substr(offset), searchTarget);
}

size_t MappedName::findTagInElementName(long* tagOut, size_t* lenOut, std::string* postfixOut,
                                     char* typeOut, bool negative, bool recursive) const
{
    size_t pos = this->rfind(POSTFIX_TAG);

    // Example name, POSTFIX_TAG == ;:H
    // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
    //                                     ^
    //                                     |
    //                                    pos

    bool hex = true;
    if(pos == std::string::npos) {
        pos = this->rfind(POSTFIX_DECIMAL_TAG);
        if (pos == std::string::npos) {
            return std::string::npos;
        }
        hex = false;
    }


    std::string after_postfix = this->data.asConst().substr(pos + POSTFIX_TAG_SIZE);
    const char* nameAsChars = after_postfix.c_str();
    size_t size = after_postfix.size();

    // check if the number followed by the tagPosfix is negative
    bool isNegative = (nameAsChars[0] == '-');
    if (isNegative) {
        ++nameAsChars;
        --size;
    }
    boost::iostreams::stream<boost::iostreams::array_source> iss(nameAsChars, size);

    long _tag = 0;
    char sep = 0;
    if (!hex) {
        // no hex is an older version of the encoding scheme
        iss >> _tag >> sep;
    } else {
        // The purpose of postfix tag is to encode one model operation. The
        // 'tag' field is used to record the own object ID of that model shape,
        // and the 'len' field indicates the length of the operation codes
        // before the postfix tag. These fields are in hex. The trailing 'F' is
        // the shape type of this element, 'F' for face, 'E' edge, and 'V' vertex.
        //
        // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
        //                     |              |   ^^ ^^
        //                     |              |   |   |
        //                     ---len = 0x10---  tag len

        iss >> std::hex;
        // _tag field can be skipped, if it is 0
        if (nameAsChars[0] == ',' || nameAsChars[0] == ':') {
            iss >> sep;
        }
        else {
            iss >> _tag >> sep;
        }
    }

    if (isNegative) {
        _tag = -_tag;
    }

    int _len = 0;
    char sep2 = 0;
    char tp = 0;
    char eof = 0;
    if (sep == ':') {
        // ':' is followed by _len field.
        //
        // For decTagPostfix() (i.e. older encoding scheme), this is the length
        // of the string before the entire postfix (A postfix may contain
        // multiple segments usually separated by ELEMENT_MAP_PREFIX).
        //
        // For newer POSTFIX_TAG, this counts the number of characters that
        // proceeds this tag postfix segment that forms the op code (see
        // example above).
        //
        // The reason of this change is so that the postfix can stay the same
        // regardless of the prefix, which can increase memory efficiency.
        //
        iss >> _len >> sep2 >> tp >> eof;

        // The next separator to look for is either ':' for older tag postfix, or ','
        if (!hex && sep2 == ':') {
            sep2 = ',';
        }
    }
    else if (hex && sep == ',') {
        // ',' is followed by a single character that indicates the element type.
        iss >> tp >> eof;
        sep = ':';
        sep2 = ',';
    }

    if (_len < 0 || sep != ':' || sep2 != ',' || tp == 0 || eof != 0) {
        return std::string::npos;
    }

    if (hex) {
        // len is checked to be positive just above, can cast to unsigned
        if (pos < static_cast<size_t>(_len)) {
           return std::string::npos;
        }
        if ((_len != 0) && recursive && (tagOut || lenOut)) {
            // in case of recursive tag postfix (used by hierarchy element
            // map), look for any embedded tag postfix
            size_t next = this->data.asConst().substr(pos-_len, _len).rfind(POSTFIX_TAG);
            if (next != std::string::npos) {
                next += pos - _len;
                // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
                //                     ^               ^
                //                     |               |
                //                    next            pos
                //
                // There maybe other operation codes after this embedded tag
                // postfix, search for the seperator.
                //
                size_t end;
                if (pos == next) {
                    end = std::string::npos;
                }
                else {
                    end = this->data.asConst().substr(next+1, pos-next-1).find(ELEMENT_MAP_PREFIX);
                }
                if (end != std::string::npos) {
                    end += next+1;
                    // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
                    //                            ^
                    //                            |
                    //                           end
                    _len = pos - end;
                    // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
                    //                            |       |
                    //                            -- len --
                } else {
                    _len = 0;
                }
            }
        }

        // Now convert the 'len' field back to the length of the remaining name
        //
        // #94;:G0;XTR;:H19:8,F;:H1a,F;BND:-1:0;:H1b:10,F
        // |                         |
        // ----------- len -----------
        _len = pos - _len;
    }
    if(typeOut) {
        *typeOut = tp;
    }
    if(tagOut) {
        if (_tag == 0 && recursive) {
            return MappedName(*this, 0, _len)
                .findTagInElementName(tagOut, lenOut, postfixOut, typeOut, negative); //FIXME creates copy
        }
        if(_tag>0 || negative) {
            *tagOut = _tag;
        }
        else {
            *tagOut = -_tag;
        }
    }
    if(lenOut) {
        *lenOut = static_cast<size_t>(_len); //len has been checked to be positive
    }
    if(postfixOut) {
        *postfixOut = this->data.asConst().substr(pos);
    }
    return pos;
}