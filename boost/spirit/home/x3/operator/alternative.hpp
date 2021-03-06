/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_ALTERNATIVE_JAN_07_2013_1131AM)
#define SPIRIT_ALTERNATIVE_JAN_07_2013_1131AM

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/spirit/home/x3/support/traits/attribute_of.hpp>
#include <boost/spirit/home/x3/core/parser.hpp>
#include <boost/spirit/home/x3/operator/detail/alternative.hpp>

namespace boost { namespace spirit { namespace x3
{
    template <typename Left, typename Right>
    struct alternative : binary_parser<Left, Right, alternative<Left, Right>>
    {
        typedef binary_parser<Left, Right, alternative<Left, Right>> base_type;

        alternative(Left left, Right right)
            : base_type(left, right) {}

        template <typename Iterator, typename Context>
        bool parse(
            Iterator& first, Iterator const& last
          , Context const& context, unused_type) const
        {
            return this->left.parse(first, last, context, unused)
               || this->right.parse(first, last, context, unused);
        }

        template <typename Iterator, typename Context, typename Attribute>
        bool parse(
            Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr) const
        {
            if (detail::parse_alternative(this->left, first, last, context, attr))
                return true;
            if (detail::parse_alternative(this->right, first, last, context, attr))
                return true;
            return false;
        }
    };

    template <typename Left, typename Right>
    inline alternative<
        typename extension::as_parser<Left>::value_type
      , typename extension::as_parser<Right>::value_type>
    operator|(Left const& left, Right const& right)
    {
        typedef alternative<
            typename extension::as_parser<Left>::value_type
          , typename extension::as_parser<Right>::value_type>
        result_type;

        return result_type(as_parser(left), as_parser(right));
    }
}}}

namespace boost { namespace spirit { namespace x3 { namespace traits
{
    template <typename Left, typename Right, typename Context>
    struct attribute_of<x3::alternative<Left, Right>, Context>
        : x3::detail::attribute_of_alternative<Left, Right, Context> {};
}}}}

#endif
