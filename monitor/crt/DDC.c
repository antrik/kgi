/* ----------------------------------------------------------------------------
**	DDC monitor meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2001	 	Jos Hulzink	
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: DDC.c,v $
**	Revision 1.9  2003/03/15 20:47:31  redmondp
**	-forgot one
**	
**	Revision 1.8  2003/02/11 23:02:46  aldot
**	- remove redundant include
**	  test syncmail for nsouch
**	
**	Revision 1.7  2003/02/09 11:15:33  foske
**	Initial commit of DDC2 testing code.
**	
**	Revision 1.6  2003/02/06 09:37:09  foske
**	Dropped VESA standard for faster DDC1 detection. New module option added
**	monitor_ddc: 0 = no ddc, 1 = ddc2 + fast ddc1, 2 = ddc2 + slow ddc1
**	
**	Revision 1.5  2003/02/02 22:49:34  cegger
**	fix warnings about implicit function calls
**	
**	Revision 1.4  2003/01/18 16:13:43  cegger
**	fix a dangling else issue confusing compilers
**	
**	Revision 1.3  2003/01/14 18:53:02  foske
**	Monitors in DPMS Power Off state now wake up when the DDC1 signal detection starts. This fixes DDC1 on secondary heads.
**	
**	Revision 1.2  2003/01/03 14:53:23  foske
**	DTD offset fixed. Should return correct data now.
**	
**	Revision 1.1  2002/12/09 18:20:24  fspacek
**	- initial version of the unified monitor driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jos_Hulzink
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.9 $"

#define DEBUG_LEVEL 2 

#include <kgi/module.h>

#include "monitor/crt/crt-meta.h"
#include "monitor/crt/DDC.h"
#include "monitor/crt/I2C.h"
#include "monitor/crt/edid.h"

#ifndef KRN_BOOT
#define KRN_BOOT KRN_NOTICE
#endif

extern const crt_monitor_timing_set_t crt_monitor_VGA_timings;
extern const crt_monitor_timing_set_t crt_monitor_VESA_timings;

/* EDID decoding procedures */

/* EDID uses an ASCII format that can, but does not have to end on 0x0A.
 * Linux (e.g. BSD systems) have memccpy to deal with this, but this
 * is not defined in ANSI C. As EDID ASCII strings are always of short size,
 * and this procedure doesn't have to be called often, we deal with the
 * problem ourselves.
 */

static void EDID_ASCII_to_string (kgi_u8_t *data, kgi_u_t maxlen, kgi_u_t offset, 
			   char * dest, kgi_u_t buflen) {
	kgi_u_t j;
	if (maxlen>=buflen) {
		KRN_ERROR ("Buffer too small");
		return;
	}
	for (j=0; j<maxlen; j++) {
		if (data[offset+j]==0x0a) break;	
		dest[j]=data[offset+j];
	}
	dest[j]=0;
	while (j--) dest[j] = (dest[j] == 0x20) ? 0 : dest[j];
}

/* EDID supports a bitfield to define the support of some basic modes.
 * Here we decode that bitfield, and store the detected modes in our
 * timelist array.
 */
static void EDID_Decode_ET (kgi_u8_t *data, crt_monitor_t *crt)
{
	kgi_u_t i,j;
	const crt_monitor_timing_t * ET[3][8] =
	{
		{
			&crt_monitor_VESA_timings.t[8],
			&crt_monitor_VESA_timings.t[7],
			&crt_monitor_VESA_timings.t[5],
			&crt_monitor_VESA_timings.t[4],
			NULL,
			&crt_monitor_VGA_timings.t[1],
			NULL,
			&crt_monitor_VGA_timings.t[0]
		},
		{
			&crt_monitor_VESA_timings.t[20],
			&crt_monitor_VESA_timings.t[14],
			&crt_monitor_VESA_timings.t[13],
			&crt_monitor_VESA_timings.t[12],
			NULL,
			NULL,
			&crt_monitor_VESA_timings.t[10],
			&crt_monitor_VESA_timings.t[9]
		},
		{
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
		}
	};

	KRN_BOOT("-- Standard Timings --");
	
	for (i = 0; i < 3; i++) {
	  
		for (j = 0; j < 8; j++) {
		
			if ((data[i] & (1<<j)) != 0) {
			
				if (ET[i][j] != 0) {
				
					if (crt->num_fixed >=
					    CRT_MONITOR_MAX_FIXED_TIMINGS) {
					
						KRN_ERROR("Too many fixed timings(%i)",
							  crt->num_fixed);
						return;
					}

					crt->fixed[crt->num_fixed] = ET[i][j];
					if (crt->fixed[crt->num_fixed]->name) {
						KRN_BOOT("%s", crt->fixed[crt->num_fixed]->name);
					}
					crt->num_fixed++;
				}
				else {
				
					KRN_DEBUG(3,"ET: EDID contains unhandled bit");
				}
			} 
		}
	}
}

