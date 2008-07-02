/*
 * make up an elliptical arc knowing 5 points lying on the arc  
 *
 * Authors:
 * 		Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2008  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/path.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/numeric/linear_system.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>


namespace Geom
{

struct ellipse_equation
{
	ellipse_equation(double a, double b, double c, double d, double e, double f)
		: A(a), B(b), C(c), D(d), E(e), F(f)
	{
	}
	
	double operator()(double x, double y) const
	{
		return A * x * x + B * x * y + C * y * y + D * x + E * y + F;
	}
	
	double operator()(Point const& p) const
	{
		return (*this)(p[X], p[Y]);
	}
	
	Point normal(double x, double y) const
	{
		Point n( 2 * A * x + B * y + D, 2 * C * y + B * x + E );
		return unit_vector(n);
	}
	
	Point normal(Point const& p) const
	{
		return normal(p[X], p[Y]);
	}
	
	double A, B, C, D, E, F;
};


class elliptiarc_maker
{
  public:
	elliptiarc_maker(   EllipticalArc& _ea,
					    Point const& _initial_point, 
					    Point const&_final_point,
					    Point const& inner_point1, 
					    Point const& inner_point2, 
					    Point const& inner_point3,
					    double _tolerance )
		: ea(_ea),
		  tolerance(_tolerance), 
		  tol_at_extr(tolerance/2), 
		  tol_at_center(0.1),
		  m(N, N), col(N),
		  initial_point(_initial_point), 
		  final_point(_final_point)
	{
		p[0] = initial_point;
		p[1] = inner_point1;
		p[2] = inner_point2;
		p[3] = inner_point3;
		p[4] = final_point;
	}
	
  private:
	void init_x_y(unsigned int k)
	{
		x1 = p[k][X]; x2 = x1 * x1;  x3 = x2 * x1;
		y1 = p[k][Y]; y2 = y1 * y1;  y3 = y2 * y1; y4 = y3 * y1;
		x1y1 = x1 * y1;
		x2y1 = x2 * y1; x1y2 = x1 * y2; x2y2 = x2*y2;
		x3y1 = x3 * y1; x1y3 = x1 * y3;
	}

	void init_linear_system()
	{
		m(0,0) += y4;
		m(0,1) += x1y3;
		m(0,2) += x1y2;
		m(0,3) += y3;
		m(0,4) += y2;

		m(1,0) += x1y3;
		m(1,1) += x2y2;
		m(1,2) += x2y1;
		m(1,3) += x1y2;
		m(1,4) += x1y1;

		m(2,0) += x1y2;
		m(2,1) += x2y1;
		m(2,2) += x2;
		m(2,3) += x1y1;
		m(2,4) += x1;

		m(3,0) += y3;
		m(3,1) += x1y2;
		m(3,2) += x1y1;
		m(3,3) += y2;
		m(3,4) += y1;

		m(4,0) += y2;
		m(4,1) += x1y1;
		m(4,2) += x1;
		m(4,3) += y1;
		m(4,4) += 1;

		col[0] -= x2y2;
		col[1] -= x3y1;
		col[2] -= x3;
		col[3] -= x2y1;
		col[4] -= x2;
	}

	void evaluate_coefficients()
	{
		// calculating best coefficients with the least squares method
		m.set_all(0);
		col.set_all(0);
		for ( unsigned int k = 0; k < N; ++k )
		{
			init_x_y(k);
			init_linear_system();
		}

		NL::LinearSystem ls(m, col);
		ls.SV_solve();
		
		A = 1;					// x^2 coeff
		B = ls.solution()[1];	// xy  coeff
		C = ls.solution()[0];	// y^2 coeff
		D = ls.solution()[2];	// x   coeff
		E = ls.solution()[3];	// y   coeff
		F = ls.solution()[4];	// constant coeff
	}

	bool bound_exceeded( unsigned int k, ellipse_equation const & ee, 
			             double e1x, double e1y, double e2 )
	{
		dist_err = std::fabs( ee(p[k]) );
		dist_bound = std::fabs( e1x * p[k][X] + e1y * p[k][Y] + e2 );
		return ( dist_err  > dist_bound  );
	}
	
	bool check_bound()
	{		
		// check error magnitude
		ellipse_equation ee(A, B, C, D, E, F);
		
		double e1x = (2*A + B) * tol_at_extr;
		double e1y = (B + 2*C) * tol_at_extr;
		double e2 = ((D + E)  + (A + B + C) * tol_at_extr) * tol_at_extr;
		if ( bound_exceeded(0, ee, e1x, e1y, e2) )
		{
			print_bound_error(0);
			return false;
		}
		if ( bound_exceeded(0, ee, e1x, e1y, e2) )
		{
			print_bound_error(last);
			return false;
		}

		e1x = (2*A + B) * tolerance;
		e1y = (B + 2*C) * tolerance;
		e2 = ((D + E)  + (A + B + C) * tolerance) * tolerance;
	//	std::cerr << "e1x = " << e1x << std::endl;
	//	std::cerr << "e1y = " << e1y << std::endl;
	//	std::cerr << "e2 = " << e2 << std::endl;

		for ( unsigned int k = 1; k < last; ++k )
		{
			if ( bound_exceeded(k, ee, e1x, e1y, e2) )
			{
				print_bound_error(k);
				return false;
			}
		}

		return true;
	}

	bool evaluate_properties()
	{
		// evaluate ellipse centre
		Point centre;
		double den = 4*C - B*B;
		if ( den == 0 )
		{
			THROW_LOGICALERROR("den == 0, while computing ellipse centre");
		}
		centre[Y] = (B*D - 2*E) / den;
		centre[X] = (B*E - 2*C*D) / den;
		
		// evaluate the a coefficient of the ellipse equation in normal form
		// E(x,y) = a*(x-cx)^2 + b*(x-cx)*(y-cy) + c*(y-cy)^2 = 1
		// where b = a*B , c = a*C, (cx,cy) == centre
		den = sqr(centre[X]) 
				+ B * centre[X] * centre[Y] 
				+ C * sqr(centre[Y]) - F;
		if ( den == 0 )
		{
			THROW_LOGICALERROR("den == 0, while computing 'a' coefficient");
		}
		double a = 1 / den;

//		std::cerr << "a = " << a << std::endl;
//		std::cerr << "B = " << B << std::endl;
//		std::cerr << "C = " << C << std::endl;
//		std::cerr << "D = " << D << std::endl;
//		std::cerr << "E = " << E << std::endl;
//		std::cerr << "F = " << F << std::endl;

		//evaluate ellipse rotation angle
		double rot = std::atan2( -B, -(A - C) )/2;
//		std::cerr << "rot = " << rot << std::endl;
		bool swap_axes = false;
		if ( are_near(rot, 0) ) rot = 0;
		if ( are_near(rot, M_PI/2)  || rot < 0 )
		{
			swap_axes = true;
		}

		// evaluate the length of the ellipse rays	
		double cosrot = std::cos(rot);
		double sinrot = std::sin(rot);
		double cos2 = cosrot * cosrot;
		double sin2 = sinrot * sinrot;
		double cossin = cosrot * sinrot;

		den = a * (cos2 + B * cossin + C * sin2);
		if ( den <= 0 )
		{
			print_rays_evaluation_error(X);
			return false;
		}
		double rx = std::sqrt( 1/den );

		den = a * ( C * cos2 - B * cossin + sin2 );
		if ( den <= 0 )
		{
			print_rays_evaluation_error(Y);
			return false;
		}
		double ry = std::sqrt(1/den);

		// the solution is not unique so we choose always the ellipse 
		// with a rotation angle between 0 and PI/2
		if ( swap_axes ) std::swap(rx, ry);
		if (    are_near(rot,  M_PI/2) 
			 || are_near(rot, -M_PI/2) 
			 || are_near(rx, ry)       )
		{
			rot = 0;
		}
		else if ( rot < 0 )
		{
			rot += M_PI/2;
		}

//		std::cerr << "swap axes: " << swap_axes << std::endl;
//		std::cerr << "rx = " << rx << " ry = " << ry << std::endl;
//		std::cerr << "rot = " << rad_to_deg(rot) << std::endl;
//		std::cerr << "centre: " << centre << std::endl;

		// find out how we should set the large_arc_flag and sweep_flag
		bool large_arc_flag = true;
		bool sweep_flag = true;

		Point inner_point = p[1];

		Point sp_cp = initial_point - centre;
		Point ep_cp = final_point - centre;
		Point ip_cp = inner_point - centre;
		
		double angle1 = angle_between(sp_cp, ep_cp);
		double angle2 = angle_between(sp_cp, ip_cp);
		double angle3 = angle_between(ip_cp, ep_cp);

		if ( angle1 > 0 )
		{
			if ( angle2 > 0 && angle3 > 0 )
			{
				large_arc_flag = false;
				sweep_flag = true;
			}
			else
			{
				large_arc_flag = true;
				sweep_flag = false;
			}
		}
		else
		{
			if ( angle2 < 0 && angle3 < 0 )
			{
				large_arc_flag = false;
				sweep_flag = false;
			}
			else
			{
				large_arc_flag = true;
				sweep_flag = true;
			}
		}

		// finally we're going to create the elliptical arc!
		try
		{
			ea.set( initial_point, rx, ry, rot, 
					large_arc_flag, sweep_flag, final_point );
		}
        catch( RangeError e )
        {
        	std::cerr << e.what() << std::endl;
        	return false;
        }

		if ( !are_near( centre, ea.center(), tol_at_center * std::min(rx,ry) ) )
		{
			print_center_error(centre, ea.center());
			return false;
		}
		return true;
	}

	void print_bound_error(unsigned int k)
	{
		std::cerr
			<< "tolerance error" << std::endl
			<< "at point: " << k << std::endl
			<< "error value: "<< dist_err << std::endl
		    << "bound: " << dist_bound << std::endl;
	}

	void print_rays_evaluation_error(unsigned int dim)
	{
		std::cerr << "!(den > 0) error" << std::endl;
		if ( dim == X )
		{
			std::cerr << "evaluating rx" << std::endl;
		}
		else if ( dim == Y )
		{
			std::cerr << "evaluating ry" << std::endl;
		}
	}

	void print_center_error( Point const& c1, Point const& c2 )
	{
		std::cerr
			<< "center mismatch error: " << std::endl
			<< "evaluated center: " << c1 <<  std::endl
			<< "elliptical arc center: " << c2 <<  std::endl
			<< "distance: " << distance(c1, c2) << std::endl;
	}

  public:
	bool operator()()
	{
		evaluate_coefficients();
		if ( !check_bound() ) return false;
		if ( !evaluate_properties() ) return false;
		return true;
	}
	
	EllipticalArc& result()
		{ return ea; }


	double get_tolerance() const
		{ return tolerance; }
	void set_tolerance(double _tolerance)
		{ tolerance = _tolerance; }
	double get_tolerance_at_extremes() const
		{ return tol_at_extr; }
	void set_tolerance_at_extremes(double _tol_at_extr)
		{ tol_at_extr = _tol_at_extr; }
	double get_tolerance_at_center() const
		{ return tol_at_center; }
	void set_tolerance_at_center(double _tol_at_center)
		{ tol_at_center = _tol_at_center; }
	
	double get_error()
		{ return dist_err; }
	double get_bound()
		{ return dist_bound; }

private:
	//cairo_t* cr;
	EllipticalArc& ea;
	double tolerance, tol_at_extr, tol_at_center;
	NL::Matrix m;
	NL::Vector col;
	Point initial_point, final_point;
	static const unsigned int N = 5;
	static const unsigned int last = N - 1;
	Point p[N];
	double A, B, C, D, E, F;
	double dist_err, dist_bound;
	double x1, x2, x3, y1, y2, y3, y4, x1y1, x2y1, x1y2, x2y2, x3y1, x1y3;
};

}


using namespace Geom;


class ElliptiArcMaker : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
  	      	   int width, int height, bool save ) 
    {
    	cairo_set_line_width (cr, 0.2);
    	cairo_set_source_rgba(cr, 0.0, 0.0, 0.7, 1.0);
    	draw_text(cr, psh.pts[0], "initial");
    	draw_text(cr, psh.pts[1], "inner");
    	draw_text(cr, psh.pts[4], "final");
    	cairo_stroke(cr);
    	cairo_set_line_width (cr, 0.4);
    	cairo_set_source_rgba(cr, 0.0, 0.0, 0.7, 1.0);
    	try
    	{
    		EllipticalArc EA;
    		elliptiarc_maker make( EA, 
    		                       psh.pts[0], psh.pts[4],
    		                       psh.pts[1], psh.pts[2], psh.pts[3], 
    							   tolerance );
    		if ( !make() ) 
    		{
    			*notify << "distance error: " << make.get_error() 
    			        << " ( " << make.get_bound() << " )" << std::endl;
    			Toy::draw(cr, notify, width, height, save);
    			return;
    		}
    		D2<SBasis> easb = EA.toSBasis();
        	cairo_md_sb(cr, easb);
        	cairo_stroke(cr);
    	}
        catch( RangeError e )
        {
        	std::cerr << e.what() << std::endl;
        	Toy::draw(cr, notify, width, height, save);
        	return;
        } 	

    	Toy::draw(cr, notify, width, height, save);
    }

  public:
	  ElliptiArcMaker( double _tolerance )
		: tolerance(_tolerance)
	{
		total_handles = 5;
		for ( unsigned int i = 0; i < total_handles; ++i )
		{
			psh.push_back(uniform()*400, uniform()*400);
		}
		handles.push_back(&psh);
	}
	
	unsigned int total_handles;
	double tolerance;
	PointSetHandle psh;
};



int main(int argc, char **argv) 
{	
	double tolerance = 8;
	if(argc > 1)
	        sscanf(argv[1], "%lf", &tolerance);
    init( argc, argv, new ElliptiArcMaker(tolerance) );
    return 0;
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
