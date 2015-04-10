/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.5
 * \file
 * \brief All stuff related to tuple_as_message.
 */

#pragma once

#include <so_5/rt/h/message.hpp>

#include <tuple>

namespace so_5 {

namespace rt {

/*!
 * \since v.5.5.5
 * \brief A template which allows to use tuples as messages.
 *
 * \tparam TAG a type tag for distinguish messages with the same fields list.
 * \tparam TYPES types for message fields.
 *
 * This template is added to allow a user to use simple constructs for
 * very simple messages, when there is no need to define a full class
 * for message.
 *
 * Just for comparison:

	\code
	// This is recommended way to defining of messages.
	// Separate class allows to easily extend or refactor
	// message type in the future.
	struct process_data : public so_5::rt::message_t
	{
		const std::uint8_t * m_data;

		// Constructor is necessary.
		process_data( const std::uint8_t * data )
			:	m_data( data )
		{}
	};

	// And the event-handler for this message.
	void data_processing_agent::evt_process_data( const process_data & evt )
	{
		do_data_processing( evt.m_data ); // Some processing 
	}
	\endcode

 * And this the one possible usage of tuple_as_message_t for the same task:

	\code
	// This way of defining messages is not recommended for big projects
	// with large code base and big amount of agents and messages.
	// But can be useful for small to-be-thrown-out utilities.
	struct process_data_tag {};
	using process_data = so_5::rt::tuple_as_message_t< process_data_tag, const std::uint8_t * >;

	// And the event-handler for this message.
	void data_processing_agent::evt_process_data( const process_data & evt )
	{
		do_data_processing( std::get<0>( evt ) );
	}
	\endcode

 * Or even yet more simplier and dirtier:

	\code
	// This way of defining messages can be good only for very and very
	// small to-be-thrown-out programs (like tests and samples).
	using process_data = so_5::rt::tuple_as_message_t<
				std::integral_constant<int, 0>, const std::uint8_t * >;

	// And the event-handler for this message.
	void data_processing_agent::evt_process_data( const process_data & evt )
	{
		do_data_processing( std::get<0>( evt ) );
	}
	\endcode
 */
template< typename TAG, typename... TYPES >
struct tuple_as_message_t
	:	public std::tuple< TYPES... >
	,	public so_5::rt::message_t
{
	using base_tuple_type = std::tuple< TYPES... >;

	tuple_as_message_t() 
		{}

	tuple_as_message_t( const tuple_as_message_t & ) = default;

	tuple_as_message_t &
	operator=( const tuple_as_message_t & ) = default;

#if !defined( _MSC_VER )
	tuple_as_message_t( tuple_as_message_t && ) = default;

	tuple_as_message_t &
	operator=( tuple_as_message_t && ) = default;
#endif

	explicit tuple_as_message_t( const TYPES &... args )
		:	base_tuple_type( args... )
	{}

	template< typename... UTYPES >
	tuple_as_message_t( UTYPES &&... args )
		:	base_tuple_type( std::forward< UTYPES >( args )... )
	{}
};

} /* namespace rt */

} /* namespace so_5 */