static void EDID_Decode_STI(kgi_u8_t *c, crt_monitor_t *crt)
{
	kgi_s_t i, hsize, vsize = 0;

	KRN_BOOT(" -- Additional Timings --");

	for (i = 0; i < STD_TIMINGS; i++) {
		if (STD_TIMING_ID != 257) {
			hsize = HSIZE1;
			VSIZE1(vsize);
			KRN_BOOT("Mode %d x %d @ %d Id=%d", hsize, vsize,
			       REFRESH_R, STD_TIMING_ID);
			c += STD_TIMING_INFO_LEN;
		}
	}
}

static void EDID_Decode_DTD(kgi_u8_t *data, crt_monitor_t *crt)
{
	char ascii[14];	
	kgi_u_t i, n, offset=0;

	for (i=0; i<DET_TIMINGS; i++) {
	if ((data[offset] == 0) && (data[offset+1] == 0))
	{
		/* Datablock is Monitor Descriptor Data */
		switch (data[offset+3]) {
		case SERIAL_NUMBER:
			EDID_ASCII_to_string (data, 13, offset+5, ascii, 14);
			strncpy(crt->kgim.vendor, ascii, 14);
			KRN_DEBUG(1, "vendor: %s", crt->kgim.vendor);
			break;
			
		case ASCII_STR:
			EDID_ASCII_to_string (data, 13, offset+5, ascii, 14);
			if (ascii[0]) {
				KRN_DEBUG(1, "ASCII data string: %s", ascii);
			}
			break;
			
		case MONITOR_RANGES:
			KRN_DEBUG (1, "  MDD Monitor Range Limits");
			if (crt->num_vfreq >= CRT_MONITOR_MAX_FREQ_RANGES ||
				crt->num_hfreq >= CRT_MONITOR_MAX_FREQ_RANGES||
				crt->num_dclk >= CRT_MONITOR_MAX_FREQ_RANGES) {
				
				KRN_ERROR ("Too many frequency ranges");
				return;
			}
			crt->vfreq[crt->num_vfreq].min = data[offset+5];
			crt->vfreq[crt->num_vfreq].max = data[offset+6];
			crt->num_vfreq++;
			crt->hfreq[crt->num_hfreq].min = data[offset+7] * 1000;
			crt->hfreq[crt->num_hfreq].max = data[offset+8] * 1000;
			crt->num_hfreq++;
			crt->dclk[crt->num_dclk].min = 0;
			crt->dclk[crt->num_dclk].max = data[offset+9]*10000000;
			crt->num_dclk++;
			break;
			
		case MONITOR_NAME:
			EDID_ASCII_to_string (data, 13, offset+5, ascii, 14);
			strncpy(crt->kgim.vendor, ascii, 14);
			KRN_DEBUG(1, "stuff: %s", ascii);
			break;
			
		case ADD_COLOR_POINT:
			KRN_DEBUG (1,"  MDD Color Point");
			break;
			
		case ADD_STD_TIMINGS:
			KRN_DEBUG (1,"  MDD Standard Timing Identifiers<-CHECK !");
			for (n = 5; n < 17; n += 2) {
				
				EDID_Decode_STI (data + offset + n, crt);
			}
			break;
		default:
			if (data[offset+3] < 0x10) {
				KRN_DEBUG (1,"  MDD: Unhandled extension.");
			} else {
				KRN_DEBUG (1,"  MDD: Unknown Data Tag.");
			}
		break;
	  }
	} else {
		/* Datablock is Detailed Timing Descriptor */
		KRN_DEBUG (1,"DTD Datablock");
		n = (data [offset] + 256 * data [offset+1]) * 10000;
		KRN_DEBUG (2,"  Pixelclock:          %d", n);
		n = data [offset + 2] + ((data [offset + 4] << 4) & 0xf00);
		KRN_DEBUG (2,"  Horizontal active:   %d", n);
		n = data [offset + 3] + ((data [offset + 4] << 8) & 0xf00);
		KRN_DEBUG (2,"  Horizontal Blanking: %d", n);
		n = data [offset + 5] + ((data [offset + 7] << 4) & 0xf00);
		KRN_DEBUG (2,"  Vertical active:     %d", n);
		n = data [offset + 6] + ((data [offset + 7] << 8) & 0xf00);
		KRN_DEBUG (2,"  Vertical Blanking:   %d", n);
		n = data [offset + 8] + ((data [offset + 11] << 2) & 0x300);
		KRN_DEBUG (2,"  Hor. Sync Offset:    %d", n);
		n = data [offset + 9] + ((data [offset + 11] << 4) & 0x300);
		KRN_DEBUG (2,"  Hor. Sync Width:     %d", n);
		n = (data[offset+10] & 0x0f)+((data[offset+11] << 4) & 0x30);
		KRN_DEBUG (2,"  Ver. Sync Width:     %d", n);
		n = ((data[offset+10]&0xf0)>>4)+((data[offset+11] << 2) & 0x30);
		KRN_DEBUG (2,"  Ver. Sync Offset:    %d", n);
		n = data [offset + 12] + ((data [offset + 14] << 4) & 0x300);
		KRN_DEBUG (2,"  Hor. Image Size:     %d", n);
		n = data [offset + 13] + ((data [offset + 14] << 8) & 0x300);
		KRN_DEBUG (2,"  Ver. Image Size:     %d", n);
		n = data [offset + 15];
		KRN_DEBUG (2,"  Hor. Border:         %d", n);
		n = data [offset + 16];
		KRN_DEBUG (2,"  Ver. Border:         %d", n);
	}

	offset += DET_TIMING_INFO_LEN;
	}
}
	
