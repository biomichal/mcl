/*   Copyright (C) 2004, 2005 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 2 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef util_duck_h
#define util_duck_h

#define RANDOM_MAX (2*((1<<30)-1)+1)

/*   seed -> dees -> geez -> geese -> goose -> duck
 *   formerly called brand: braindead rand(om number stuff).
*/



/*   This is for weak seeding, to obtain fresh seeds which will definitely
 *   *not* be suitable for cryptographic needs
*/

unsigned int mcxSeed
(  void
)  ;


#endif

