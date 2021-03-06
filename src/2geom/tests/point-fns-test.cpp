#include <cassert>
#include <cmath>
#include <glib/gmacros.h>
#include <stdlib.h>

#include "utest/utest.h"
#include "point-fns.h"
#include "isnan.h"

using Geom::Point;

int main(int argc, char *argv[])
{
    utest_start("point-fns");

    Point const p3n4(3.0, -4.0);
    Point const p0(0.0, 0.0);
    double const small = pow(2.0, -1070);
    double const inf = 1e400;
    double const nan = inf - inf;

    Point const small_left(-small, 0.0);
    Point const small_n3_4(-3.0 * small, 4.0 * small);
    Point const part_nan(3., nan);
    Point const inf_left(-inf, 5.0);

    assert(isNaN(nan));
    assert(!isNaN(small));

    UTEST_TEST("L1") {
        UTEST_ASSERT( Geom::L1(p0) == 0.0 );
        UTEST_ASSERT( Geom::L1(p3n4) == 7.0 );
        UTEST_ASSERT( Geom::L1(small_left) == small );
        UTEST_ASSERT( Geom::L1(inf_left) == inf );
        UTEST_ASSERT( Geom::L1(small_n3_4) == 7.0 * small );
        UTEST_ASSERT(isNaN(Geom::L1(part_nan)));
    }

    UTEST_TEST("L2") {
        UTEST_ASSERT( Geom::L2(p0) == 0.0 );
        UTEST_ASSERT( Geom::L2(p3n4) == 5.0 );
        UTEST_ASSERT( Geom::L2(small_left) == small );
        UTEST_ASSERT( Geom::L2(inf_left) == inf );
        UTEST_ASSERT( Geom::L2(small_n3_4) == 5.0 * small );
        UTEST_ASSERT(isNaN(Geom::L2(part_nan)));
    }

    UTEST_TEST("LInfty") {
        UTEST_ASSERT( Geom::LInfty(p0) == 0.0 );
        UTEST_ASSERT( Geom::LInfty(p3n4) == 4.0 );
        UTEST_ASSERT( Geom::LInfty(small_left) == small );
        UTEST_ASSERT( Geom::LInfty(inf_left) == inf );
        UTEST_ASSERT( Geom::LInfty(small_n3_4) == 4.0 * small );
        UTEST_ASSERT(isNaN(Geom::LInfty(part_nan)));
    }

    UTEST_TEST("is_zero") {
        UTEST_ASSERT(Geom::is_zero(p0));
        UTEST_ASSERT(!Geom::is_zero(p3n4));
        UTEST_ASSERT(!Geom::is_zero(small_left));
        UTEST_ASSERT(!Geom::is_zero(inf_left));
        UTEST_ASSERT(!Geom::is_zero(small_n3_4));
        UTEST_ASSERT(!Geom::is_zero(part_nan));
    }

    UTEST_TEST("atan2") {
        UTEST_ASSERT( Geom::atan2(p3n4) == atan2(-4.0, 3.0) );
        UTEST_ASSERT( Geom::atan2(small_left) == atan2(0.0, -1.0) );
        UTEST_ASSERT( Geom::atan2(small_n3_4) == atan2(4.0, -3.0) );
    }

    UTEST_TEST("unit_vector") {
        UTEST_ASSERT( Geom::unit_vector(p3n4) == Point(.6, -0.8) );
        UTEST_ASSERT( Geom::unit_vector(small_left) == Point(-1.0, 0.0) );
        UTEST_ASSERT( Geom::unit_vector(small_n3_4) == Point(-.6, 0.8) );
    }

    UTEST_TEST("is_unit_vector") {
        UTEST_ASSERT(!Geom::is_unit_vector(p3n4));
        UTEST_ASSERT(!Geom::is_unit_vector(small_left));
        UTEST_ASSERT(!Geom::is_unit_vector(small_n3_4));
        UTEST_ASSERT(!Geom::is_unit_vector(part_nan));
        UTEST_ASSERT(!Geom::is_unit_vector(inf_left));
        UTEST_ASSERT(!Geom::is_unit_vector(Point(.5, 0.5)));
        UTEST_ASSERT(Geom::is_unit_vector(Point(.6, -0.8)));
        UTEST_ASSERT(Geom::is_unit_vector(Point(-.6, 0.8)));
        UTEST_ASSERT(Geom::is_unit_vector(Point(-1.0, 0.0)));
        UTEST_ASSERT(Geom::is_unit_vector(Point(1.0, 0.0)));
        UTEST_ASSERT(Geom::is_unit_vector(Point(0.0, -1.0)));
        UTEST_ASSERT(Geom::is_unit_vector(Point(0.0, 1.0)));
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