static void EDID_Decode_Data_V1 (kgi_u8_t *edid_data, crt_monitor_t *crt)
{
	kgi_u_t version, revision;
	char man[4], *c = NULL;

	man[0] = ((edid_data[0x08]>>2)&0x1f)+'A'-1;
	man[1] = ((edid_data[0x08]<<3)&0x18)+((edid_data[0x09]>>5)&0x07)+'A'-1;
	man[2] = (edid_data[0x09]&0x1f)+'A'-1;
	man[3] = '\0';

	version = GET(VERSION_SECTION + V_VERSION);
	revision = GET(VERSION_SECTION + V_REVISION);

	KRN_BOOT("EDID revision: %d.%d", version, revision);
	KRN_BOOT("Display Size: %dx%d cm.",
	       GET(DISPLAY_SECTION + D_HSIZE),
	       GET(DISPLAY_SECTION + D_VSIZE));
	KRN_BOOT("VESA DPMS support: %s%s%s",
	       (GET(DISPLAY_SECTION + FEAT_S)&0x80 ? "Stand-by ":""),
	       (GET(DISPLAY_SECTION + FEAT_S)&0x40 ? "Suspend ":""),
	       (GET(DISPLAY_SECTION + FEAT_S)&0x20 ? "Active off ":""));
	
	if (GET(DISPLAY_SECTION + FEAT_S)&0x02) {
		KRN_DEBUG(1,"First DTD is Preferred Timing");
	}
	if (GET(DISPLAY_SECTION + FEAT_S)&0x01) {
		KRN_DEBUG(1,"Monitor survives GTF calculations");
	}
	
	EDID_Decode_ET (GET_ARRAY(ESTABLISHED_TIMING_SECTION), crt);

	EDID_Decode_STI (GET_ARRAY(STD_TIMING_SECTION), crt);

	c = GET_ARRAY(DET_TIMING_SECTION);
	if (version == 1 && revision >= 1 && IS_MONITOR_DESC) {
		EDID_Decode_DTD (GET_ARRAY(DET_TIMING_SECTION), crt);
	}
}

static void EDID_Decode_Data_V2 (kgi_u8_t *edid_data, crt_monitor_t *crt)
{
	KRN_DEBUG (0, "EDID V2 not decoded yet.");
}

static void EDID_Decode_Data (kgi_u8_t *edid_data, crt_monitor_t *crt)
{
	kgi_u_t edid_ver,edid_rel;
	
	if (edid_data[0]!=0) {
	  /* EDID v.2 and higher */
	  edid_ver=(edid_data[0]>>4) & 0xf;
	  edid_rel=edid_data[0] & 0xf;
	} else {
	  /* EDID v.1 */
	  edid_ver=edid_data[18];
	  edid_rel=edid_data[19];
	}
	KRN_DEBUG (1,"Decoding EDID datablock revision %d.%d",
		   edid_ver, edid_rel);
	switch (edid_ver) {
		case 1:
			EDID_Decode_Data_V1 (edid_data, crt);
			break;
		case 2:
			EDID_Decode_Data_V2 (edid_data, crt);
			break;
		default: 
			KRN_DEBUG (0, "Unhandled EDID version %d", edid_ver);
			break;
	}	
}

