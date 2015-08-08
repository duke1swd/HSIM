/*
  This file is a portion of Hsim 0.2
 
  Hsim is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

/*
 * cpropep linakge
 */
struct cpp_s {
	/* INPUTS */
	double	OF;	// O:F ratio
	double	Cp;	// Chamber pressure in PSI
	double	Nzr;	// Nozzle Expansion Ratio
	/* OUTPUTS */
	double	Isp;
	double	Cs;	// Cstar in fps
	double	Cf;
	double	Ep;	// Exit pressure in ATM
};

#ifdef DATACREATE

#define	N_OF	25
#define N_CP	17

/*
 * These #defines link the various pieces together.
 */

/*
 * This directory is the cache of cpropep data.
 */
#define	CPROPEPDATA	"./cpropepdata"

#define	CREATENZR	"./createNzr"	/* Program to create data matrix */
#define	RUNCMD		"./runcp"	/* shell script that runs cpropep */

/*
 * This directoy is where the actual cpropep program lives. 
 * createNzr creates a temporary cpropep input file in this directorty too.
 */
#define	CPRODIR		"./cpropep"

/* The cpropep input file created by createNzr */
#define	CPROTEMP "cpro.input"

double OFvector[N_OF] = {
	1.,  1.5, 2.,  2.5, 3.,
	3.5, 4.,  4.5, 5.,  5.5,
	6.,  6.5, 7.,  7.5, 8.,
	8.5, 9.,  9.5, 10., 10.5,
	11., 12., 15., 20., 100.,
};

double CPvector[N_CP] = {
	30, 50, 75, 100, 125, 150, 175, 200, 250, 300, 350, 400, 450, 500, 600, 700, 800,
};

static struct cpp_s data[N_OF][N_CP];

#endif /* DATACREATE */
