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

#ifndef BEAST_HTTP_RFC2616_H_INCLUDED
#define BEAST_HTTP_RFC2616_H_INCLUDED

#include <algorithm>
#include <string>
#include <utility>

#include <boost/regex.hpp>

namespace beast {
namespace http {

/** Routines for performing RFC2616 compliance.
    RFC2616:
        Hypertext Transfer Protocol -- HTTP/1.1
        http://www.w3.org/Protocols/rfc2616/rfc2616
*/
namespace rfc2616 {

/** Returns `true` if `c` is linear white space.
    This excludes the CRLF sequence allowed for line continuations.
*/
template <class CharT>
bool
is_lws (CharT c)
{
    return c == ' ' || c == '\t';
}

/** Returns `true` if `c` is any whitespace character. */
template <class CharT>
bool
is_white (CharT c)
{
    switch (c)
    {
    case ' ':  case '\f': case '\n':
    case '\r': case '\t': case '\v':
        return true;
    };
    return false;
}

/** Returns `true` if `c` is a control character. */
template <class CharT>
bool
is_ctl (CharT c)
{
    return c <= 31 || c >= 127;
}

/** Returns `true` if `c` is a separator. */
template <class CharT>
bool
is_sep (CharT c)
{
    switch (c)
    {
    case '(': case ')': case '<': case '>':  case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '{': case '}': case ' ': case '\t':
        return true;
    };
    return false;
}

template <class FwdIter>
FwdIter
trim_left (FwdIter first, FwdIter last)
{
    return std::find_if_not (first, last,
        &is_white <typename FwdIter::value_type>);
}

template <class FwdIter>
FwdIter
trim_right (FwdIter first, FwdIter last)
{
    if (first == last)
        return last;
    do
    {
        --last;
        if (! is_white (*last))
            return ++last;
    }
    while (last != first);
    return first;
}

template <class CharT, class Traits, class Allocator>
void
trim_right_in_place (std::basic_string <
    CharT, Traits, Allocator>& s)
{
    s.resize (std::distance (s.begin(),
        trim_right (s.begin(), s.end())));
}

template <class FwdIter>
std::pair <FwdIter, FwdIter>
trim (FwdIter first, FwdIter last)
{
    first = trim_left (first, last);
    last = trim_right (first, last);
    return std::make_pair (first, last);
}

template <class String>
String
trim (String const& s)
{
    using std::begin;
    using std::end;
    auto first (begin(s));
    auto last (end(s));
    std::tie (first, last) = trim (first, last);
    return { first, last };
}

template <class String>
String
trim_right (String const& s)
{
    using std::begin;
    using std::end;
    auto first (begin(s));
    auto last (end(s));
    last = trim_right (first, last);
    return { first, last };
}

inline
std::string
trim (std::string const& s)
{
    return trim <std::string> (s);
}

/** Call a functor for each comma delimited element.
    Quotes and escape sequences will be parsed and converted appropriately.
    Excess white space, commas, double quotes, and empty elements are not
    passed to func.
    Format:
       #(token|quoted-string)
    Reference:
        http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2
*/
template <class FwdIter, class Function>
void
for_each_element (FwdIter first, FwdIter last, Function func)
{
    FwdIter iter (first);
    std::string e;
    while (iter != last)
    {
        if (*iter == '"')
        {
            // quoted-string
            ++iter;
            while (iter != last)
            {
                if (*iter == '"')
                {
                    ++iter;
                    break;
                }

                if (*iter == '\\')
                {
                    // quoted-pair
                    ++iter;
                    if (iter != last)
                        e.append (1, *iter++);
                }
                else
                {
                    // qdtext
                    e.append (1, *iter++);
                }
            }
            if (! e.empty())
            {
                func (e);
                e.clear();
            }
        }
        else if (*iter == ',')
        {
            e = trim_right (e);
            if (! e.empty())
            {
                func (e);
                e.clear();
            }
            ++iter;
        }
        else if (is_lws (*iter))
        {
            ++iter;
        }
        else
        {
            e.append (1, *iter++);
        }
    }

    if (! e.empty())
    {
        e = trim_right (e);
        if (! e.empty())
            func (e);
    }
}

// Parse a comma-delimited list of values.
template <class CharT, class Traits, class Allocator>
std::vector<std::basic_string<CharT, Traits, Allocator>>
parse_csv (std::basic_string <CharT, Traits, Allocator> const& in,
    std::ostream& log)
{
    auto first = in.cbegin();
    auto const last = in.cend();
    std::vector<std::basic_string<CharT, Traits, Allocator>> result;
    if (first != last)
    {
        static boost::regex const re(
            "^"                         // start of line
            "(?:\\s*)"                  // whitespace (optional)
            "([a-zA-Z][_a-zA-Z0-9]*)"   // identifier
            "(?:\\s*)"                  // whitespace (optional)
            "(?:,?)"                    // comma (optional)
            "(?:\\s*)"                  // whitespace (optional)
            , boost::regex_constants::optimize
        );
        for(;;)
        {
            boost::smatch m;
            if (! boost::regex_search(first, last, m, re,
                boost::regex_constants::match_continuous))
            {
                log << "Expected <identifier>\n";
                throw std::exception();
            }
            result.push_back(m[1]);
            first = m[0].second;
            if (first == last)
                break;
        }
    }
    return result;
}

} // rfc2616

} // http
} // beast

#endif

