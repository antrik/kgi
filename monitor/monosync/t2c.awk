#!/usr/bin/awk -f
# -----------------------------------------------------------------------------
#	AWK script to convert monosync timing file into monosync timing C code
# -----------------------------------------------------------------------------
#	Copyright (C)	1999-2000	Steffen Seeger
#
#	This file is distributed under the terms and conditions of the 
#	MIT/X public license. Please see the file COPYRIGHT.MIT included
#	with this software for details of these terms and conditions.
#
# -----------------------------------------------------------------------------
#
#	command-line usage:	
#		t2c.awk timingset=<file-base-name> <file-name>
#
#	$Log: t2c.awk,v $
#	Revision 1.2  2002/07/10 17:08:25  aldot
#	- typo fix (avoid -potential- over..flaw)
#	  adjust includes
#	
#	Revision 1.1.1.1  2000/04/18 08:51:08  aldot
#	import of kgi-0.9 20020321
#	
#	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
#	- initial import of pre-SourceForge tree
#	
#
/htiming/ {

	xwidth[htimings]  = $2;
	xbstart[htimings] = $3;
	xsstart[htimings] = $4;
	xsend[htimings]   = $5;
	xbend[htimings]   = $6;
	xtotal[htimings]  = $7;
	htimings++;
	xwidth[htimings]  = 0;
	xbstart[htimings] = 0;
	xsstart[htimings] = 0;
	xsend[htimings]   = 0;
	xbend[htimings]   = 0;
	xtotal[htimings]  = 0;
}

/hsync/ {
	if ($2 == "+") { hsync[vtimings] = "HP"; }
	if ($2 == "-") { hsync[vtimings] = "HN"; }
}

/vsync/ {
	if ($2 == "+") { vsync[vtimings] = "VP"; }
	if ($2 == "-") { vsync[vtimings] = "VN"; }
}

/vtiming/ {

	ywidth[vtimings]  = $2;
	ybstart[vtimings] = $3;
	ysstart[vtimings] = $4;
	ysend[vtimings]   = $5;
	ybend[vtimings]   = $6;
	ytotal[vtimings]  = $7;
	htiming[vtimings] = htimings-1;

	vtimings++;

	ywidth[vtimings]  = 0;
	ybstart[vtimings] = 0;
	ysstart[vtimings] = 0;
	ysend[vtimings]   = 0;
	ybend[vtimings]   = 0;
	ytotal[vtimings]  = 0;
	hsync[vtimings]  = "HP";
	vsync[vtimings]  = "VP";
}

BEGIN {

	htimings = 0;
	vtimings = 0;
}

END {
	if (htimings && vtimings) {

		printf("#include <kgi/module.h>\n");
		printf("#include \"monitor/monosync/monosync-meta.h\"\n\n");

		printf("#define\tMONOSYNC_MONITOR_%s_HTIMINGS\t%i\n",
			timingset, htimings);
		printf("static const kgim_monitor_timing_t monosync_monitor_");
		printf("%s_htiming[MONOSYNC_MONITOR_%s_HTIMINGS] =\n{\n",
			timingset, timingset);
		for (i = 0; i < htimings; i++) {

			printf("/* %.3f */\t{ %6i, %6i, %6i, %6i, %6i, %6i, %i }",
				1e6/xtotal[i],
				xwidth[i], xbstart[i], xsstart[i],
				xsend[i], xbend[i], xtotal[i], i);
			printf((i < htimings-1) ? ",\n" : "\n");
		}
		printf("};\n");


		printf("\n#define\tMONOSYNC_MONITOR_%s_VTIMINGS\t%i\n",
			timingset, vtimings);
		printf("static const kgim_monitor_timing_t monosync_monitor_");
		printf("%s_vtiming[MONOSYNC_MONITOR_%s_VTIMINGS] =\n{\n",
			timingset, timingset);
		for (i = 0; i < vtimings; i++) {

			printf("/* %.1f */\t{ %5i, %5i, %5i, %5i, %5i, %5i, MONOSYNC_%s%s | %i }",
				1e9/xtotal[htiming[i]]/ytotal[i],
				ywidth[i], ybstart[i], ysstart[i],
				ysend[i], ybend[i], ytotal[i],
				hsync[i], vsync[i], htiming[i]);
			printf((i < vtimings-1) ? ",\n" : "\n");
		}
		printf("};\n");

		printf("\nconst monosync_timing_t monosync_monitor_timing =\n{\n",
			timingset);
		printf("\t\"%s\",\n", timingset);
		printf("\tMONOSYNC_MONITOR_%s_HTIMINGS,\n", timingset);
		printf("\tmonosync_monitor_%s_htiming,\n", timingset);
		printf("\tMONOSYNC_MONITOR_%s_VTIMINGS,\n", timingset);
		printf("\tmonosync_monitor_%s_vtiming\n};\n", timingset);
	}
}