/* DDC2 handling */
#if 0
kgi_error_t DDC2_Read_Configuration (crt_monitor_t *crt, 
	crt_monitor_io_t *crt_io, const kgim_options_t *options) {
	I2CDevRec d;
	int i;
	/* Experimental code to check the I2C functionality */	
	d.pI2CBus = &(crt_io->DDC2_io);
	d.SlaveAddr = 0xA0;
	((kgim_i2c_io_t*)(d.pI2CBus))->chipset_io = crt_io; 
	i2c_BusInit(d.pI2CBus);
	for (i=0x00; i<=0xFF; i+=2) {
		/* Address A0 should respond, it is the DDC2 ROM */
		if (i2c_ProbeAddress (d.pI2CBus, i))
			KRN_NOTICE ("i2c %i: NO");
		else
			KRN_NOTICE ("i2c %i: YES");
	}
	i2c_DevInit(&d);
	
	return -KGI_ERRNO(MONITOR, UNKNOWN);
}
#endif
/* DDC1 handling*/

/* The idea: DDC1 is a contiuous bitstream. We get it in as 128 x 9 bits
 * and the DDC driver will find the sync bit, order and CRC the data.
 */

static kgi_error_t DDC1_Fetch_Data(kgi_u16_t * ddc1_data, 
			   crt_monitor_io_t *crt_io, kgi_u_t mode)
{
	kgi_u_t i,j;
	/* When mode = 2, compliance to the VESA standard is requested. If the
	 * monitor returns no signal, this procedure can take up to 17 seconds.
	 * Otherwise we try to find DDC 1 data for only 16 byte cycles (2 secs).
	 * In very rare cases, this might cause the detection to miss the
	 * signal on monitors with almost empty DDC data structs,
	 * though I've never seen such situation.
	 */
	kgi_u_t n = (mode == 2 ? 128 * 9 : 16 * 9);  
	CRT_MONITOR_DDC_INIT(crt_io);
	
	/* First check for activity on the DDC1 line.
	 * This must be done on VGA VSYNC speed to prevent smoking CRTs
	 */	
	j = CRT_MONITOR_DDC_GET_SDA(crt_io);
	for (i=n;i>0;i--) { 
		CRT_MONITOR_DDC_SET_VSYNC(crt_io, 1);
		kgi_udelay (63); // 2 lines vsync  
		CRT_MONITOR_DDC_SET_VSYNC(crt_io, 0);
		kgi_udelay (7080); // 447 lines no vsync  
		kgi_udelay (7143); // (449 lines = 14290 us or 1/70 th sec) 
		if (j != CRT_MONITOR_DDC_GET_SDA(crt_io)) break;
	}
	if (i) {
	  KRN_DEBUG (1, "DDC1: Signal detected in %d cycli.",n-i+1);
	} else {
	  KRN_DEBUG (1, "DDC1: No signal detected.");
	  return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	for (i=0;i<128;i++) {
	  ddc1_data[i]=0;
 	  for (j=0; j<9; j++) {
	    /* This loop gets DDC1 data in at 16.6 kHz 
	     * DDC1 limit is at 25 kHz, but this is fast enough.
	     * the kgi_udelay's are unbalanced to meet all timing constraints
	     * and still be as fast as possible. This code doesn't care
	     * what the rising and falling edge is, and is thus easier for 
	     * chipset driver programmers. To make it faster,
	     * you have to find that out. (DDC version 3, page 28)
	     */

	    CRT_MONITOR_DDC_SET_VSYNC(crt_io, 0);
	    kgi_udelay (25); // 20 = min pulse width + 5  
	    CRT_MONITOR_DDC_SET_VSYNC(crt_io, 1);
	    kgi_udelay (35); // 30 = min data ready time + 5 
	    /* Shift & get the next databit in */
    	    ddc1_data[i]<<=1;
	    ddc1_data[i]|= CRT_MONITOR_DDC_GET_SDA(crt_io);
	  }
	  KRN_DEBUG (4, "DDC raw input: %x", ddc1_data[i]);	
	}
	CRT_MONITOR_DDC_DONE(crt_io);
	return -KGI_EOK;
}

#if 0
static kgi_error_t DDC1_Test(crt_monitor_io_t *crt_io)
{
	kgi_u_t count = 9 * 6;	/* header is 6 bytes with 9 bits per byte */
	kgi_u8_t data, previous;

	previous = CRT_MONITOR_DDC_GET_SDA(crt_io);
	while (count--) {
		data = CRT_MONITOR_DDC_GET_SDA(crt_io);
		if (data != previous)
			return KGI_EOK;
		previous = data;
	}
	return -KGI_ERRNO(MONITOR, UNKNOWN);
}
#endif

kgi_error_t DDC1_Read_Configuration (crt_monitor_t *crt, 
	crt_monitor_io_t *crt_io, const kgim_options_t *options)
{
	kgi_u_t header=0, checksum, syncbit, error;
	kgi_u_t k,l,i,j;
	kgi_u16_t edid_words[256];
	kgi_u8_t edid_data[256];
	
	kgi_u_t retries=2;
	kgi_u_t data_ok=0;
	
	while ((!data_ok) && (retries--)) { 
	
		KRN_DEBUG (1,"DDC1: Looking for data. Retries: %d", retries);
		/* The communication might loose a bit, so if we get data
		 * that's incorrect, we try 3 times. If we get no data at all
		 * we bail out immediately.
		 */
		if (DDC1_Fetch_Data (edid_words, crt_io, options->monitor->ddc)) 
		{
			KRN_DEBUG (1,"DDC1: No signal. Bailing out.");
			retries=0;
			continue;
		};

		syncbit=9;
		error=0;

	/* Syncbit detection algorithm: The DDC standard defines all
	 * syncbits within a frame to have the same value (0 or 1).
	 * As DDC also defines that the databytes contain at least 
	 * both FF and 00, the syncbit is the only bit for which
	 * the logical AND of all 9-bit values will be 1, or the logical 
	 * OR will be 0. Syncbits are recommended all 1, but allowed all 0
	 */
		k=0; l=0x1ff;
		for (i=0;i<128;i++) {
	  		k|=edid_words[i];
	  		l&=edid_words[i];
		}
		for (i=0; i<9; i++) {
	  		if (((k & (1 << i)) == 0) || ((l & (1 << i)) != 0)) {
	    			if (syncbit > 8) {syncbit=i;} else {error = 1;}
	  		}
		}
		if (error != 0 || syncbit == 9) {
			KRN_DEBUG (1,"Error scanning DDC1 data stream");
			continue;
		}
	
		/* Now the syncbit has been found, we have to convert the 
		 * bitstream into bytes.
		 */
		edid_words[128]=edid_words[0];
		for (i=0;i<128;i++) {
	  		edid_words[i]=	((edid_words[i] << (8-syncbit))  |
					 (edid_words[i+1])>>(syncbit+1)) & 
					0xff;		 
		}
		/* Now the bitstream has been converted into bytes,
	 	* calculate the total of all bytes. This should be 0 modulo 256.
 	 	*/

		checksum=0;	
		for (i=0;i<128;i++) checksum+=edid_words[i];
		if  ((error=(checksum & 0xff)) != 0) {
		  	KRN_DEBUG (1,"DDC1 Checksum error: %d",error);	
			continue;
		}
	
	/* The checksum is ok. Now we have to find the beginning of the
	 * EDID data. DDC1 only provides EDID version 1, so we can check
	 * for the EDID v.1 00FFFFFFFFFFFF00 header. This must be done
	 * byte by byte for endianness reasons.
	 */
		for (i=0; i< 128; i++) {
	  		for (j=0; j<8; j++) {
	    			if (edid_words[(i+j)&127]!=(j==0||j==7?0:0xff)) 
	      			break;
	  		}
	  		if (j==8) break;	
		}
		if (((header=i)==128)!=0) {
 	  		KRN_DEBUG(1,"No header found.");
			continue;
		}
		data_ok=1;
	} /* while */
	if (data_ok == 0) {
		KRN_NOTICE ("DDC1 data fetching failed.");
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
	/* Now reorder data to make the datablock EDID compatible
	 */
	for (i=0;i<header;i++) edid_words[i+128]=edid_words[i];
	for (i=0;i<128;i++) edid_words[i]=edid_words[i+header];
	for (i=0;i<128;i++) edid_data[i]=edid_words[i]&0xff;

	/* Convert the raw edid data into monitor mode parameters */	
	EDID_Decode_Data (edid_data, crt);
	return -KGI_EOK;
}
