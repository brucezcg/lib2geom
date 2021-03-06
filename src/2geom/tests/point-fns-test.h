// point-fns-test.h
#include <cxxtest/TestSuite.h>

#include <cassert>
#include <cmath>
#include <glib/gmacros.h>
#include <stdlib.h>

#include "point-fns.h"
#include "isnan.h"

using Geom::Point;

class NrPointFnsTest : public CxxTest::TestSuite
{
public:
    NrPointFnsTest() :
        setupValid(true),
        p3n4( 3.0, -4.0 ),
        p0( 0.0, 0.0 ),
        small( pow( 2.0, -1070 ) ),
        inf( 1e400 ),
        nan( inf - inf ),
        small_left( -small, 0.0 ),
        small_n3_4( -3.0 * small, 4.0 * small ),
        part_nan( 3., nan ),
        inf_left( -inf, 5.0 )
    {
        TS_ASSERT( isNaN(nan) );
        TS_ASSERT( !isNaN(small) );

        setupValid &= isNaN(nan);
        setupValid &= !isNaN(small);
    }
    virtual ~NrPointFnsTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrPointFnsTest *createSuite() { return new NrPointFnsTest(); }
    static void destroySuite( NrPointFnsTest *suite ) { delete suite; }

// Called before each test in this suite
    void setUp()
    {
        TS_ASSERT( setupValid );
    }

    bool setupValid;
    Point const p3n4;
    Point const p0;
    double const small;
    double const inf;
    double const nan;

    Point const small_left;
    Point const small_n3_4;
    Point const part_nan;
    Point const inf_left;


    void testL1(void)
    {
        TS_ASSERT_EQUALS( Geom::L1(p0), 0.0  );
        TS_ASSERT_EQUALS( Geom::L1(p3n4), 7.0  );
        TS_ASSERT_EQUALS( Geom::L1(small_left), small  );
        TS_ASSERT_EQUALS( Geom::L1(inf_left), inf  );
        TS_ASSERT_EQUALS( Geom::L1(small_n3_4), 7.0 * small  );
        TS_ASSERT(isNaN(Geom::L1(part_nan)));
    }

    void testL2(void)
    {
        TS_ASSERT_EQUALS( Geom::L2(p0), 0.0 );
        TS_ASSERT_EQUALS( Geom::L2(p3n4), 5.0 );
        TS_ASSERT_EQUALS( Geom::L2(small_left), small );
        TS_ASSERT_EQUALS( Geom::L2(inf_left), inf );
        TS_ASSERT_EQUALS( Geom::L2(small_n3_4), 5.0 * small );
        TS_ASSERT( isNaN(Geom::L2(part_nan)) );
    }

    void testLInfty(void)
    {
        TS_ASSERT_EQUALS( Geom::LInfty(p0), 0.0 );
        TS_ASSERT_EQUALS( Geom::LInfty(p3n4), 4.0 );
        TS_ASSERT_EQUALS( Geom::LInfty(small_left), small );
        TS_ASSERT_EQUALS( Geom::LInfty(inf_left), inf );
        TS_ASSERT_EQUALS( Geom::LInfty(small_n3_4), 4.0 * small );
        TS_ASSERT( isNaN(Geom::LInfty(part_nan)) );
    }

    void testIsZero(void)
    {
        TS_ASSERT( Geom::is_zero(p0) );
        TS_ASSERT( !Geom::is_zero(p3n4) );
        TS_ASSERT( !Geom::is_zero(small_left) );
        TS_ASSERT( !Geom::is_zero(inf_left) );
        TS_ASSERT( !Geom::is_zero(small_n3_4) );
        TS_ASSERT( !Geom::is_zero(part_nan) );
    }

    void testAtan2(void)
    {
        TS_ASSERT_EQUALS( Geom::atan2(p3n4), atan2(-4.0, 3.0) );
        TS_ASSERT_EQUALS( Geom::atan2(small_left), atan2(0.0, -1.0) );
        TS_ASSERT_EQUALS( Geom::atan2(small_n3_4), atan2(4.0, -3.0) );
    }

    void testUnitVector(void)
    {
        TS_ASSERT_EQUALS( Geom::unit_vector(p3n4), Point(.6, -0.8) );
        TS_ASSERT_EQUALS( Geom::unit_vector(small_left), Point(-1.0, 0.0) );
        TS_ASSERT_EQUALS( Geom::unit_vector(small_n3_4), Point(-.6, 0.8) );
    }

    void testIsUnitVector(void)
    {
        TS_ASSERT( !Geom::is_unit_vector(p3n4) );
        TS_ASSERT( !Geom::is_unit_vector(small_left) );
        TS_ASSERT( !Geom::is_unit_vector(small_n3_4) );
        TS_ASSERT( !Geom::is_unit_vector(part_nan) );
        TS_ASSERT( !Geom::is_unit_vector(inf_left) );
        TS_ASSERT( !Geom::is_unit_vector(Point(.5, 0.5)) );
        TS_ASSERT( Geom::is_unit_vector(Point(.6, -0.8)) );
        TS_ASSERT( Geom::is_unit_vector(Point(-.6, 0.8)) );
        TS_ASSERT( Geom::is_unit_vector(Point(-1.0, 0.0)) );
        TS_ASSERT( Geom::is_unit_vector(Point(1.0, 0.0)) );
        TS_ASSERT( Geom::is_unit_vector(Point(0.0, -1.0)) );
        TS_ASSERT( Geom::is_unit_vector(Point(0.0, 1.0)) );
    }
};

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
