/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.1
 * \brief Implementation of free functions send/send_delayed.
 */

#pragma once

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/environment.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

	/*
	 * This is helpers for so_5::send implementation.
	 */

	template< class MESSAGE, bool IS_SIGNAL >
	struct instantiator_and_sender_t
		{
		};

	template< class MESSAGE >
	struct instantiator_and_sender_t< MESSAGE, true >
		{
			void
			send( const so_5::rt::mbox_t & to )
				{
					to->deliver_signal< MESSAGE >();
				}

			void
			send_delayed(
				so_5::rt::environment_t & env,
				const so_5::rt::mbox_t & to,
				std::chrono::steady_clock::duration pause )
				{
					env.single_timer< MESSAGE >( to, pause );
				}

			timer_id_t
			send_periodic(
				so_5::rt::environment_t & env,
				const so_5::rt::mbox_t & to,
				std::chrono::steady_clock::duration pause,
				std::chrono::steady_clock::duration period )
				{
					return env.schedule_timer< MESSAGE >( to, pause, period );
				}
		};

} /* namespace impl */

} /* namespace rt */

/*!
 * \since v.5.5.9
 * \brief Implementation details for send-family and request_future/value helper functions.
 */
namespace send_functions_details {

inline const so_5::rt::mbox_t &
arg_to_mbox( const so_5::rt::mbox_t & mbox ) { return mbox; }

inline const so_5::rt::mbox_t &
arg_to_mbox( const so_5::rt::agent_t & agent ) { return agent.so_direct_mbox(); }

inline const so_5::rt::mbox_t &
arg_to_mbox( const so_5::rt::adhoc_agent_definition_proxy_t & agent ) { return agent.direct_mbox(); }

} /* namespace send_functions_details */

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a message.
 */
