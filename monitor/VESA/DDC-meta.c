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
**	$Log: DDC-meta.c,v $
**	Revision 1.9  2003/11/16 21:41:05  aldot
**	- add missing parenthesis
**	
**	Revision 1.8  2003/01/03 14:43:02  foske
**	DTD bug fixed.
**	
**	Revision 1.7  2002/12/02 10:18:31  foske
**	Fixed error detection in DDC1 parsing code.
**	
**	Revision 1.6  2002/07/30 09:13:05  foske
**	Detailed Timing Descriptor decoding in place. My monitor returns nonsense.
**	Don't use the DTD results for now.
**	
**	Revision 1.5  2002/07/29 18:39:24  foske
**	Cleanup, decoder produces output that can actually be used for modesetting.
**	What is left: the actual modecalculations, resource management (for multihead),
**	a proper interface to the chipset driver.
**	
**	Revision 1.4  2002/07/28 12:23:52  foske
**	Fixed KRN_DEBUG issue
**	Cleaned up DDC1 procedures, accept right parameters, etc.
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jos_Hulzink
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.9 $"

#undef	DEBUG_LEVEL
#define	DEBUG_LEVEL	3

#include <kgi/module.h>
#include <kgi/kgi.h> 

#include "monitor/VESA/DDC-meta.h"
#include "monitor/VESA/DDC.h" 

DDC_monitor_data_t ddc_monitor_data;

void print_monitor_data (DDC_monitor_data_t *dmd) {
	kgi_u_t i;

	KRN_NOTICE (dmd->id);
	for (i=0; i<dmd->range.n; i++) {
	  KRN_NOTICE ("hrange: %d-%d Hz, vrange: %d-%d Hz, prange: %d-%d Hz",
		dmd->range.data[i].hclk.min,
		dmd->range.data[i].hclk.max,
		dmd->range.data[i].vclk.min,
		dmd->range.data[i].vclk.max,
		dmd->range.data[i].pclk.min,
		dmd->range.data[i].pclk.max);
	}	
	for (i=0; i<dmd->fixed.n; i++) {
	  KRN_NOTICE ("x: %d, y: %d, f: %d Hz",
		dmd->fixed.data[i].x,
		dmd->fixed.data[i].y,
		dmd->fixed.data[i].r);
	}	
}

/* EDID decoding procedures */

/* EDID uses an ASCII format that can, but does not have to end on 0x0A.
 * Linux (e.g. BSD systems) have memccpy to deal with this, but this
 * is not defined in ANSI C. As EDID ASCII strings are always of short size,
 * and this procedure doesn't have to be called often, we deal with the
 * problem ourselves.
 */

void EDID_ASCII_to_string (kgi_u16_t *data, kgi_u_t maxlen, kgi_u_t offset, 
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
}

/* EDID supports a bitfield to define the support of some basic modes.
 * Here we decode that bitfield, and store the detected modes in our
 * timelist array.
 */
static void EDID_Decode_ET (kgi_u16_t *data,kgi_u_t offset, 
			     DDC_monitor_data_t *dmd) {
	kgi_u_t n;
	struct {
		kgi_u_t x,y,r;
	} ET [3][8] = {
	       {{ 800, 600, 60 },
		{ 800, 600, 56 },
		{ 640, 480, 75 },
		{ 640, 480, 72 },
		{ 640, 480, 67 },
		{ 640, 480, 60 },
		{ 720, 400, 88 },
		{ 720, 400, 70 }},
	       {{ 1280, 1024, 75 },
		{ 1024, 768, 75 },
		{ 1024, 768, 70 },
		{ 1024, 768, 60 },
		{ 1024, 768, 87 },
		{ 832, 624, 75 },
		{ 800, 600, 75 },
		{ 800, 600, 72 }},
	       {{ 0, 0, 0 },  
		{ 0, 0, 0 },  
		{ 0, 0, 0 },  
		{ 0, 0, 0 },  
		{ 0, 0, 0 },  
		{ 0, 0, 0 },  
		{ 0, 0, 0 },  
		{ 1152, 870, 75 }}};  
	
	kgi_u_t i,j;
	for (i=0;i<3;i++) {
		for (j=0;j<8;j++) {
			if ((data[offset+i]&(1<<j))!=0) {
			  	if (ET[i][j].x!=0) {
					if (dmd->fixed.n >= DMD_FIXED_MAX_N) {
		  			KRN_ERROR("DMD_FIXED_MAX_N too small.");
					return;
			  	}
				n = dmd->fixed.n++;		
				dmd->fixed.data[n].x = ET[i][j].x;
				dmd->fixed.data[n].y = ET[i][j].y;
				dmd->fixed.data[n].r = ET[i][j].r;
			  } else {
				KRN_DEBUG (1,"ET: EDID contains unhandled bit");
			  }
			} 
		}
	}
}
static void EDID_Decode_STI (kgi_u16_t *data,kgi_u_t offset, 
			     DDC_monitor_data_t *dmd) {
	kgi_u_t n,i;
	kgi_u_t rx[4]={1,4,5,16};
	kgi_u_t ry[4]={1,3,4,9};	

	if (data[offset]==0x01) return;
	else {
		if (dmd->fixed.n >= DMD_FIXED_MAX_N) {
		  	KRN_ERROR ("DMD_FIXED_MAX_N too small. Data lost.");
			return;
		}
		n = dmd->fixed.n++;		
		dmd->fixed.data[n].x = (31+data[offset])<<3;
		i=(data[offset+1]>>6)&0x3;
		dmd->fixed.data[n].y = ry[i]*dmd->fixed.data[n].x/rx[i];
		dmd->fixed.data[n].r = (data[offset+1]&0x3f)+60;
	}
}

