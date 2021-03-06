// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Robotron K8915

2010-08-30

When it says DIAGNOSTIC RAZ P, press enter.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"

class k8915_state : public driver_device
{
public:
	k8915_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void k8915(machine_config &config);

	void init_k8915();

private:
	DECLARE_WRITE8_MEMBER(k8915_a8_w);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	uint8_t m_framecnt;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


WRITE8_MEMBER( k8915_state::k8915_a8_w )
{
// seems to switch ram and rom around.
	if (data == 0x87)
		membank("boot")->set_entry(0); // ram at 0000
	else
		membank("boot")->set_entry(1); // rom at 0000
}

void k8915_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).bankrw("boot");
	map(0x1000, 0x17ff).ram().share("videoram");
	map(0x1800, 0xffff).ram();
}

void k8915_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x50, 0x53).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x58, 0x5b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xa8, 0xa8).w(FUNC(k8915_state::k8915_a8_w));
}

/* Input ports */
static INPUT_PORTS_START( k8915 )
INPUT_PORTS_END

void k8915_state::machine_reset()
{
	membank("boot")->set_entry(1);
}

void k8915_state::init_k8915()
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0x10000);
}

uint32_t k8915_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;

				if (ra < 9)
				{
					chr = m_p_videoram[x];

					/* Take care of flashing characters */
					if ((chr & 0x80) && (m_framecnt & 0x08))
						chr = 0x20;

					chr &= 0x7f;

					gfx = m_p_chargen[(chr<<4) | ra ];
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=80;
	}
	return 0;
}


MACHINE_CONFIG_START(k8915_state::k8915)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(4'915'200) / 2)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(k8915_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, XTAL(4'915'200) / 2)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("ctc", z80ctc_device, trg2))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(4'915'200) / 2)
	MCFG_Z80CTC_ZC2_CB(WRITELINE("sio", z80sio_device, rxtxcb_w))

	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'915'200) / 2));
	sio.out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio", z80sio_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("sio", z80sio_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio", z80sio_device, ctsb_w))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( k8915 )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "k8915.bin", 0x10000, 0x1000, CRC(ca70385f) SHA1(a34c14adae9be821678aed7f9e33932ee1f3e61c))

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP( 1982, k8915,  0,      0,      k8915,   k8915, k8915_state, init_k8915, "Robotron", "K8915",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
