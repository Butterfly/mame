// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_signal.h
 *
 */

#ifndef NLD_SIGNAL_H_
#define NLD_SIGNAL_H_

#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// MACROS
// ----------------------------------------------------------------------------------------

#define NETLIB_SIGNAL(_name, _num_input, _check, _invert)                           \
	class NETLIB_NAME(_name) : public net_signal_t<_num_input, _check, _invert>     \
	{                                                                               \
	public:                                                                         \
		ATTR_COLD NETLIB_NAME(_name) ()                                             \
		: net_signal_t<_num_input, _check, _invert>() { }                           \
	}

// ----------------------------------------------------------------------------------------
// net_signal_t
// ----------------------------------------------------------------------------------------

template <int _numdev, int _check, int _invert>
class net_signal_t : public netlist_device_t
{
public:
	net_signal_t()
	: netlist_device_t(), m_active(1)
	{
	}

	ATTR_COLD void start()
	{
		const char *sIN[8] = { "A", "B", "C", "D", "E", "F", "G", "H" };

		register_output("Q", m_Q[0]);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_i[i]);
		}
		save(NLNAME(m_active));
	}

	ATTR_COLD void reset()
	{
		m_Q[0].initial(0);
		m_active = 1;
	}

	ATTR_HOT inline netlist_sig_t process()
	{
		for (int i = 0; i< _numdev; i++)
		{
			this->m_i[i].activate();
			if (INPLOGIC(this->m_i[i]) == _check)
			{
				for (int j = 0; j < i; j++)
					this->m_i[j].inactivate();
				for (int j = i + 1; j < _numdev; j++)
					this->m_i[j].inactivate();
				return _check ^ (1 ^ _invert);
			}
		}
		return _check ^ _invert;
	}

	ATTR_HOT virtual void inc_active()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};
		nl_assert(netlist().use_deactivate());
		if (++m_active == 1)
		{
			// FIXME: need to activate before time is accessed !
			netlist_time mt = netlist_time::zero;
			for (int i = 0; i< _numdev; i++)
			{
				if (this->m_i[i].net().time() > mt)
					mt = this->m_i[i].net().time();
			}
			netlist_sig_t r = process();
			m_Q[0].net().set_Q_time(r, mt + times[r]);
		}
	}

	ATTR_HOT virtual void dec_active()
	{
		nl_assert(netlist().use_deactivate());
		if (--m_active == 0)
		{
			for (int i = 0; i< _numdev; i++)
				m_i[i].inactivate();
		}
	}

	virtual void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(15), NLTIME_FROM_NS(22)};

		netlist_sig_t r = process();
		OUTLOGIC(this->m_Q[0], r, times[r]);
	}

public:
	netlist_logic_input_t m_i[_numdev];
	netlist_logic_output_t m_Q[1];
	INT32 m_active;
};

#endif /* NLD_SIGNAL_H_ */