static void EDID_Decode_DTD (kgi_u16_t *data,kgi_u_t offset,
			     DDC_monitor_data_t *dmd) {
	char ascii[14];	
	kgi_u_t n;

	if ((data[offset]==0) && (data[offset+1]==0)) {
	  /* Datablock is Monitor Descriptor Data */
	  KRN_DEBUG (1,"MDD Datablock");
	  switch (data[offset+3]) {
	    case 0xff:
		EDID_ASCII_to_string (data, 13, offset+5, ascii, 14);
		strcat (dmd->id, ascii);
		break;
	    case 0xfe:
		EDID_ASCII_to_string (data, 13, offset+5, ascii, 14);
		strcat (dmd->id, ascii);
		break;
	    case 0xfd:
		KRN_DEBUG (1,"  MDD Monitor Range Limits");
		if (dmd->range.n >= DMD_RANGE_MAX_N) {
		  	KRN_ERROR ("DMD_RANGE_MAX_N too small. Data lost.");
			return;
		}
		n = dmd->range.n++;
		dmd->range.data[n].vclk.min=data[offset+5];
		dmd->range.data[n].vclk.max=data[offset+6];
		dmd->range.data[n].hclk.min=data[offset+7]*1000;
		dmd->range.data[n].hclk.max=data[offset+8]*1000;
		dmd->range.data[n].pclk.min=0;
		dmd->range.data[n].pclk.max=data[offset+9]*10000000;
		break;
	    case 0xfc:
		EDID_ASCII_to_string (data, 13, offset+5, ascii, 14);
		strcat (dmd->id, ascii);
		break;
	    case 0xfb:
		KRN_DEBUG (1,"  MDD Color Point");
		break;
	    case 0xfa:
		KRN_DEBUG (1,"  MDD Standard Timing Identifiers<-CHECK !");
		for (n=5; n<17; n+=2) EDID_Decode_STI (data, offset + n, dmd);
		break;
	    default:
		KRN_DEBUG (1,"  MDD Unhandled.");
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
}
	
static void EDID_Decode_Data_V1 (edid_data_t edid_data,
				 DDC_monitor_data_t *dmd) {
	kgi_u_t i;
	char man[4];

	man[0] = ((edid_data[0x08]>>2)&0x1f)+'A'-1;
	man[1] = ((edid_data[0x08]<<3)&0x18)+((edid_data[0x09]>>5)&0x07)+'A'-1;
	man[2] = (edid_data[0x09]&0x1f)+'A'-1;
	man[3] = '\0';
	strcpy (dmd->id, man);

	KRN_DEBUG (1,"EDID revision: %d,%d", edid_data[0x12], edid_data[0x13]);
	KRN_DEBUG (1,"Display Size: %dx%d cm.",
		   edid_data[0x15],
		   edid_data[0x16]);
	KRN_DEBUG (1,"VESA DPMS support: %s%s%s",
		   (edid_data[0x18]&0x80 ? "Stand-by ":""),
		   (edid_data[0x18]&0x40 ? "Suspend ":""),
		   (edid_data[0x18]&0x20 ? "Active off ":""));
	
	EDID_Decode_ET (edid_data, 35, dmd);
	for (i=0; i<8; i++) EDID_Decode_STI (edid_data, 38+2*i, dmd);	
	for (i=0; i<4; i++) EDID_Decode_DTD (edid_data, 54+18*i, dmd);
}

static void EDID_Decode_Data_V2 (edid_data_t edid_data,
				 DDC_monitor_data_t *dmd) {
	KRN_DEBUG (0, "EDID V2 not decoded yet.");
}

static void EDID_Decode_Data (edid_data_t edid_data,
			      DDC_monitor_data_t *dmd) {
	kgi_u_t checksum=0;
	kgi_u_t i,j;
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
			EDID_Decode_Data_V1 (edid_data, dmd);
			break;
		case 2:
			EDID_Decode_Data_V2 (edid_data, dmd);
			break;
		default: 
			KRN_DEBUG (0, "Unhandled EDID version %d", edid_ver);
			break;
	}	
}

/* DDC1 handling*/

/* The idea: DDC1 is a contiuous bitstream. We get it in as 128 x 9 bits
 * and the DDC driver will find the sync bit, order and CRC the data.
 */

#include "monitor/VESA/DDC.h" 

static kgi_error_t DDC1_Fetch_Data(kgi_u16_t * ddc1_data, 
				   struct ddc1_io_t *ddc1_io) {
	kgi_u_t i,j;

	KRN_DEBUG (1, "Entering DDC1_Fetch_Data");	
      
	ddc1_io->init(ddc1_io->io);
	
	/* First check for activity on the DDC1 line.
	 * This must be done on VGA VSYNC speed to prevent smoking CRTs
	 * Fixme: create accurate VGA VSYNC instead of 50/50 block
	 */	
	j=ddc1_io->get_sda(ddc1_io->io);
	for (i=1152;i>0;i--) { 
		ddc1_io->set_scl(ddc1_io->io,0);
		kgi_udelay (7143); // 1/2 * 1/70 Sec  
		ddc1_io->set_scl(ddc1_io->io,1);
		kgi_udelay (7143); // 1/2 * 1/70 Sec
		if (j!=ddc1_io->get_sda(ddc1_io->io)) break;
	}
	if (i) {
	  KRN_DEBUG (1, "DDC1: Signal detected in %d cycli, fetching data.",1153-i);
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

	    ddc1_io->set_scl(ddc1_io->io,0);
	    kgi_udelay (25); // 20 = min pulse width + 5  
	    ddc1_io->set_scl(ddc1_io->io,1);
	    kgi_udelay (35); // 30 = min data ready time + 5 
	    /* Shift & get the next databit in */
    	    ddc1_data[i]<<=1;
	    ddc1_data[i]|=ddc1_io->get_sda(ddc1_io->io);
	  }
	  KRN_DEBUG (4, "DDC raw input: %x", ddc1_data[i]);	
	}
	ddc1_io->done(ddc1_io->io);
	return -KGI_ERRNO(MONITOR, EOK);
}

