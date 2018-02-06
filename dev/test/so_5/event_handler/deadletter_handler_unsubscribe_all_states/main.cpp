/*
 * A test for so_drop_subscription_for_all_states for deadletter handlers.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>
#include <various_helpers_1/ensure.hpp>

class direct_mbox_case_t
{
	const so_5::agent_t & m_owner;
public :
	direct_mbox_case_t(
		so_5::environment_t & /*env*/,
		const so_5::agent_t & owner )
		:	m_owner(owner)
	{}

	const so_5::mbox_t &
	mbox() const SO_5_NOEXCEPT { return m_owner.so_direct_mbox(); }
};

class mpmc_mbox_case_t
{
	const so_5::mbox_t m_mbox;
public:
	mpmc_mbox_case_t(
		so_5::environment_t & env,
		const so_5::agent_t & /*owner*/ )
		:	m_mbox( env.create_mbox() )
	{}

	const so_5::mbox_t &
	mbox() const SO_5_NOEXCEPT { return m_mbox; }
};

class test_message final : public so_5::message_t {};

class test_signal final : public so_5::signal_t {};

class next_step final : public so_5::signal_t {};
class finish final : public so_5::signal_t {};

class nontemplate_basic_part_t : public so_5::agent_t
{
protected:
	state_t st_test{ this, "test" };
	state_t st_test_passed{ this, "test_passed" };

	int m_deadletters{ 0 };

	void
	actual_deadletter_handler()
	{
		ensure_or_die( 0 == m_deadletters, "m_deadletters must be 0" );
		++m_deadletters;
		this >>= st_test_passed;
	}

	virtual void
	on_next_step( mhood_t<next_step> ) = 0;

public:
	nontemplate_basic_part_t( context_t ctx )
		:	so_5::agent_t( std::move(ctx) )
	{}

	virtual void
	so_define_agent() override
	{
		this >>= st_test;

		st_test_passed
				.on_enter( [this]{ so_5::send< next_step >( *this ); } )
				.event( &nontemplate_basic_part_t::on_finish )
				.event( &nontemplate_basic_part_t::on_next_step );
	}

private:
	void
	on_finish( mhood_t<finish> )
	{
		so_deregister_agent_coop_normally();
	}
};

template< typename Mbox_Case, typename Msg_Type >
class template_basic_part_t : public nontemplate_basic_part_t
{
protected:
	const Mbox_Case m_mbox_holder;

	virtual void
	on_next_step( mhood_t< next_step > ) override
	{
		so_drop_subscription_for_all_states< Msg_Type >( m_mbox_holder.mbox() );

		so_5::send< Msg_Type >( *this );
		so_5::send< finish >( *this );
	}

public:
	template_basic_part_t( context_t ctx )
		:	nontemplate_basic_part_t( std::move(ctx) )
		,	m_mbox_holder( so_environment(), *self_ptr() )
	{}

	virtual void
	so_evt_start() override
	{
		so_5::send<Msg_Type>( m_mbox_holder.mbox() );
	}
};

template< typename Mbox_Case, typename Msg_Type >
class pfn_test_case_t final : public template_basic_part_t< Mbox_Case, Msg_Type >
{
	using base_type_t = template_basic_part_t< Mbox_Case, Msg_Type >;

public:
	pfn_test_case_t( typename base_type_t::context_t ctx )
		: base_type_t( std::move(ctx) ) {}

	virtual void
	so_define_agent() override
	{
		base_type_t::so_define_agent();

		this->so_subscribe_deadletter_handler( this->m_mbox_holder.mbox(),
				&pfn_test_case_t::on_deadletter );
	}	

private:
	void
	on_deadletter( so_5::mhood_t<Msg_Type> )
	{
		this->actual_deadletter_handler();
	}
};

template< typename Mbox_Case, typename Msg_Type >
class lambda_test_case_t final : public template_basic_part_t< Mbox_Case, Msg_Type >
{
	using base_type_t = template_basic_part_t< Mbox_Case, Msg_Type >;

public:
	lambda_test_case_t( typename base_type_t::context_t ctx )
		: base_type_t( std::move(ctx) ) {}

	virtual void
	so_define_agent() override
	{
		base_type_t::so_define_agent();

		this->so_subscribe_deadletter_handler( this->m_mbox_holder.mbox(),
				[this](so_5::mhood_t<Msg_Type>) {
					this->actual_deadletter_handler();
				} );
	}
};

template<
	typename Mbox_Case,
	typename Msg_Type,
	template <class, class> class Test_Agent >
void
introduce_test_agent( so_5::environment_t & env )
{
	env.introduce_coop( [&]( so_5::coop_t & coop ) {
			coop.make_agent< Test_Agent<Mbox_Case, Msg_Type> >();
		} );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( [&]( so_5::environment_t & env ) {
						introduce_test_agent<
								direct_mbox_case_t,
								test_message,
								pfn_test_case_t >( env );
						introduce_test_agent<
								direct_mbox_case_t,
								so_5::mutable_msg<test_message>,
								pfn_test_case_t >( env );
						introduce_test_agent<
								direct_mbox_case_t,
								test_signal,
								pfn_test_case_t >( env );
						introduce_test_agent<
								mpmc_mbox_case_t,
								test_message,
								pfn_test_case_t >( env );
						introduce_test_agent<
								mpmc_mbox_case_t,
								test_signal,
								pfn_test_case_t >( env );

						introduce_test_agent<
								direct_mbox_case_t,
								test_message,
								lambda_test_case_t >( env );
						introduce_test_agent<
								direct_mbox_case_t,
								so_5::mutable_msg<test_message>,
								lambda_test_case_t >( env );
						introduce_test_agent<
								direct_mbox_case_t,
								test_signal,
								lambda_test_case_t >( env );
						introduce_test_agent<
								mpmc_mbox_case_t,
								test_message,
								lambda_test_case_t >( env );
						introduce_test_agent<
								mpmc_mbox_case_t,
								test_signal,
								lambda_test_case_t >( env );
					} );
			},
			20 );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 2;
	}

	return 0;
}
