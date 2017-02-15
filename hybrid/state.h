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
 * This file defines the simulator's state.
 */

	/***********\
	*           *
	*   State   *
	*           *
	\***********/

/*
 * What type of simulation is this?
 */
extern int sim_type;
#define	UNKNOWN	0
#define	HYBRID	1
#define	LIQUID	2

/*
 * Simulating a dry firing (no ignition)?  Boolean.
 */
extern int dry_fire;

/*
 * Tank State
 */

/* Internal energy of the nitrous.  */
extern double	tank_energy;		/* in Joules	*/

/* Mass of N2O in the tank.  */
extern double	tank_n2o_mass;		/* in Kg	*/

/*
 * Derived Tank State
 *
 * These variables are derived from the primary
 * state variable (above)
 */
extern double	tank_pressure;		/* in pascals	*/
extern double	tank_temperature;	/* degrees K	*/
extern double	n2o_liquid_mass;	/* in Kg	*/
extern double	n2o_liquid_density;	/* in Kg/m**3	*/
extern double	vent_mass;		/* in Kg	*/

/*
 * Solid Fuel State
 */
extern double	fuel_mass;		/* in Kg	*/

/*
 * Derived Fuel State
 */
extern double	grain_core;		/* in meters	*/
extern double	fuel_rb;		/* in m/s ?	*/

/*
 * Liquid Fuel State
 */
extern double	lfuelmass;		/* in Kg	*/

/*
 * Derived Fuel State
 */
extern double	lfuelvolume;		/* in m**3	*/

/*
 * Nitrogen State
 */
extern double nitrogen_pressure;	/* in pascals	*/

/*
 * Chamber State
 */
extern double	chamber_pressure;	/* in pascals	*/
/* characteristic velocity */
extern double	c_star;			/* in m/s	*/
extern double	n2o_flow_rate;		/* in Kg/sec	*/
extern double	n2o_flux;		/* in Kg/sec/m2	*/
extern double	fuel_flow_rate;		/* in Kg/sec	*/

/*
 * Vent State
 */
extern double	n2o_vent_rate;		/* in Kg/sec	*/


	/***********\
	*           *
	*  Outputs  *
	*           *
	\***********/


/*
 * Current simulated time
 */
extern double	sim_time;		/* in seconds	*/

/* Specific impulse */
extern double	isp;			/* in m/s	*/

extern double	nozzle_cf;		/* unitless	*/
extern double	thrust;			/* in newtons	*/


extern double	exit_pressure;		/* in pascals	*/


	/************\
	*            *
	* Parameters *
	*            *
	\************/


/*
 * Tank Geometry.
 * Assumed to be a vertically oriented cylinder.
 */
extern double	tank_height;		/* in meters	*/
extern double	tank_volume;		/* in meters**3	*/

/*
 * Grain Parameters
 */
extern double	grain_length;		/* in meters	*/
extern double	grain_diameter;		/* in meters	*/
extern double	grain_init_core;	/* in meters	*/

/*
 * Nozzle Parameters
 */
extern double	nozzle_throat_area;	/* in meters**2	*/
extern double	nozzle_exit_area;	/* in meters**2	*/
extern double	nozzle_cf_correction;	/* unitless	*/
extern double	nozzle_half_angle;	/* in radians	*/

/*
 * Fuel Parameters
 */
extern char *	fuel;			/* PVC or PBAN	*/
extern double	fuel_density;		/* in kg/m**3	*/
extern double	combustion_efficiency;	/* Unitless	*/

/* fuel burn rate model parameters */
extern double	fuel_k;			/* ??? */
extern double	fuel_n;			/* ??? */
extern double	fuel_a;			/* ??? */

/*
 * Injector Parameters
 */
extern double	injector_area;		/* in meters**2	*/
extern double	liquid_injector_area;

/* discharge efficiency */
extern double	injector_cd;		/* unitless	*/
extern int	injector_count;

/*
 * Vent Parameters
 */
extern double	vent_area;		/* in meters**2	*/
extern double	vent_cd;		/* unitless	*/

/*
 * Global Parameters
 */
extern double	dry_mass;		/* in kg	*/
extern double	ambient_air_pressure;	/* in Pascal	*/

/*
 * Liquid Fuel Parameters
 */
extern double	lfuelinjectordia;
extern double	lfuelinjectorid;
extern double	lfuelinjectorod;
extern int	lfuelinjector_count;
extern double	lfuelinjectorcd;
extern double	lfueltankvolume;
extern double	lfueldensity;		/* in kg / m**3 */

/*
 * Nitrogen Parameters
 */
extern double	nitrogen_pressure_initial;
extern double	nitrogen_gamma;


/*
 * Simulator
 */
extern double	sim_time_step;		/* in seconds	*/
extern int	use_enthalpy;		/* Use N2O enthalpy, not energy */
extern int	ok_to_create_nzr;	/* flag		*/

#define	NZR_CREATE_NONE		0
#define	NZR_CREATE_SYSTEM	1
#define	NZR_CREATE_EXEC		2


	/***********\
	*           *
	* Constants *
	*           *
	\***********/

extern double atmosphere_pressure;	/* in pascals	*/
extern double pi;
extern double n2o_mols_per_kg;
extern double ideal_gas_constant;
extern double earth_gravity;		/* in m/sec/sec */


	/**********\
	*          *
	* Warnings *
	*          *
	\**********/

#define	WARN_INJECTOR_RATIO	0.3
extern int warn_injector_pressure;
extern double warn_injector_pressure_drop_value;
extern double warn_injector_pressure_drop_chamber_value;

/*
 * The flux rule of them is 0.8 lbm / in ** 2.
 * This is the metric version of that.
 */
#define	WARN_N2O_FLUX_LIMIT 562.455664	/* kg / meter **2 */
extern int warn_n2o_flux;
extern double warn_n2o_flux_value;

#define	WARN_CORE_THROAT_RATIO_1 2.
#define	WARN_CORE_THROAT_RATIO_2 3.
extern int warn_core_throat_ratio;
extern double warn_core_throat_ratio_value;

#define	WARN_EXIT_PRESSURE	(0.7 * atmosphere_pressure)
extern int warn_exit_pressure;

extern int n2o_thermo_error;	/* zero or one of the two below */
#define	TOO_HOT		(-2)
#define	TOO_COLD	(-1)

#define	WARN_SUPPLY_PRESSURE_DROP (6894.7573 * 100.)	/* 100 psi */
extern int warn_supply_pressure;
extern double warn_supply_pressure_drop_value;

extern int warn_negative_vent_to_fill;		/* supply tank too cold or dip tube too long */
