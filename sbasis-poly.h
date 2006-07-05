#ifndef _SBASIS_TO_POLY
#define _SBASIS_TO_POLY

#include "poly.h"
#include "s-basis.h"

/*** Conversion between SBasis and Poly.  Not recommended for general
 * use due to instability.
 */

SBasis poly_to_sbasis(Poly const & p);
Poly sbasis_to_poly(SBasis const & s);

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

#endif
