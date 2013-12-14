/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_X3_ACTION_JANUARY_07_2007_1128AM)
#define SPIRIT_X3_ACTION_JANUARY_07_2007_1128AM

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/spirit/home/x3/support/context.hpp>
#include <boost/spirit/home/x3/support/traits/attribute_of.hpp>
#include <boost/spirit/home/x3/support/traits/make_attribute.hpp>
#include <boost/spirit/home/x3/nonterminal/detail/transform_attribute.hpp>

namespace boost { namespace spirit { namespace x3
{
    struct rule_context_tag;

    template <typename Subject, typename Action>
    struct action : unary_parser<Subject, action<Subject, Action>>
    {
        typedef unary_parser<Subject, action<Subject, Action>> base_type;
        static bool const is_pass_through_unary = true;
        static bool const has_action = true;

        action(Subject const& subject, Action f)
          : base_type(subject), f(f) {}

        template <typename Iterator, typename Context, typename Attribute>
        bool parse(Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr) const
        {
            Iterator save = first;
            if (this->subject.parse(first, last, context, attr))
            {
                // call the function, passing the enclosing rule's context
                // and the subject's attribute.
                f(context, attr);
                return true;

                // reset iterators if semantic action failed the match
                // retrospectively
                first = save;
            }
            return false;
        }

        template <typename Iterator, typename Context>
        bool parse(Iterator& first, Iterator const& last
          , Context const& context, unused_type) const
        {
            typedef typename
                traits::attribute_of<action<Subject, Action>, Context>::type
            attribute_type;
            typedef traits::make_attribute<attribute_type, unused_type> make_attribute;
            typedef traits::transform_attribute<
                typename make_attribute::type, attribute_type, parser_id>
            transform;

            // synthesize the attribute since one is not supplied
            typename make_attribute::type made_attr = make_attribute::call(unused_type());
            typename transform::type attr = transform::pre(made_attr);
            return parse(first, last, context, attr);
        }

        Action f;
    };
}}}

#endif
