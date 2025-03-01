//------------------------------------------------------------------------------
/*
    This file is part of Beast: https://github.com/vinniefalco/Beast
    Copyright 2013, Vinnie Falco <vinnie.falco@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#ifndef BEAST_UTILITY_CI_CHAR_TRAITS_H_INCLUDED
#define BEAST_UTILITY_CI_CHAR_TRAITS_H_INCLUDED

#include <beast/cxx14/algorithm.h> // <algorithm>
#include <cctype>
#include <locale>
#include <string>

namespace beast {

/** Case-insensitive function object for performing less than comparisons. */
struct ci_less
{
    static bool const is_transparent = true;

    template <class String>
    bool
    operator() (String const& lhs, String const& rhs) const
    {
        typedef typename String::value_type char_type;
        return std::lexicographical_compare (std::begin(lhs), std::end(lhs),
            std::begin(rhs), std::end(rhs),
            [] (char_type lhs, char_type rhs)
            {
                return std::tolower(lhs) < std::tolower(rhs);
            }
        );
    }
};

/** Returns `true` if strings are case-insensitive equal. */
template <class String>
bool
ci_equal(String const& lhs, String const& rhs)
{
    typedef typename String::value_type char_type;
    return std::equal (lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [] (char_type lhs, char_type rhs)
        {
            return std::tolower(lhs) == std::tolower(rhs);
        }
    );
}

}

#endif
