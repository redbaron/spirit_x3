/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(SPIRIT_X3_PARSE_INTO_CONTAINER_JAN_15_2013_0957PM)
#define SPIRIT_X3_PARSE_INTO_CONTAINER_JAN_15_2013_0957PM

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/spirit/home/x3/support/traits/container_traits.hpp>
#include <boost/spirit/home/x3/support/traits/value_traits.hpp>
#include <boost/spirit/home/x3/support/traits/attribute_of.hpp>
#include <boost/spirit/home/x3/support/traits/handles_container.hpp>
#include <boost/spirit/home/x3/support/traits/has_attribute.hpp>
#include <boost/spirit/home/x3/support/traits/is_substitute.hpp>
#include <boost/mpl/and.hpp>
#include <boost/fusion/include/front.hpp>
#include <boost/fusion/include/deque.hpp>
#include <boost/variant.hpp>

namespace boost { namespace spirit { namespace x3 { namespace detail
{
    template <typename Key, typename Enable = void>
    struct save_into_fusion_map_impl{
        template <typename Attribute, typename T>
        static void call(Attribute& attr, const T& kv_pair) {
	    fusion::at_key<Key>(attr) = fusion::at_c<1>(kv_pair);
        }
    };

    template <typename T, typename ... Types>
    struct save_into_fusion_map_impl <boost::variant<T, Types...> >{

	template <typename Map, typename Value>
	struct saver_visitor : boost::static_visitor<> {
	    saver_visitor(Map& map, const Value& val)
		: map(map)
		, val(val) {}

	    template<typename Key>
	    void operator() (Key&) const {
		// attributes from alternative come as single
		// element fusion::deque<T> type, so need to extract T
		typedef typename fusion::result_of::value_at_c<Key, 0>::type map_key;

		fusion::at_key<map_key>(map) = val;
	    }
	    Map& map;
	    const Value& val;
	};

        template <typename Attribute, typename Pair>
        static void call(Attribute& attr, const Pair& kv_pair) {
	    boost::apply_visitor(
	        saver_visitor< Attribute
		, typename fusion::result_of::value_at_c<Pair, 1>::type >(
		    attr, fusion::at_c<1>(kv_pair))
		, fusion::at_c<0>(kv_pair));
        }
    };

    template <typename Parser>
    struct parse_into_container_base_impl
    {
        // Parser has attribute (synthesize; Attribute is a container)
        template <typename Iterator, typename Context, typename Attribute>
        static bool call_synthesize(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::false_, mpl::false_)
        {
            // synthesized attribute needs to be value initialized
            typedef typename
                traits::container_value<Attribute>::type
            value_type;
            value_type val = traits::value_initialize<value_type>::call();

            if (!parser.parse(first, last, context, val))
                return false;

            // push the parsed value into our attribute
            traits::push_back(attr, val);
            return true;
        }

        template <typename Iterator, typename Context, typename Attribute>
        static bool call_synthesize(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::true_, mpl::true_)
        {
	    typedef typename traits::attribute_of<Parser, Context>::type attr_type;
	    static_assert(fusion::result_of::size<attr_type>::value == 2, 
	    		  "Only tuple of 2 attributes can be used");

	    typedef typename fusion::result_of::value_at_c<attr_type, 0>::type map_key;
            attr_type val = traits::value_initialize<attr_type>::call();

            if (!parser.parse(first, last, context, val))
                return false;

            // save parsed value in fusion associative sequence
	    save_into_fusion_map_impl<map_key>::call(attr, val);
            return true;
	}

        // Parser has attribute (synthesize; Attribute is a single element fusion sequence)
        template <typename Iterator, typename Context, typename Attribute>
        static bool call_synthesize(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::true_, mpl::false_)
        {
            static_assert(traits::has_size<Attribute, 1>::value,
                "Expecting a single element fusion sequence");
            return call_synthesize(parser, first, last, context,
                fusion::front(attr), mpl::false_(), mpl::false_());
        }

        // Parser has attribute (synthesize)
        template <typename Iterator, typename Context, typename Attribute>
        static bool call(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::true_)
        {
            return call_synthesize(parser, first, last, context, attr,
                   fusion::traits::is_sequence<Attribute>(),
		   mpl::and_<fusion::traits::is_sequence<Attribute>,
			     fusion::traits::is_associative<Attribute> >());
        }

        // Parser has no attribute (pass unused)
        template <typename Iterator, typename Context, typename Attribute>
        static bool call(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::false_)
        {
            return parser.parse(first, last, context, unused);
        }

        template <typename Iterator, typename Context, typename Attribute>
        static bool call(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr)
        {
            return call(parser, first, last, context, attr
              , mpl::bool_<traits::has_attribute<Parser, Context>::value>());
        }
    };

    template <typename Parser, typename Context, typename Enable = void>
    struct parse_into_container_impl : parse_into_container_base_impl<Parser> {};

    template <typename Parser, typename Container, typename Context>
    struct parser_attr_is_substitute_for_container_value
        : traits::is_substitute<
            typename traits::attribute_of<Parser, Context>::type
          , typename traits::container_value<Container>::type
        >
    {};

    template <typename Parser, typename Context>
    struct parse_into_container_impl<Parser, Context,
        typename enable_if<traits::handles_container<Parser, Context>>::type>
    {
        template <typename Iterator, typename Attribute>
        static bool call(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::true_)
        {
            return parse_into_container_base_impl<Parser>::call(
                parser, first, last, context, attr);
        }

        template <typename Iterator, typename Attribute>
        static bool call(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr, mpl::false_)
        {
            return parser.parse(first, last, context, attr);
        }

        template <typename Iterator, typename Attribute>
        static bool call(
            Parser const& parser
          , Iterator& first, Iterator const& last
          , Context const& context, Attribute& attr)
        {
            return call(parser, first, last, context, attr,
                parser_attr_is_substitute_for_container_value<Parser, Attribute, Context>());
        }
    };

    template <typename Parser, typename Iterator
      , typename Context, typename Attribute>
    bool parse_into_container(
        Parser const& parser
      , Iterator& first, Iterator const& last
      , Context const& context, Attribute& attr)
    {
        return parse_into_container_impl<Parser, Context>::call(
            parser, first, last, context, attr);
    }

}}}}

#endif
