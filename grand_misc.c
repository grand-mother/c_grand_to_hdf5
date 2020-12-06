/** \file grand_misc.c
 *  \brief miscellaneous library routines I use for GRAND
 *
 *
 *  Date: 4/12/2020
 *
 *  Author: C. Timmermans
 */
#include "grand_misc.h"

#define RPOLE 6357000. /**<  Radius of the Earth at the poles */
#define REQ 6378000. /**<  Radius of the Earth at the equator */

/**
 * calculate the radius of the Earth at a specific lattitude assuming a flattened sphere
 * @param[in] latitude
 */
double rad_earth(float latitude)
{
    double phi = latitude/RADTODEG;
    double radius = pow(REQ*REQ*cos(phi),2)+pow(RPOLE*RPOLE*sin(phi),2);
    radius = radius/(pow(REQ*cos(phi),2)+pow(RPOLE*sin(phi),2));
    radius = sqrt(radius);
    return(radius);
}
