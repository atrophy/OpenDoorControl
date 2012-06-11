/**
 * @file
 * @brief Timer control and polling functions
 *
 * @author Daniel Harmsworth <atrophy@artifactory.org.au>
 * @author Brett Downing <brett@artifactory.org.au>
 * @version 2.0
 *
 * @section LICENSE
 *	
 * This file is part of OpenDoorControl.
 *
 * OpenDoorControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenDoorControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenDoorControl.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * The timers are the core of OpenDoorControl, in order to remove blocking code and
 * allow for asynchronous operation most of the functionality is tied to one or more
 * timers.
 *
**/


/**
 * Poll the timers and update or refresh as needed
 *
 * Checks the declared timers for expiries and runs their required expire function.
 * Dependant on the repeat flag may either stop or reset the timer.
 *
**/

void serviceTimers ()
{

}