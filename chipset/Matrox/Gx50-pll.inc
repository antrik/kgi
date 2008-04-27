/* ----------------------------------------------------------------------------
**	Gx50 PLL programming functions
** ----------------------------------------------------------------------------
**	Copyright (C)	2002	Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx50-pll.inc,v 1.1 2002/11/30 18:23:13 ortalo Exp $
**	
*/

/*
** This code is #include-d by both the chipset/ and clock/
** subsystems. (In the first one for initialization of the system
** clock on a secundary board, in the second for pixel
** clock programming.)
** The following hack allows to include two different
** declaration of the available functions, one relying
** on mga_chipset_io_t, the other on mga_clock_io_t.
** Both versions use MGA_EDAC_{IN,OUT}8 macros to perform
** the actual I/O.
*/
#if defined(_Gx50_PLL_FOR_CHIPSET)
typedef mgag_chipset_io_t Gx50_pll_io_t;
#else
typedef Gx50_clock_io_t Gx50_pll_io_t;
#endif

/*
** Gx50 PLL programming
*/
typedef struct
{
	kgi_u_t	f;		/* produced frequency	*/
	kgi_u_t	fvco;		/* VCO frequency	*/
	kgi_s_t	p;		/* post-VCO divider	*/
	kgi_u_t	mul;		/* multiplier value	*/
	kgi_u_t	div;		/* divider value	*/
	kgi_u_t	valid;		/* valid programming ?  */
} Gx50_pll_mode_t;

typedef struct
{
	kgi_u_t		fref;	/* reference frequency		*/
	kgi_urange_t	fvco;	/* VCO frequency limits		*/
	kgi_ratio_t	a;	/* generator dependent factor	*/
	kgi_srange_t	p;	/* post-VCO divider range	*/
	kgi_urange_t	mul;	/* multiplier value range	*/
	kgi_urange_t	div;	/* divider value range		*/

} Gx50_pll_t;

typedef enum
{
  Gx50_SYSTEM_PLL  = 0,
  Gx50_PIXEL_A_PLL = 1,
  Gx50_PIXEL_B_PLL = 2,
  Gx50_PIXEL_C_PLL = 3,
  Gx50_VIDEO_PLL   = 4
} Gx50_pll_type_t;

static kgi_u8_t Gx50_PLLM[] =
{ XSYSPLLM, XPIXPLLAM, XPIXPLLBM, XPIXPLLCM, XVIDPLLM };
static kgi_u8_t Gx50_PLLN[] =
{ XSYSPLLN, XPIXPLLAN, XPIXPLLBN, XPIXPLLCN, XVIDPLLN };
static kgi_u8_t Gx50_PLLP[] =
{ XSYSPLLP, XPIXPLLAP, XPIXPLLBP, XPIXPLLCP, XVIDPLLP };
/* NB: Only one "stat" register for A,B and C clocks */
static kgi_u8_t Gx50_PLLSTAT[] =
{ XSYSPLLSTAT, XPIXPLLSTAT, XPIXPLLSTAT, XPIXPLLSTAT, XVIDPLLSTAT };
static kgi_u8_t Gx50_PLLSTAT_LOCK[] =
{ XSYSPLLSTAT_SYSLOCK, XPIXPLLSTAT_PIXLOCK, XPIXPLLSTAT_PIXLOCK,
  XPIXPLLSTAT_PIXLOCK, XVIDPLLSTAT_VIDLOCK };

/*
 * TODO: - better error reporting
 */
static inline void Gx50_register_values(Gx50_pll_mode_t *mode,
					kgi_u8_t *m, kgi_u8_t *n, kgi_u8_t *ps)
{
  (*m) = mode->div - 1;
  (*n) = mode->mul - 2;
  (*ps) = (mode->p) ? (mode->p - 1) : 0x40;
  if (mode->fvco < 550 MHZ) {
#if 1
    (*ps) |= 0x00;
#else
    (*ps) |= 0x00;
#endif
  } else if (mode->fvco < 700 MHZ) {
#if 0
    (*ps) |= 0x08;
#else
    (*ps) |= 0x00;
#endif
  } else if (mode->fvco < 1000 MHZ) {
#if 0
    (*ps) |= 0x10;
#else
    (*ps) |= 0x08;
#endif
  } else if (mode->fvco < 1150 MHZ) {
    (*ps) |= 0x18;
  } else {
#if 0
    (*ps) |= 0x20;
#else
    (*ps) |= 0x10;
#endif
  }
#if 0
  KRN_DEBUG(2, "Using actual values (m,n,p)=(%.2x,%.2x,%.2x) fvco=%i Hz",
	 (*m), (*n), (*ps), mode->fvco);
#endif
}

/* Program a given clock mode
 */
