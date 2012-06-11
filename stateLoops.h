/**
 * @file
 * @brief Main loop prototypes for the FSM
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
 * FIXME Main loop prototypes for the various states.
 *
**/

void closedLoop();

void openLoop();

void lockupLoop(){}

void guestLoop(){}

void restrictLoop(){}