void DDC1_Read_Configuration (struct ddc1_io_t *ddc1_io) {
	kgi_u_t header=0, checksum, syncbit, error;
	kgi_u_t k,l,i,j;
	edid_data_t edid_data;
	
	kgi_u_t retries=3;
	kgi_u_t data_ok=0;
	
	while ((!data_ok) && (--retries)) { 
		KRN_DEBUG (1,"DDC1: Looking for data. Retries: %d", retries);
		DDC1_Fetch_Data (edid_data, ddc1_io);

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
	  		k|=edid_data[i];
	  		l&=edid_data[i];
		}
		for (i=0; i<9; i++) {
	  		if (((k & (1 << i)) == 0) || ((l & (1 << i)) != 0)) {
	    			if (syncbit > 8) {syncbit=i;} else {error = 1;}
	  		}
		}
		if ((error != 0) || (syncbit == 9)) {
			KRN_DEBUG (1,"Sync bit not found in DDC1 data stream");
			continue;
		}
	
		/* Now the syncbit has been found, we have to convert the 
		 * bitstream into bytes.
		 */
		edid_data[128]=edid_data[0];
		for (i=0;i<128;i++) {
	  		edid_data[i]=	((edid_data[i] << (8-syncbit))  |
					 (edid_data[i+1])>>(syncbit+1)) & 
					0xff;		 
		}
		/* Now the bitstream has been converted into bytes,
	 	* calculate the total of all bytes. This should be 0 modulo 256.
 	 	*/

		checksum=0;	
		for (i=0;i<128;i++) checksum+=edid_data[i];
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
	    			if (edid_data[(i+j)&127]!=(j==0||j==7?0:0xff)) 
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
		return;
	}
	/* Now reorder data to make the datablock EDID compatible
	 */
	for (i=0;i<header;i++) edid_data[i+128]=edid_data[i];
	for (i=0;i<128;i++) edid_data[i]=edid_data[i+header];
	for (i=0;i<128;i++) KRN_DEBUG(5,"%x : %x",i,edid_data[i]);

	/* Convert the raw edid data into monitor mode parameters */	
	EDID_Decode_Data (edid_data, &ddc_monitor_data);	
	/* Here the data is useful ! */
	print_monitor_data (&ddc_monitor_data); 
}


