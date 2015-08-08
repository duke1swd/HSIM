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
 * Rocksim RSE Format Output Routines
 *
 * The routines can be used to create a RSE format file as simulator output.
 */
#include <stdio.h>
#include <stdlib.h>
#include "rocksim.h"

static FILE *datafile;
static int engine_open;

void
rse_begin(FILE *output)
{
	datafile = output;

	fprintf(output, "<engine-database>\n"
			"  <engine-list>\n");
	engine_open = 0;
}

static void
close_engine()
{
	if (!engine_open)
		return;

	fprintf(datafile, "    </data>\n");
	fprintf(datafile, "  </engine>\n");
	engine_open = 0;
}

/*
 * Output the header.  Note that units are normally mm and grams.
 */

void
rse_datafile(struct rse_s *rp)
{
	close_engine();

	fprintf(datafile, "    <engine mfg=\"%s\"\n", rp->EngineMfg);
	fprintf(datafile, "      code=\"%c%.0f\"\n",
		rp->EngineImpulseClass,
		rp->AverageThrust);
	fprintf(datafile, "      Type=\"%s\"\n", rp->EngineType);
	fprintf(datafile, "      dia=\"%0.f\"\n", rp->EngineDia * 1000.);
	fprintf(datafile, "      len=\"%.0f\"\n", rp->EngineLen * 1000.);
	fprintf(datafile, "      initWt=\"%.1f\"\n",
		rp->EngineWetMass * 1000.);
	fprintf(datafile, "      propWt=\"%.1f\"\n",
		rp->PropellantMass * 1000.);
	fprintf(datafile, "      delays=\"0\"\n");		// plugged
	fprintf(datafile, "      auto-calc-mass=\"1\"\n");	// lazy for now
	fprintf(datafile, "      auto-calc-cg=\"1\"\n");		// lazy for now
	fprintf(datafile, "      avgThrust=\"%.2f\"\n", rp->AverageThrust);
	fprintf(datafile, "      peakThrust=\"%.2f\"\n", rp->PeakThrust);
	fprintf(datafile, "      throatDia=\"%.1f\"\n",
		rp->NozzleThroatDia * 1000.);
	fprintf(datafile, "      exitDia=\"%.1f\"\n",
		rp->NozzleExitDia * 1000.);
	fprintf(datafile, "      Itot=\"%.1f\"\n", rp->TotalImpulse);
	fprintf(datafile, "      burn-time=\"%.1f\"\n", rp->BurnTimeSecs);
	fprintf(datafile, "      massFrac=\"%.1f\"\n", rp->MassFrac * 100.);
	fprintf(datafile, "      Isp=\"%.1f\"\n", rp->ISPSecs / 9.807);
	// Not sure what to put here
	fprintf(datafile, "      tDiv=\"10\"\n");
	fprintf(datafile, "      tStep=\"-1.\"\n");
	fprintf(datafile, "      tFix=\"1\"\n");
	fprintf(datafile, "      FDiv=\"10\"\n");	// ??
	fprintf(datafile, "      FStep=\"-1.\"\n");
	fprintf(datafile, "      FFix=\"1\"\n");
	fprintf(datafile, "      mDiv=\"10\"\n");	// ??
	fprintf(datafile, "      mStep=\"-1.\"\n");
	fprintf(datafile, "      mFix=\"1\"\n");
	fprintf(datafile, "      cgDiv=\"10\"\n");	// ??
	fprintf(datafile, "      cgStep=\"-1.\"\n");
	fprintf(datafile, "      cgFix=\"1\"\n");
	fprintf(datafile, "    >\n");

	if (rp->comment) {
		fprintf(datafile, "    <comments>\n");
		fputs(rp->comment, datafile);
		fprintf(datafile, "    </comments>\n");
	}
	fprintf(datafile, "    <data>\n");
	engine_open = 1;
}

void
rse_datafile_point(struct rse_datapoint_s *rdp)
{
	extern char *myname;

	if (!engine_open) {
		fprintf(stderr, "%s: INTERNAL ERROR: miscall to rse\n",
			myname);
		exit(1);
	}

	fprintf(datafile, "      <eng-data");
	fprintf(datafile, "  t=\"%.3f\"", rdp->Time);
	fprintf(datafile, "  f=\"%.3f\"", rdp->Thrust);
	fprintf(datafile, "  m=\"%.1f\"", rdp->Mass * 1000.);
	fprintf(datafile, "  cg=\"%.1f\"", rdp->CG * 1000.);
	fprintf(datafile, "/>\n");
}

void
rse_end()
{
	close_engine();

	fprintf(datafile, "  </engine-list>\n"
		"</engine-database>\n");
}
