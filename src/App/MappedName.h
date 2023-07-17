// SPDX-License-Identifier: LGPL-2.1-or-later

/****************************************************************************
 *   Copyright (c) 2022 Zheng, Lei (realthunder) <realthunder.dev@gmail.com>*
 *   Copyright (c) 2023 FreeCAD Project Association                         *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#ifndef APP_MAPPED_NAME_H
#define APP_MAPPED_NAME_H

#include "ElementNamingUtils.h"
#include "IndexedName.h"
#include "StringHasher.h"
#include "LazyClass.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <QVector>

#include <iostream>
#include <memory>
#include <string>

namespace Data
{


// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

class AppExport MappedName
{
public:
    explicit MappedName(const std::string& name);
    explicit MappedName(const char* name, size_t size = std::string::npos);
    explicit MappedName(const IndexedName& element);
    explicit MappedName(const App::StringIDRef& sid);

    MappedName() = default;

    MappedName(const MappedName& other) = default;
    MappedName& operator=(const MappedName& other) = default;

    MappedName(const MappedName& other, size_t startPosition, size_t size = std::string::npos);
    MappedName(const MappedName& other, const char* postfix);

    MappedName(MappedName&& other) noexcept = default;
    MappedName& operator=(MappedName&& other) noexcept = default;

    ~MappedName() = default;

    MappedName& operator=(const std::string& other);
    MappedName& operator=(const char* other);

    friend std::ostream& operator<<(std::ostream& stream, const MappedName& mappedName)
    {
        stream.write(mappedName.data.asConst().c_str(), mappedName.data.asConst().size());
        return stream;
    }

    bool operator==(const MappedName& other) const;
    bool operator!=(const MappedName& other) const;

    MappedName operator+(const MappedName& other) const;
    MappedName operator+(const char* other) const;
    MappedName operator+(const std::string& other) const;

    MappedName& operator+=(const MappedName& other);
    MappedName& operator+=(const char* other);
    MappedName& operator+=(const std::string& other);

    void append(const char* dataToAppend, size_t size = std::string::npos);
    void append(const MappedName& other, size_t startPosition = 0, size_t size = std::string::npos);

    const std::string& toString() const;
    const std::string name() const;
    const std::string postfix() const;

    /// Create an IndexedName from the data portion of this MappedName. If this data has a postfix,
    /// the function returns an empty IndexedName. The function will fail if this->data contains
    /// anything other than the ASCII letter a-z, A-Z, and the underscore, with an optional integer
    /// suffix, returning an empty IndexedName (e.g. an IndexedName that evaluates to boolean
    /// false and isNull() == true).
    ///
    /// \return a new IndexedName that shares its data with this instance's data member.
    IndexedName toIndexedName() const;

    /// Equivalent to C++20 operator<=>. Performs byte-by-byte comparison of this and other,
    /// starting at the first byte and continuing through both data and postfix, ignoring which is
    /// which. If the combined data and postfix members are of unequal size but start with the same
    /// data, the shorter array is considered "less than" the longer.
    int compare(const MappedName& other) const;

    /// \see compare()
    bool operator<(const MappedName& other) const;

    char operator[](size_t index) const;

    size_t size() const;
    bool empty() const;
    explicit operator bool() const;
    void clear();

    size_t find(const char* searchTarget, size_t startPosition = 0) const;
    size_t find(const std::string& searchTarget, size_t startPosition = 0) const;

    size_t rfind(const char* searchTarget, size_t startPosition = std::string::npos) const;
    size_t rfind(const std::string& searchTarget, size_t startPosition = std::string::npos) const;

    bool endsWith(const char* searchTarget) const;
    bool endsWith(const std::string& searchTarget) const;

    bool startsWith(const char* searchTarget, size_t offset = 0) const;
    bool startsWith(const std::string& searchTarget, size_t offset = 0) const;

    /// Extract tagOut and other information from a encoded element name
    ///
    /// \param tagOut: optional pointer to receive the extracted tagOut
    /// \param lenOut: optional pointer to receive the length field after the tagOut field.
    ///             This gives the length of the previous hashed element name starting
    ///             from the beginning of the give element name.
    /// \param postfixOut: optional pointer to receive the postfixOut starting at the found tagOut field.
    /// \param typeOut: optional pointer to receive the element typeOut character
    /// \param negative: return negative tagOut as it is. If disabled, then always return positive tagOut.
    ///                  Negative tagOut is sometimes used for element disambiguation.
    /// \param recursive: recursively find the last non-zero tagOut
    ///
    /// \return Return the end position of the tagOut field, or return -1 if not found.
    size_t findTagInElementName(long* tagOut = nullptr, size_t* lenOut = nullptr, std::string* postfixOut = nullptr,
                             char* typeOut = nullptr, bool negative = false,
                             bool recursive = true) const;

private:
    Lazy<std::string> data; 
    size_t postfixStartIdx = 0;
};


using ElementIDRefs = QVector<::App::StringIDRef>;

struct MappedNameRef
{
    MappedName name;
    ElementIDRefs sids;
    std::unique_ptr<MappedNameRef> next;

    MappedNameRef() = default;

    ~MappedNameRef() = default;

    MappedNameRef(MappedName name, ElementIDRefs sids = ElementIDRefs())
        : name(std::move(name))
        , sids(std::move(sids))
    {
        compact();
    }

    MappedNameRef(const MappedNameRef& other)
        : name(other.name)
        , sids(other.sids)
    {}

    MappedNameRef(MappedNameRef&& other) noexcept
        : name(std::move(other.name))
        , sids(std::move(other.sids))
        , next(std::move(other.next))
    {}

    MappedNameRef& operator=(const MappedNameRef& other) noexcept
    {
        name = other.name;
        sids = other.sids;
        return *this;
    }

    MappedNameRef& operator=(MappedNameRef&& other) noexcept
    {
        name = std::move(other.name);
        sids = std::move(other.sids);
        next = std::move(other.next);
        return *this;
    }

    explicit operator bool() const
    {
        return !name.empty();
    }

    void append(const MappedName& _name, const ElementIDRefs _sids = ElementIDRefs())
    {
        if (!_name) {
            return;
        }
        if (!this->name) {
            this->name = _name;
            this->sids = _sids;
            compact();
            return;
        }
        std::unique_ptr<MappedNameRef> mappedName(new MappedNameRef(_name, _sids));
        if (!this->next) {
            this->next = std::move(mappedName);
        }
        else {
            this->next.swap(mappedName);
            this->next->next = std::move(mappedName);
        }
    }

    void compact()
    {
        if (sids.size() > 1) {
            std::sort(sids.begin(), sids.end());
            sids.erase(std::unique(sids.begin(), sids.end()), sids.end());
        }
    }

    bool erase(const MappedName& _name)
    {
        if (this->name == _name) {
            this->name.clear();
            this->sids.clear();
            if (this->next) {
                this->name = std::move(this->next->name);
                this->sids = std::move(this->next->sids);
                std::unique_ptr<MappedNameRef> tmp;
                tmp.swap(this->next);
                this->next = std::move(tmp->next);
            }
            return true;
        }

        for (std::unique_ptr<MappedNameRef>* ptr = &this->next; *ptr; ptr = &(*ptr)->next) {
            if ((*ptr)->name == _name) {
                std::unique_ptr<MappedNameRef> tmp;
                tmp.swap(*ptr);
                *ptr = std::move(tmp->next);
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        this->name.clear();
        this->sids.clear();
        this->next.reset();
    }
};


// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)


}// namespace Data


#endif// APP_MAPPED_NAME_H
