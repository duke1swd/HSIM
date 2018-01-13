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

void cpropep();
void chamber();
void liquid_init();
void fuel_init();
void fuel_regression();
int fuel_step(double delta_t);
int tank();
int tank_step(double delta_t);
void tank_boil_off(double filltemp);
void injector();
void liquid_injector();
int liquid_step();
void chamber_init();
void vent();
void sim_loop();
void sim_init();
void constants_init();
void record_data();
void record_data_init(double s, FILE *out);
void record_data_term();
double liquid_density(double temp);
double vapor_density(double temp);
double saturation_pressure(double temp);
double liquid_energy(double temperature);
double vapor_energy(double temperature);
double vapor_entropy(double temperature);
double cpcv(double temperature);
double sound_speed(double temperature);
double temp_from_pressure(double pressure);
double temp_from_vapor_energy(double vapor_energy);
void n2o_thermo_init();
void errors_init();
void print_errors(FILE *output);
void error_exit(int code);
void license(int c);
