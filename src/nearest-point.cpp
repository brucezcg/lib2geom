/*
 * nearest point routines for D2<SBasis> and Piecewise<D2<SBasis>>
 *
 * Authors:
 * 		
 * 		Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2007-2008  authors
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


#include "nearest-point.h"

namespace Geom
{

////////////////////////////////////////////////////////////////////////////////
// D2<SBasis> versions

/*
 * Return the parameter t of a nearest point on the portion of the curve "c", 
 * related to the interval [from, to], to the point "p".
 * The needed curve derivative "dc" is passed as parameter.
 * The function return the first nearest point to "p" that is found.
 */

double nearest_point( Point const& p, 
					  D2<SBasis> const& c, 
					  D2<SBasis> const& dc, 
		              double from, double to )
{
	SBasis dd = dot(c - p, dc);	
	std::vector<double> zeros = Geom::roots(dd);
	
	double closest = from;
	double min_dist_sq = L2sq(c(from) - p);
	double distsq;
	for ( unsigned int i = 0; i < zeros.size(); ++i )
	{
		distsq = L2sq(c(zeros[i]) - p);
		if ( min_dist_sq > L2sq(c(zeros[i]) - p) )
		{
			closest = zeros[i];
			min_dist_sq = distsq;
		}
	}
	if ( min_dist_sq > L2sq( c(to) - p ) )
		closest = to;
	return closest;

}

/*
 * Return the parameters t of all the nearest points on the portion of 
 * the curve "c", related to the interval [from, to], to the point "p".
 * The needed curve derivative "dc" is passed as parameter.
 */

std::vector<double> 
all_nearest_points( Point const& p, 
		            D2<SBasis> const& c, 
		            D2<SBasis> const& dc, 
		            double from, double to )
{
	std::vector<double> result;
	SBasis dd = dot(c - p, Geom::derivative(c));
	
	std::vector<double> zeros = Geom::roots(dd);
	std::vector<double> candidates;
	candidates.push_back(from);
	candidates.insert(candidates.end(), zeros.begin(), zeros.end());
	candidates.push_back(to);
	std::vector<double> distsq;
	distsq.reserve(candidates.size());
	for ( unsigned int i = 0; i < candidates.size(); ++i )
	{
		distsq.push_back( L2sq(c(candidates[i]) - p) );
	}
	unsigned int closest = 0;
	double dsq = distsq[0];
	for ( unsigned int i = 1; i < candidates.size(); ++i )
	{
		if ( dsq > distsq[i] )
		{
			closest = i;
			dsq = distsq[i];
		}
	}
	for ( unsigned int i = 0; i < candidates.size(); ++i )
	{
		if( distsq[closest] == distsq[i] )
		{
			result.push_back(candidates[i]);
		}
	}
	return result;
}


////////////////////////////////////////////////////////////////////////////////
// Piecewise< D2<SBasis> > versions


double nearest_point( Point const& p,  
					  Piecewise< D2<SBasis> > const& c, 
					  Piecewise< D2<SBasis> > const& dc,
		              double from, double to )
{
	if ( c.size() != dc.size() )
	{
		throwRangeError("the passed piecewise curve and its derivative have "
				        "a mismatching number of pieces");
	}
	unsigned int si = c.segN(from);
	unsigned int ei = c.segN(to);
	if ( si == ei )
	{
		return nearest_point(p, c[si], dc[si], c.segT(from, si), c.segT(to, si));
	}
	double t;
	double nearest = nearest_point(p, c[si], dc[si], c.segT(from, si));
	for ( unsigned int i = si + 1; i < ei; ++i )
	{
		t = nearest_point(p, c[i], dc[i]);
		if ( nearest > t )
		{
			nearest = t;
		}
	}
	t = nearest_point(p, c[ei], dc[ei], 0, c.segT(to, ei));
	if ( nearest > t )
	{
		nearest = t;
	}
	return nearest;
}

std::vector<double> 
all_nearest_points( Point const& p, 
		            Piecewise< D2<SBasis> > const& c, 
		            Piecewise< D2<SBasis> > const& dc, 
		            double from, double to )
{
	if ( c.size() != dc.size() )
	{
		throwRangeError("the passed piecewise curve and its derivative have "
				        "a mismatching number of pieces");
	}
	unsigned int si = c.segN(from);
	unsigned int ei = c.segN(to);
	if ( si == ei )
	{
		return 
		all_nearest_points(p, c[si], dc[si], c.segT(from, si), c.segT(to, si));
	}
	std::vector<double> all_t;
	std::vector<double> all_nearest 
		= all_nearest_points(p, c[si], dc[si], c.segT(from, si));
	for ( unsigned int i = si + 1; i < ei; ++i )
	{
		all_t = all_nearest_points(p, c[i], dc[i]);
		if ( all_nearest.front() > all_t.front() )
		{
			all_nearest = all_t;
		}
		else if ( all_nearest.front() == all_t.front() )
		{
			all_nearest.insert(all_nearest.end(), all_t.begin(), all_t.end());
		}
	}
	all_t = all_nearest_points(p, c[ei], dc[ei], 0, c.segT(to, ei));
	if ( all_nearest.front() > all_t.front() )
	{
		all_nearest = all_t;
	}
	else if ( all_nearest.front() == all_t.front() )
	{
		all_nearest.insert(all_nearest.end(), all_t.begin(), all_t.end());
	}
	return all_nearest;
}

} // end namespace Geom


