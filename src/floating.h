/*
 * floating.h
 *
 * Copyright 2006 MenTaLguY <mental@rydia.net>
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

#include <vector>
#include <algorithm>

#ifndef SEEN_GEOM_FLOATING_H
#define SEEN_GEOM_FLOATING_H

namespace Geom {

class FloatingObject;

class FloatingFrame {
public:
    FloatingFrame() {
        _parent = _top_frame;
        _top_frame = this;
    }
    ~FloatingFrame() {
        _top_frame = _parent;
        std::vector<FloatingObject *>::iterator iter;
        for ( iter = _floating.begin() ; iter != _floating.end() ; ++iter ) {
            delete *iter;
        }
    }

private:
    static FloatingFrame *_top_frame=NULL;
    FloatingFrame *_parent;
    std::vector<FloatingObject *> _floating;

    static void _float(FloatingObject *object) {
        _top_frame->_do_float(object, _top_frame);
    }
    void _do_float(FloatingObject *object) {
        _floating.push_back(object);
    }

    static void _unfloat(FloatingObject *object) {
        _top_frame->_do_unfloat(object);
    }
    void _do_unfloat(FloatingObject *object) {
        for ( frame = this ; frame ; frame = frame->_parent ) {
            if (frame->_do_unfloat_one(object)) {
                break;
            }
        }
    }
    bool _do_unfloat_one(FloatingObject *object) {
        std::vector<FloatingObject *>::iterator found;
        found = std::find(_floating.begin(), _floating.end(), object);
        if ( found != _floating.end() ) {
            _floating.erase(found);
            return true;
        } else {
            return false;
        }
    }

    static void _refloat(FloatingObject *object) {
        if (_top_frame->_do_unfloat_one(object)) {
            _top_frame->_parent->_do_float(object);
        }
    }

friend class FloatingObject;
};

class FloatingObject {
public:
    FloatingObject() {
        FloatingFrame::_float(this);
    }
    virtual ~FloatingObject() {
        /* it is an error if a floating object is manually
           deleted while still floating */
    }

    FloatingObject *unfloat() {
        FloatingFrame::_unfloat(this);
        return this;
    }

    FloatingObject *refloat() {
        FloatingFrame::_refloat(this);
        return this;
    }
};

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