/**/
kgi_error_t DDC_monitor_init(DDC_monitor_t *DDC, 
	DDC_monitor_io_t *DDC_io, const kgim_options_t *options)
{
	KRN_NOTICE("VESA DDC monitor driver " KGIM_MONITOR_DRIVER);

	return KGI_EOK;
}
static kgi_s_t DDC_monitor_err(kgi_s_t pixels, kgi_s_t width, kgi_u_t dclk)
{
	kgi_s_t delta = (100*(pixels - width)) / width;
	kgi_s_t ret = (delta < -2) || (2 < delta);

	if (ret) {

		//KRN_DEBUG(0, "Timing difference %i > 2% (%i pixel %i ns %i dclk)",
		//	delta, pixels, ns, dclk);
	}

	return ret;
}

static inline kgi_error_t DDC_monitor_fcheck(DDC_monitor_t *DDC, 
	kgi_u_t dclk, kgi_u_t xtotal, kgi_u_t ytotal)
{
	kgi_u_t flags = 0, i;
	kgi_u_t hfreq = dclk / xtotal;
	kgi_u_t vfreq = dclk / (xtotal * ytotal);

	i = KGIM_MONITOR_MAX_HFREQ;
	while (i--) {

		if ((DDC->monitor.hfreq[i].min <= hfreq) &&
			(hfreq <= DDC->monitor.hfreq[i].max)) {

			flags |= 1;
		}
	}

	i = KGIM_MONITOR_MAX_VFREQ;
	while (i--) {

		if ((DDC->monitor.vfreq[i].min <= vfreq) &&
			(vfreq <= DDC->monitor.vfreq[i].max)) {

			flags |= 2;
		}
	}
	return (flags == 3) ? KGI_EOK : -KGI_ERRNO(MONITOR,INVAL);
}

kgi_error_t DDC_monitor_mode_mrl_check(DDC_monitor_t *DDC,
	DDC_monitor_io_t *DDC_io,
	DDC_monitor_mode_t *DDC_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images) {
	
	return -KGI_ERRNO(MONITOR, UNKNOWN);
}

kgi_error_t DDC_monitor_mode_tl_check(DDC_monitor_t *DDC,
	DDC_monitor_io_t *DDC_io,
	DDC_monitor_mode_t *DDC_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images) {

	return -KGI_ERRNO(MONITOR, UNKNOWN);
}

