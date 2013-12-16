/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman
    Copyright (c) 2001-2011 Hartmut Kaiser

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_X3_KLEENE_JANUARY_07_2007_0818AM)
#define SPIRIT_X3_KLEENE_JANUARY_07_2007_0818AM

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/spirit/home/x3/core/parser.hpp>
#include <boost/spirit/home/x3/support/traits/container_traits.hpp>
#include <boost/spirit/home/x3/support/traits/attribute_of.hpp>
#include <boost/spirit/home/x3/core/detail/parse_into_container.hpp>

namespace boost { namespace spirit { namespace x3
{
    template <typename Subject>
    struct kleene : unary_parser<Subject, kleene<Subject>>
    {
        typedef unary_parser<Subject, kleene<Subject>> base_type;
        static bool const handles_container = true;

        kleene(Subject const& subject)
          : base_type(subject) {}

        template <typename Iterator, typename Context, typename Attribute>
        bool parse(Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr) const
        {
	    // to stop kleene from polluting attribute on partial
	    // match (but still failing overall) we make it operate on
	    // copy of attribute and copy result back to provided
	    // on every successfull iteration
	    Attribute attr_(attr);
            while (detail::parse_into_container(
		       this->subject, first, last, context, attr_)) {
		attr = attr_;
	    };
            return true;
        }
    };

    template <typename Subject>
    inline kleene<typename extension::as_parser<Subject>::value_type>
    operator*(Subject const& subject)
    {
        typedef
            kleene<typename extension::as_parser<Subject>::value_type>
        result_type;

        return result_type(as_parser(subject));
    }
}}}

namespace boost { namespace spirit { namespace x3 { namespace traits
{
    template <typename Subject, typename Context>
    struct attribute_of<x3::kleene<Subject>, Context>
        : build_container<
            typename attribute_of<Subject, Context>::type> {};
}}}}

#endif
