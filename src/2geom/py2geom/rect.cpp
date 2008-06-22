/*
 * Copyright 2008 Aaron Spike <aaron@ekips.org>
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
 *
 */

#include <boost/python.hpp>
#include <boost/python/implicit.hpp>

#include "py2geom.h"
#include "helpers.h"

#include "../matrix.h"
#include "../d2.h"
#include "../interval.h"

using namespace boost::python;

void wrap_rect() {
    //TODO: fix overloads
    //def("unify", Geom::unify);
    def("union_list", Geom::union_list);
    //def("intersect", Geom::intersect);
    //def("distanceSq", Geom::distanceSq);
    //def("distance", Geom::distance);

    class_<Geom::Rect>("Rect", init<Geom::Interval, Geom::Interval>())
        .def("__getitem__", python_getitem<Geom::Rect,Geom::Interval,2>)
    
        .def("min", &Geom::Rect::min)
        .def("max", &Geom::Rect::max)
        .def("corner", &Geom::Rect::corner)
        .def("top", &Geom::Rect::top)
        .def("bottom", &Geom::Rect::bottom)
        .def("left", &Geom::Rect::left)
        .def("right", &Geom::Rect::right)
        .def("width", &Geom::Rect::width)
        .def("height", &Geom::Rect::height)
        .def("dimensions", &Geom::Rect::dimensions)
        .def("midpoint", &Geom::Rect::midpoint)
        .def("area", &Geom::Rect::area)
        .def("maxExtent", &Geom::Rect::maxExtent)
        .def("isEmpty", &Geom::Rect::isEmpty)
        .def("intersects", &Geom::Rect::intersects)
        // TODO: overloaded
        //.def("contains", &Geom::Rect::contains)
        .def("expandTo", &Geom::Rect::expandTo)
        .def("unionWith", &Geom::Rect::unionWith)
        // TODO: overloaded
        //.def("expanBy", &Geom::Rect::expandBy)

        .def(self * Geom::Matrix())
    ;

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
