/*
  This file is a portion of Hsim 0.4
 
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
 * Allocate storage for all the state variables.
 */
#include "state.h"

int	sim_type;
int	dry_fire;
double	tank_energy;
double	tank_n2o_mass;
double	tank_pressure;
double	tank_temperature;
double	n2o_liquid_mass;
double	n2o_liquid_density;
double	vent_mass;
char *	fuel;
double	fuel_mass;
double	grain_core;
double	fuel_rb;
double	chamber_pressure;
double	c_star;
double	n2o_flow_rate;
double	n2o_flux;
double	fuel_flow_rate;
double	n2o_vent_rate;
double	sim_time;
int	ok_to_create_nzr;
int	use_enthalpy;
double	isp;
double	nozzle_cf;
double	thrust;
double	exit_pressure;
double	tank_height;
double	tank_volume;
double	grain_length;
double	grain_diameter;
double	grain_init_core;
double	nozzle_throat_area;
double	nozzle_exit_area;
double	nozzle_cf_correction;
double	nozzle_half_angle;
double	fuel_density;
double	combustion_efficiency;
double	fuel_k;
double	fuel_n;
double	fuel_a;
double	injector_area;
double	injector_cd;
int	injector_count;
double	vent_area;
double	vent_cd;
double	dry_mass;
double	ambient_air_pressure;
double	lfuelinjectordia;
double	lfuelinjectorid;
double	lfuelinjectorod;
int	lfuelinjector_count;
double	lfuelinjectorcd;
double	lfueltankvolume;
double	lfuelmass;
double	lfuelvolume;
double	lfueldensity;
double	liquid_injector_area;
double	nitrogen_pressure;
double	nitrogen_gamma;
double	nitrogen_pressure_initial;
double	sim_time_step;
double	atmosphere_pressure;
double	pi;
double	n2o_mols_per_kg;
double	ideal_gas_constant;
double	earth_gravity;
int warn_injector_pressure;
double warn_injector_pressure_drop_value;
double warn_injector_pressure_drop_chamber_value;
int warn_n2o_flux;
double warn_n2o_flux_value;
int warn_core_throat_ratio;
double warn_core_throat_ratio_value;
int warn_exit_pressure;
int n2o_thermo_error;
int warn_supply_pressure;
double warn_supply_pressure_drop_value;
int warn_negative_vent_to_fill;
