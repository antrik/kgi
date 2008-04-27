#!/usr/bin/awk -f
# -----------------------------------------------------------------------------
#	AWK script to convert monosync timing file into monosync timing C code
# -----------------------------------------------------------------------------
#	Copyright (C)	1999-2000	Steffen Seeger
#	Copyright (C)	2002		Filip Spacek
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#
# -----------------------------------------------------------------------------
#
#	command-line usage:	
#		vga2c.awk timingset=<file-base-name> <file-name>
#
#	$Log: vga2c.awk,v $
#	Revision 1.2  2003/02/03 23:00:42  cegger
#	fix warning about missing dclk initializer. 0 might be a wrong value, but this is definitely better than having an undefined dclk value
#	
#	Revision 1.1  2002/12/09 18:20:29  fspacek
#	- initial version of the unified monitor driver
#	
#	
#
/htiming/ {

	xwidth[timings]  = $2;
	xbstart[timings] = $3;
	xsstart[timings] = $4;
	xsend[timings]   = $5;
	xbend[timings]   = $6;
	xtotal[timings]  = $7;
}

/hsync/ {
	if ($2 == "+") { hsync[timings] = "HPOS"; }
	if ($2 == "-") { hsync[timings] = "HNEG"; }
}

/vsync/ {
	if ($2 == "+") { vsync[timings] = "VPOS"; }
	if ($2 == "-") { vsync[timings] = "VNEG"; }
}

/vtiming/ {

	ywidth[timings]  = $2;
	ybstart[timings] = $3;
	ysstart[timings] = $4;
	ysend[timings]   = $5;
	ybend[timings]   = $6;
	ytotal[timings]  = $7;

	timings++;
	hsync[timings]  = "HP";
	vsync[timings]  = "VP";
}

BEGIN {

	timings = 0;
}

END {
#	if (timings) {

		printf("#include <kgi/module.h>\n");
		printf("#include \"monitor/crt/crt-meta.h\"\n\n");

		printf("static const crt_monitor_timing_t timings[] =\n{\n");
		for (i = 0; i < timings; i++) {

			printf("\t{\n");
			printf("\t\t\"\",\n");
			printf("/* %.3f */\t{ %6i, %6i, %6i, %6i, %6i, %6i, CRT_%s },\n",
				1e6/xtotal[i],
				xwidth[i], xbstart[i], xsstart[i],
				xsend[i], xbend[i], xtotal[i], hsync[i]);
			printf("/* %.1f */\t{ %6i, %6i, %6i, %6i, %6i, %6i, CRT_%s },\n",
				1e9/xtotal[i]/ytotal[i],
				ywidth[i], ybstart[i], ysstart[i],
				ysend[i], ybend[i], ytotal[i], vsync[i]);
			printf("/* dclk */\t0\n");
			printf((i < timings-1) ? "\t},\n" : "\t}\n");
		}
		printf("};\n\n");
		
		printf("crt_monitor_timing_set_t");
		printf(" crt_monitor_%s_timings =\n", timingset);
		printf("{\n");
		printf("\t\"%s\",\n", timingset);
		printf("\tsizeof(timings)/sizeof(crt_monitor_timing_t),\n");
		printf("\ttimings\n");
		printf("};\n");
#	}
}