static int Gx50_pll_mode_program(Gx50_pll_mode_t *mode,
				 Gx50_pll_io_t *pll_io,
				 Gx50_pll_type_t type)
{
  kgi_u8_t m,n,p;
  kgi_s_t j;

  KRN_DEBUG(2,"enter");
  Gx50_register_values(mode,&m,&n,&p);
  KRN_DEBUG(2, "Actual register values: (m,n,p)=(%.2x,%.2x,%.2x) fvco=%i Hz f=%i Hz",m,n,p, mode->fvco, mode->f);
  /* Program the clock */
  MGAG_EDAC_OUT8(pll_io, m, Gx50_PLLM[type]);
  MGAG_EDAC_OUT8(pll_io, n, Gx50_PLLN[type]);
  MGAG_EDAC_OUT8(pll_io, p, Gx50_PLLP[type]);
  /* Check stability */
  for (j = PLL_DELAY; j > 0 ; j--)
    {
      if (MGAG_EDAC_IN8(pll_io, Gx50_PLLSTAT[type])
	  & Gx50_PLLSTAT_LOCK[type]) {
	break;
      }
    }
  KRN_ASSERT(j != 0);
  KRN_DEBUG(2,"leave");
  return (j == 0);
}

/* Indicates if a target clock mode is stable for the Gx50
 * Update the validity flag
 */
static int Gx50_pll_mode_stable(Gx50_pll_mode_t *mode,
				Gx50_pll_io_t *pll_io,
				Gx50_pll_type_t type)
{
  kgi_u8_t m,n,p;
  kgi_s_t i, j;
  kgi_u_t still_valid = 1;
#define Gx50_PLL_ATTEMPTS 7
  kgi_s_t Gx50_attempts_noffset[] = { -3, 3, -2, 2, -1, 1, 0 };

  Gx50_register_values(mode,&m,&n,&p);
  KRN_DEBUG(2, "Actual register values: (m,n,p)=(%.2x,%.2x,%.2x)",m,n,p);
  for (i = 0; i < Gx50_PLL_ATTEMPTS; i++)
    {
      /* Do some offset */
      kgi_u8_t my_n = n + Gx50_attempts_noffset[i];
      /* Program the clock */
      MGAG_EDAC_OUT8(pll_io, m, Gx50_PLLM[type]);
      MGAG_EDAC_OUT8(pll_io, n, Gx50_PLLN[type]);
      MGAG_EDAC_OUT8(pll_io, p, Gx50_PLLP[type]);
      /* Check stability */
      for (j = PLL_DELAY; j > 0 ; j--)
	{
	  if (MGAG_EDAC_IN8(pll_io, Gx50_PLLSTAT[type])
	      & Gx50_PLLSTAT_LOCK[type]) {
	    kgi_u_t count = 0;
	    kgi_u_t k;
	    for (k = 0; k < 500; k++) {
	      if (MGAG_EDAC_IN8(pll_io, Gx50_PLLSTAT[type])
		  & Gx50_PLLSTAT_LOCK[type]) {
		count++;
	      }
	    }
	    still_valid = (count >= 460);
	    break;
	  }
	}
      if (j == 0)
	still_valid = 0;
      if (still_valid == 0) {
	KRN_DEBUG(2, "Clock unstable for delta n=%.2x (delta=%i)",
		  my_n, Gx50_attempts_noffset[i]);
	break;
      }
    }
  /* Store final result */
  mode->valid = still_valid;
  return 0;
}

/* Compute all possible mul,div,p values for the Gx50 PLL */
static int Gx50_pll_compute_all_modes(Gx50_pll_t *clock, kgi_u_t fout,
				      Gx50_pll_mode_t modes[],
				      kgi_u_t maxsize, kgi_u_t *size)
{
  kgi_u_t idx;
  kgi_u_t div;
  kgi_s_t p;
  kgi_u_t fvco, mul;
  kgi_u_t real_fvco;

  /* We do most computations in kHz (not Hz) to avoid possible
   * overflows
   */
  KRN_DEBUG(2, "enter");
  idx = 0;
  p = clock->p.max;
  while (1) {
    div = clock->div.max;
    while (1) {
      fvco = ((fout / (1 KHZ)) << p);
      mul = (fvco * div * clock->a.div + (clock->fref / (1 KHZ)))
      / ((clock->fref / (1 KHZ)) * clock->a.mul);
      if ((mul >= clock->mul.min) && (mul <= clock->mul.max))
	{
	  real_fvco = ((clock->fref / (1 KHZ)) * clock->a.mul * mul)
	    / (div * clock->a.div);
	  if ((real_fvco >= (clock->fvco.min / (1 KHZ)))
	      && (real_fvco <= (clock->fvco.max / (1 KHZ))))
	    {
	      if (idx > maxsize) {
		KRN_ERROR("Not enough space to store all computed modes");
		return 1;
	      }
	      modes[idx].p = p;
	      modes[idx].div = div;
	      modes[idx].mul = mul;
	      modes[idx].fvco = real_fvco * 1 KHZ;
	      modes[idx].f = (modes[idx].fvco >> p);
	      modes[idx].valid = 0;
	      KRN_DEBUG(2, "Found possible mode: f = %u, fvco = %u, "
			"(mul,div,pk)=(%u,%u,%u) (i=%u, err=%i)",
			modes[idx].f, modes[idx].fvco,
			modes[idx].mul, modes[idx].div, modes[idx].p,
			idx, (modes[idx].f - fout));
	      idx++;
	    }
	}
      if (div == clock->div.min)
	break;
      div--;
    }
    if (p == clock->p.min)
      break;
    p--;
  }
  *size = idx;
  if (idx == 0) {
    KRN_ERROR("No possible modes found for fout = %u", fout);
    return 1;
  }
  KRN_DEBUG(2, "leave");
  return 0;
}

