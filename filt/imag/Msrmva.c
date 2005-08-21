/* 
 * 3-D S/R WEMVA with extended split-step
 * pcs 2005
 */

/*
  Copyright (C) 2004 University of Texas at Austin
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <rsf.h>
#include "srmva.h"
#include "img2.h"

int main (int argc, char *argv[])
{
    bool inv;             /* forward or adjoint */
    bool twoway;          /* two-way traveltime */
    bool verb;            /* verbosity */
    float eps;            /* dip filter constant */  
    int   nrmax;          /* number of reference velocities */
    float dtmax;          /* time error */
    int   pmx,pmy;        /* padding in the k domain */
    int   tmx,tmy;        /* boundary taper size */

    axa amx,amy,amz;
    axa alx,aly;
    axa aw,ae;

    /* I/O files */
    sf_file Bw_s=NULL,Bw_r=NULL; /* wavefield file W ( nx, ny,nw) */
    sf_file Bs=NULL;
    sf_file Ps=NULL;             /*  slowness file S (nlx,nly,nz) */
    sf_file Pi=NULL;             /*     image file R ( nx, ny,nz) */

    /* I/O slices */
    fslice Bwfls=NULL,Bwflr=NULL;
    fslice Bslow=NULL;
    fslice Pslow=NULL;
    fslice Pimag=NULL;

    /*------------------------------------------------------------*/
    sf_init(argc,argv);

    if (!sf_getbool(  "verb",&verb ))  verb =  true; /* verbosity flag */
    if (!sf_getfloat(  "eps",&eps  ))   eps =  0.01; /* stability parameter */
    if (!sf_getbool(   "inv",&inv  ))   inv = false; /* y=ADJ scat; n=FWD scat */
    if (!sf_getbool("twoway",&twoway))twoway=  true; /* two-way traveltime */
    if (!sf_getint(  "nrmax",&nrmax)) nrmax =     1; /* max number of refs */
    if (!sf_getfloat("dtmax",&dtmax)) dtmax = 0.004; /* max time error */
    if (!sf_getint(    "pmx",&pmx  ))   pmx =     0; /* padding on x */
    if (!sf_getint(    "pmy",&pmy  ))   pmy =     0; /* padding on y */
    if (!sf_getint(    "tmx",&tmx  ))   tmx =     0; /* taper on x   */
    if (!sf_getint(    "tmy",&tmy  ))   tmy =     0; /* taper on y   */


    /*------------------------------------------------------------*/
    /* SLOWNESS */

    Bs = sf_input("slo");
    iaxa(Bs,&alx,1); alx.l="lx";
    iaxa(Bs,&aly,2); aly.l="ly";
    iaxa(Bs,&amz,3); amz.l="mz";
    Bslow = fslice_init(alx.n*aly.n, amz.n, sizeof(float));
    fslice_load(Bs,Bslow,SF_FLOAT);

    /*------------------------------------------------------------*/    
    /* WAVEFIELD */

    Bw_s = sf_input("swf");
    Bw_r = sf_input("rwf");
    
    if (SF_COMPLEX != sf_gettype(Bw_s)) sf_error("Need complex   source data");
    if (SF_COMPLEX != sf_gettype(Bw_r)) sf_error("Need complex receiver data");

    iaxa(Bw_s,&amx,1); amx.l="mx";
    iaxa(Bw_s,&amy,2); amy.l="my";
    iaxa(Bw_s,&amz,3); amz.l="mz";
    iaxa(Bw_s,&aw,4);  aw.l = "w";
    iaxa(Bw_s,&ae,5);  ae.l = "e";

    Bwfls = fslice_init( amx.n*amy.n,amz.n*aw.n,sizeof(float complex));
    Bwflr = fslice_init( amx.n*amy.n,amz.n*aw.n,sizeof(float complex));

    fslice_load(Bw_s,Bwfls,SF_COMPLEX);
    fslice_load(Bw_r,Bwflr,SF_COMPLEX);

    /*------------------------------------------------------------*/

    if(inv) {
	Pi = sf_input ( "in");
	if (SF_COMPLEX !=sf_gettype(Pi)) sf_error("Need complex image perturbation");

	Ps = sf_output("out"); sf_settype(Ps,SF_COMPLEX);
	oaxa(Ps,&amx,1);
	oaxa(Ps,&amy,2);
	oaxa(Ps,&amz,3);

	Pslow = fslice_init(amx.n*amy.n,amz.n, sizeof(float complex));
	
	Pimag = fslice_init(amx.n*amy.n,amz.n, sizeof(float complex));
	fslice_load(Pi,Pimag,SF_COMPLEX);
    } else {
	
	Ps = sf_input("in");
	if (SF_COMPLEX !=sf_gettype(Ps)) sf_error("Need complex slowness perturbation");
	
	Pi = sf_output("out"); sf_settype(Pi,SF_COMPLEX);
	oaxa(Pi,&amx,1);
	oaxa(Pi,&amy,2);
	oaxa(Pi,&amz,3);
	
	Pslow = fslice_init(amx.n*amy.n,amz.n, sizeof(float complex));
	fslice_load(Ps,Pslow,SF_COMPLEX);
	
	Pimag = fslice_init(amx.n*amy.n,amz.n, sizeof(float complex));
    }

    /*------------------------------------------------------------*/

    srmva_init (verb,eps,dtmax,twoway,
		aw,
		amx,amy,amz,
		alx,aly,
		tmx,tmy,
		pmx,pmy,
		nrmax,Bslow,Bwfls,Bwflr);
    
/*    srmva_aloc();*/
    srmva(inv,Pslow,Pimag);
/*    srmva_free();*/

    srmva_close();

    /*------------------------------------------------------------*/
      
    if(inv) fslice_dump(Ps,Pslow,SF_COMPLEX);
    else    fslice_dump(Pi,Pimag,SF_COMPLEX);
    fslice_close(Pimag);
    fslice_close(Pslow);
    
    fslice_close(Bwfls);
    fslice_close(Bwflr);
    fslice_close(Bslow);

    exit (0);
}