template< typename MESSAGE, typename TARGET, typename... ARGS >
void
send( TARGET && to, ARGS&&... args )
	{
		send_functions_details::arg_to_mbox( to )->deliver_message(
				so_5::rt::details::make_message_instance< MESSAGE >(
						std::forward<ARGS>(args)...) );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for sending a signal.
 */
template< typename MESSAGE, typename TARGET >
void
send( TARGET && to )
	{
		so_5::rt::impl::instantiator_and_sender_t<
				MESSAGE,
				so_5::rt::is_signal< MESSAGE >::value > helper;

		helper.send( send_functions_details::arg_to_mbox( to ) );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a message to
 * the agent's direct mbox.
 */
template< typename MESSAGE, typename... ARGS >
void
send_to_agent( const so_5::rt::agent_t & receiver, ARGS&&... args )
	{
		send< MESSAGE >( receiver, std::forward<ARGS>(args)... );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for creating and delivering a message to
 * the ad-hoc agent's direct mbox.
 */
template< typename MESSAGE, typename... ARGS >
void
send_to_agent(
	const so_5::rt::adhoc_agent_definition_proxy_t & receiver,
	ARGS&&... args )
	{
		send< MESSAGE >( receiver, std::forward<ARGS>(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for sending a signal to the agent's direct mbox.
 */
template< typename MESSAGE >
void
send_to_agent( const so_5::rt::agent_t & receiver )
	{
		send< MESSAGE >( receiver );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for sending a signal to the ad-hoc agent's direct mbox.
 */
template< typename MESSAGE >
void
send_to_agent( const so_5::rt::adhoc_agent_definition_proxy_t & receiver )
	{
		send< MESSAGE >( receiver.direct_mbox() );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a delayed message.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		env.single_timer(
				so_5::rt::details::make_message_instance< MESSAGE >(
						std::forward<ARGS>(args)... ),
				to, pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a delayed message.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		send_delayed< MESSAGE >( agent.so_environment(), to, pause,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a delayed message
 * to the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		send_delayed< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for creating and delivering a delayed message
 * to the ad-hoc agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		send_delayed< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a delayed signal.
 */
template< typename MESSAGE >
void
send_delayed(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		so_5::rt::impl::instantiator_and_sender_t<
				MESSAGE,
				so_5::rt::is_signal< MESSAGE >::value > helper;

		helper.send_delayed( env, to, pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a delayed signal.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
void
send_delayed(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		send_delayed< MESSAGE >( agent.so_environment(), to, pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a delayed signal to
 * the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		send_delayed< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for delivering a delayed signal to
 * the ad-hoc agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
void
send_delayed_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause )
	{
		send_delayed< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a periodic message.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return env.schedule_timer( 
				so_5::rt::details::make_message_instance< MESSAGE >(
						std::forward< ARGS >( args )... ),
				to, pause, period );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a periodic message.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				to,
				pause,
				period,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for creating and delivering a periodic message
 * to the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause,
				period,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for creating and delivering a periodic message
 * to the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE, typename... ARGS >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period,
	//! Message constructor parameters.
	ARGS&&... args )
	{
		return send_periodic< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause,
				period,
				std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a periodic signal.
 */
template< typename MESSAGE >
timer_id_t
send_periodic(
	//! An environment to be used for timer.
	so_5::rt::environment_t & env,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		so_5::rt::impl::instantiator_and_sender_t<
				MESSAGE,
				so_5::rt::is_signal< MESSAGE >::value > helper;

		return helper.send_periodic( env, to, pause, period );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a periodic signal.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
timer_id_t
send_periodic(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Mbox for the message to be sent to.
	const so_5::rt::mbox_t & to,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				to,
				pause,
				period );
	}

/*!
 * \since v.5.5.1
 * \brief A utility function for delivering a periodic signal to
 * the agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::agent_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		return send_periodic< MESSAGE >(
				agent.so_environment(),
				agent.so_direct_mbox(),
				pause,
				period );
	}

/*!
 * \since v.5.5.8
 * \brief A utility function for delivering a periodic signal to
 * the ad-hoc agent's direct mbox.
 *
 * Gets the Environment from the agent specified.
 */
template< typename MESSAGE >
timer_id_t
send_periodic_to_agent(
	//! An agent whos environment must be used.
	so_5::rt::adhoc_agent_definition_proxy_t & agent,
	//! Pause for message delaying.
	std::chrono::steady_clock::duration pause,
	//! Period of message repetitions.
	std::chrono::steady_clock::duration period )
	{
		return send_periodic< MESSAGE >(
				agent.environment(),
				agent.direct_mbox(),
				pause,
				period );
	}

/*!
 * \name Helper functions for simplification of synchronous interactions.
 * \{
 */

/*!
 * \since v.5.5.9
 * \brief Make a synchronous request and receive result in form of a future
 * object. Intended to use with messages.
 *
 * \tparam RESULT type of expected result. The std::future<RESULT> will be
 * returned.
 * \tparam MSG type of message to be sent to request processor.
 * \tparam TARGET identification of request processor. Could be reference to
 * so_5::rt::mbox_t, to so_5::rt::agent_t or
 * so_5::rt::adhoc_agent_definition_proxy_t (in two later cases agent's direct
 * mbox will be used).
 * \tparam ARGS arguments for MSG's constructors.
 *
 * \par Usage example:
 * \code
	// For sending request to mbox:
	const so_5::rt::mbox_t & convert_mbox = ...;
	auto f1 = so_5::request_future< std::string, int >( convert_mbox, 10 );
	...
	f1.get();

	// For sending request to agent:
	const so_5::rt::agent_t & a = ...;
	auto f2 = so_5::request_future< std::string, int >( a, 10 );
	...
	f2.get();

	// For sending request to ad-hoc agent:
	auto service = coop.define_agent();
	coop.define_agent().on_start( [service] {
		auto f3 = so_5::request_future< std::string, int >( service, 10 );
		...
		f3.get();
	} );
 * \endcode
 */
template< typename RESULT, typename MSG, typename TARGET, typename... ARGS >
std::future< RESULT >
request_future(
	//! Target for sending a synchronous request to.
	TARGET && who,
	//! Arguments for MSG's constructor params.
	ARGS &&... args )
	{
		using namespace send_functions_details;

		so_5::rt::ensure_not_signal< MSG >();

		return arg_to_mbox( std::forward< TARGET >(who) )
				->template get_one< RESULT >()
				.template make_async< MSG >( std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.9
 * \brief Make a synchronous request and receive result in form of a future
 * object. Intended to use with signals.
 *
 * \tparam RESULT type of expected result. The std::future<RESULT> will be
 * returned.
 * \tparam SIGNAL type of signal to be sent to request processor.
 * This type must be derived from so_5::rt::signal_t.
 * \tparam TARGET identification of request processor. Could be reference to
 * so_5::rt::mbox_t, to so_5::rt::agent_t or
 * so_5::rt::adhoc_agent_definition_proxy_t (in two later cases agent's direct
 * mbox will be used).
 * \tparam FUTURE_TYPE type of funtion return value (detected automatically).
 *
 * \par Usage example:
 * \code
	struct get_status : public so_5::rt::signal_t {};

	// For sending request to mbox:
	const so_5::rt::mbox_t & engine = ...;
	auto f1 = so_5::request_future< std::string, get_status >( engine );
	...
	f1.get();

	// For sending request to agent:
	const so_5::rt::agent_t & engine = ...;
	auto f2 = so_5::request_future< std::string, get_status >( engine );
	...
	f2.get();

	// For sending request to ad-hoc agent:
	auto engine = coop.define_agent();
	coop.define_agent().on_start( [engine] {
		auto f3 = so_5::request_future< std::string, get_status >( engine );
		...
		f3.get();
	} );
 * \endcode
 */
template<
		typename RESULT,
		typename SIGNAL,
		typename TARGET,
		typename FUTURE_TYPE =
				typename std::enable_if<
						so_5::rt::is_signal< SIGNAL >::value, std::future< RESULT >
				>::type >
FUTURE_TYPE
request_future(
	//! Target for sending a synchronous request to.
	TARGET && who )
	{
		using namespace send_functions_details;

		so_5::rt::ensure_signal< SIGNAL >();

		return arg_to_mbox( std::forward< TARGET >(who) )
				->template get_one< RESULT >()
				.template async< SIGNAL >();
	}

/*!
 * \since v.5.5.9
 * \brief Make a synchronous request and receive result in form of a value
 * with waiting for some time. Intended to use with messages.
 *
 * \tparam RESULT type of expected result.
 * \tparam MSG type of message to be sent to request processor.
 * \tparam TARGET identification of request processor. Could be reference to
 * so_5::rt::mbox_t, to so_5::rt::agent_t or
 * so_5::rt::adhoc_agent_definition_proxy_t (in two later cases agent's direct
 * mbox will be used).
 * \tparam DURATION type of waiting indicator. Can be
 * so_5::service_request_infinite_waiting_t or some of std::chrono type.
 * \tparam ARGS arguments for MSG's constructors.
 *
 * \par Usage example:
 * \code
	// For sending request to mbox:
	const so_5::rt::mbox_t & convert_mbox = ...;
	auto r1 = so_5::request_value< std::string, int >( convert_mbox, so_5::infinite_wait, 10 );
	auto r2 = so_5::request_value< std::string, int >( convert_mbox, std::chrono::milliseconds(10), 10 );

	// For sending request to agent:
	const so_5::rt::agent_t & a = ...;
	auto r3 = so_5::request_value< std::string, int >( a, so_5::infinite_wait, 10 );
	auto r4 = so_5::request_value< std::string, int >( a, std::chrono::milliseconds(10), 10 );

	// For sending request to ad-hoc agent:
	auto service = coop.define_agent();
	coop.define_agent().on_start( [service] {
		auto r5 = so_5::request_value< std::string, int >( service, so_5::infinite_wait, 10 );
		auto r6 = so_5::request_value< std::string, int >( service, std::chrono::milliseconds(10), 10 );
	} );
 * \endcode
 */
template<
		typename RESULT,
		typename MSG,
		typename TARGET,
		typename DURATION,
		typename... ARGS >
RESULT
request_value(
	//! Target for sending a synchronous request to.
	TARGET && who,
	//! Time to wait.
	DURATION timeout,
	//! Arguments for MSG's constructor params.
	ARGS &&... args )
	{
		using namespace send_functions_details;

		so_5::rt::ensure_not_signal< MSG >();

		return arg_to_mbox( std::forward< TARGET >(who) )
				->template get_one< RESULT >()
				.get_wait_proxy( timeout )
				.template make_sync_get< MSG >( std::forward< ARGS >(args)... );
	}

/*!
 * \since v.5.5.9
 * \brief Make a synchronous request and receive result in form of a value with
 * waiting for some time. Intended to use with signals.
 *
 * \tparam RESULT type of expected result.
 * returned.
 * \tparam SIGNAL type of signal to be sent to request processor.
 * This type must be derived from so_5::rt::signal_t.
 * \tparam TARGET identification of request processor. Could be reference to
 * so_5::rt::mbox_t, to so_5::rt::agent_t or
 * so_5::rt::adhoc_agent_definition_proxy_t (in two later cases agent's direct
 * mbox will be used).
 * \tparam DURATION type of waiting indicator. Can be
 * so_5::service_request_infinite_waiting_t or some of std::chrono type.
 * \tparam RESULT_TYPE type of funtion return value (detected automatically).
 *
 * \par Usage example:
 * \code
	struct get_status : public so_5::rt::signal_t {};

	// For sending request to mbox:
	const so_5::rt::mbox_t & engine = ...;
	auto r1 = so_5::request_value< std::string, get_status >( engine, so_5::infinite_wait );
	auto r2 = so_5::request_value< std::string, get_status >( engine, std::chrono::milliseconds(10) );

	// For sending request to agent:
	const so_5::rt::agent_t & engine = ...;
	auto r3 = so_5::request_value< std::string, get_status >( engine, so_5::infinite_wait );
	auto r4 = so_5::request_value< std::string, get_status >( engine, std::chrono::milliseconds(10) );

	// For sending request to ad-hoc agent:
	auto engine = coop.define_agent();
	coop.define_agent().on_start( [engine] {
		auto r5 = so_5::request_value< std::string, get_status >( engine, so_5::infinite_wait );
		auto r6 = so_5::request_value< std::string, get_status >( engine, std::chrono::milliseconds(10) );
	} );
 * \endcode
 */
template<
		typename RESULT,
		typename SIGNAL,
		typename TARGET,
		typename DURATION,
		typename RESULT_TYPE =
				typename std::enable_if<
						so_5::rt::is_signal< SIGNAL >::value, RESULT
				>::type >
RESULT_TYPE
request_value(
	//! Target for sending a synchronous request to.
	TARGET && who,
	//! Time for waiting for a result.
	DURATION timeout )
	{
		using namespace send_functions_details;

		so_5::rt::ensure_signal< SIGNAL >();

		return arg_to_mbox( std::forward< TARGET >(who) )
				->template get_one< RESULT >()
				.get_wait_proxy( timeout )
				.template sync_get< SIGNAL >();
	}
/*!
 * \}
 */

} /* namespace so_5 */