/* Validates the stability of the given clock modes and
 * indicates the best stable mode (minimal freq error
 * among the stable modes).
 */
static int Gx50_pll_modes_validate(Gx50_pll_t *clock, kgi_u_t fout,
				   Gx50_pll_type_t type,
				   Gx50_pll_io_t *pll_io,
				   Gx50_pll_mode_t modes[], kgi_u_t size,
				   kgi_u_t *best_mode)
{
  kgi_u_t i;
  kgi_u_t idx, minerr, err;
  int first, found;

  for (i = 0; i < size; i++)
    { /* Check the validity of all possible modes */
      Gx50_pll_mode_stable(&(modes[i]), pll_io, type);
      if (modes[i].valid)
	KRN_DEBUG(2,"Clock mode %i is valid and stable",i);
    }
  found = 0;
  first = 1;
  idx = 0; minerr = 0; /* Silences warnings */
  for (i = 0; i < size; i ++)
    {
      if (modes[i].valid)
	{
	  found = 1;
	  err = (modes[i].f > fout) ? (modes[i].f - fout) : (fout - modes[i].f);
	  /* <= leads to lowest VCO, < leads to highest VCO
	   * Apparently, '<' is mandatory! -- ortalo (on my config)
	   */
	  if ((err < minerr) || (first))
	    {
	      first = 0;
	      minerr = err;
	      idx = i;
	    }
	}
    }
  if (!(found)) {
    KRN_ERROR("No stable modes found for f = %u", fout);
    return 1;
  }
  KRN_DEBUG(1, "Best clock mode found for f = %u with i=%u (err=%i)",
	    fout, idx, minerr);
  (*best_mode) = idx;
  return 0;
}

#define MAX_Gx50_PLL_MODES 50

/* Program a target frequency, return the calculated clock mode
 */
static int Gx50_pll_set(Gx50_pll_t *clock, kgi_u_t fout,
			Gx50_pll_type_t type,
			Gx50_pll_io_t *pll_io,
			Gx50_pll_mode_t *clock_mode)
{
  Gx50_pll_mode_t all_modes[MAX_Gx50_PLL_MODES+1];
  kgi_u_t no, best;
  int err;

  KRN_DEBUG(2, "enter");
  KRN_DEBUG(1, "Trying to set a valid mode for f = %u Hz", fout);
  err = Gx50_pll_compute_all_modes(clock, fout,
				     all_modes, MAX_Gx50_PLL_MODES,
				     &no);
  if (err) return err;
  err = Gx50_pll_modes_validate(clock, fout, type, pll_io,
				  all_modes, no, &best);
  if (err) return err;
  err = Gx50_pll_mode_program(&(all_modes[best]), pll_io, type);
  if (err) return err;
  (*clock_mode) = all_modes[best];
  KRN_ASSERT(clock_mode->valid);
  KRN_DEBUG(2, "leave");
  return 0;
}

/* Finds a target frequency, return the calculated clock mode
 */
static int Gx50_pll_best_mode(Gx50_pll_t *clock, kgi_u_t fout,
			      Gx50_pll_type_t type,
			      Gx50_pll_io_t *pll_io,
			      Gx50_pll_mode_t *clock_mode)
{
  Gx50_pll_mode_t all_modes[MAX_Gx50_PLL_MODES+1];
  kgi_u_t no, best;
  int err;

  KRN_DEBUG(2, "enter");
  KRN_DEBUG(1, "Trying to find a valid mode for f = %u Hz", fout);
  err = Gx50_pll_compute_all_modes(clock, fout,
				     all_modes, MAX_Gx50_PLL_MODES,
				     &no);
  if (err) return err;
  err = Gx50_pll_modes_validate(clock, fout, type, pll_io,
				  all_modes, no, &best);
  if (err) return err;
  (*clock_mode) = all_modes[best];
  KRN_ASSERT(clock_mode->valid);
  KRN_DEBUG(2, "leave");
  return 0;
}
