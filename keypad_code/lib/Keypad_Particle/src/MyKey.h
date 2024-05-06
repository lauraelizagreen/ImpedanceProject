/*
	Modified April 2017 by Michael Frontz
	Renamed class Key to class MyKey (and renamed files Key.cpp, Key.h to MyKey.cpp, MyKey.h)
	Prevents conflict with new Keyboard functions included in Particle firmware
	https://docs.particle.io/reference/firmware/photon/#keyboard
	
	Modifications based on tip from Particle Community forum post
	https://community.particle.io/t/keypad-library-works-with-photon-but-not-electron/28209/4
	
||
|| @file MyKey.h
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | MyKey class provides an abstract definition of a key or button
|| | and was initially designed to be used in conjunction with a
|| | state-machine.
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/

#ifndef MYKEY_H
#define MYKEY_H

// This will load the definition for common Particle variable types
#include "Particle.h"

#define OPEN LOW
#define CLOSED HIGH

typedef unsigned int uint;
typedef enum{ IDLE, PRESSED, HOLD, RELEASED } KeyState;

const char NO_KEY = '\0';

class MyKey {
public:
	// members
	char kchar;
	int kcode;
	KeyState kstate;
	boolean stateChanged;

	// methods
	MyKey();
	MyKey(char userKeyChar);
	void key_update(char userKeyChar, KeyState userState, boolean userStatus);

private:

};

#endif

/*
|| @changelog
|| | 1.0 2012-06-04 - Mark Stanley : Initial Release
|| #
*/