kgi_error_t DDC_monitor_mode_check(DDC_monitor_t *DDC,
	DDC_monitor_io_t *DDC_io,
	DDC_monitor_mode_t *DDC_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	const DDC_timing_t *t = DDC->timing;
	kgi_u_t v = 0, h = 0;
	kgi_u_t dclk, ddclk;
	kgi_u_t dotsx = DDC_mode->kgim.in.dots.x;
	kgi_u_t dotsy = DDC_mode->kgim.in.dots.y;

	if ((dotsx > DDC->monitor.maxdots.x) || 
		(dotsy > DDC->monitor.maxdots.y)) {

		KRN_DEBUG(2, "Resolution %ix%i higher than limit %ix%i",
			dotsx, dotsy, DDC->monitor.maxdots.x, 
			DDC->monitor.maxdots.y);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	while ((v < t->vtimings) && (t->vtiming[v].width != dotsy)) {

		v++;
	}
	if ((v == t->vtimings) || (t->vtiming[v].width != dotsy)) {

		KRN_DEBUG(2, "No vtiming for %i lines in DDC %s.",
			dotsy, t->name);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	h = t->vtiming[v].polarity & ~TIMELIST_POLARITY_MASK;
	dclk = t->htiming[h].dclk;
	ddclk = (DDC_mode->kgim.in.dclk > dclk)
		? (DDC_mode->kgim.in.dclk - dclk)
		: (dclk - DDC_mode->kgim.in.dclk);

	switch (cmd) {

	case KGI_TC_PROPOSE:
		DDC_mode->kgim.in.dclk	= dclk;
		DDC_mode->kgim.y		= t->vtiming[v];
		DDC_mode->kgim.y.polarity	= 
			(t->vtiming[v].polarity & TIMELIST_VPOS) ? 1 : 0;

#		define POS(X)  (t->htiming[h].tm.X)
		DDC_mode->kgim.x.width      = POS(width);
		DDC_mode->kgim.x.blankstart = POS(blankstart);
		DDC_mode->kgim.x.syncstart  = POS(syncstart);
		DDC_mode->kgim.x.syncend    = POS(syncend);
		DDC_mode->kgim.x.blankend   = POS(blankend);
		DDC_mode->kgim.x.total      = POS(total);
		DDC_mode->kgim.x.polarity   = 
			(t->vtiming[v].polarity & TIMELIST_HPOS) ? 1 : 0;
#		undef POS

		DDC_mode->kgim.sync = DDC->monitor.sync;
		  
		if (DDC->monitor.flags & KGIM_MF_PROPSIZE) {

			DDC_mode->kgim.size.x = dotsx *
				DDC->monitor.size.x /
				DDC->monitor.maxdots.x;
			DDC_mode->kgim.size.y = dotsy *
				DDC->monitor.size.y /
				DDC->monitor.maxdots.y;
		} else {

			DDC_mode->kgim.size = DDC->monitor.size;
		}

		KRN_DEBUG(2,
			"propose dclk = %i Hz, hfreq = %i Hz, vfreq = %i Hz",
			DDC_mode->kgim.in.dclk,
			DDC_mode->kgim.in.dclk /
				DDC_mode->kgim.x.total,
			DDC_mode->kgim.in.dclk /
				(DDC_mode->kgim.x.total * 
					DDC_mode->kgim.y.total));

		return KGI_TC_LOWER;

	case KGI_TC_LOWER:
		if (DDC_mode->kgim.in.dclk < DDC->monitor.dclk.min) {

			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_DEBUG(2, "dclk is %i (should be %li) Hz",
				 DDC_mode->kgim.in.dclk, dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_RAISE:
		if (DDC_mode->kgim.in.dclk > DDC->monitor.dclk.max) {

			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_DEBUG(2, "dclk is %i (should be %li) Hz",
				DDC_mode->kgim.in.dclk, dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_CHECK:
		if ((DDC_mode->kgim.in.dclk < DDC->monitor.dclk.min) || 
			(DDC_mode->kgim.in.dclk > DDC->monitor.dclk.max)) {

			KRN_DEBUG(2, "DCLK of %i Hz is out of bounds.",
				DDC_mode->kgim.in.dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((DDC->monitor.maxdots.x < dotsx) ||
			(DDC->monitor.maxdots.y < dotsy)) {

			KRN_DEBUG(2, "resolution too high (%ix%i).",
				dotsx, dotsy);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if (((100 * ddclk) > (2 * dclk)) || 
			DDC_monitor_fcheck(DDC,
				DDC_mode->kgim.in.dclk,
				DDC_mode->kgim.x.total,
				DDC_mode->kgim.y.total)) {

			KRN_DEBUG(2, "frequency limits violated.");
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		KRN_DEBUG(2, "%i\t%i %i %i %i %i %i\t%i %i %i %i %i %i",
			DDC_mode->kgim.in.dclk,
			DDC_mode->kgim.x.width,
			DDC_mode->kgim.x.blankstart,
			DDC_mode->kgim.x.syncstart,
			DDC_mode->kgim.x.syncend,
			DDC_mode->kgim.x.blankend,
			DDC_mode->kgim.x.total,
			DDC_mode->kgim.y.width,
			DDC_mode->kgim.y.blankstart,
			DDC_mode->kgim.y.syncstart,
			DDC_mode->kgim.y.syncend,
			DDC_mode->kgim.y.blankend,
			DDC_mode->kgim.y.total);

#		define ERR(X) ((DDC_mode->kgim.y.X != t->vtiming[v].X) || \
			DDC_monitor_err(DDC_mode->kgim.x.X,	\
			t->htiming[h].tm.X, DDC_mode->kgim.in.dclk))

			if (ERR(width) || ERR(blankstart) || ERR(syncstart) ||
				ERR(syncend) || ERR(blankend) || ERR(total)) {

				KRN_DEBUG(2, "Error in timings.");
				return -KGI_ERRNO(MONITOR, UNKNOWN);
			}

#		undef	ERR

		/*	now that everything is checked, initialize the
		**	driver dependent mode.
		*/
		DDC_mode->timing = t;

		return KGI_TC_READY;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
}

