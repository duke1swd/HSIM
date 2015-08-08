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
 * Data to pass to the rocksim formatter.
 *
 * Units are SI units, always.
 */

struct rse_s {
	char 	*EngineMfg;
	char	EngineImpulseClass;
	double	AverageThrust;
	char	*EngineType;		// "hybrid"
	double	EngineDia;
	double	EngineLen;
	double	EngineWetMass;
	double	PropellantMass;
	double	PeakThrust;
	double	NozzleThroatDia;
	double	NozzleExitDia;
	double	TotalImpulse;
	double	BurnTimeSecs;
	double	MassFrac;		// fraction of weigh weight that goes away.
	double	ISPSecs;		// in seconds, not meters/sec.
	char	*comment;
};

struct rse_datapoint_s {
	double	Time;
	double	Thrust;
	double	Mass;			// propellant mass, not total mass
	double	CG;
};

void rse_begin(FILE *output);
void rse_datafile(struct rse_s *rp);
void rse_datafile_point(struct rse_datapoint_s *rdp);
void rse_end();
