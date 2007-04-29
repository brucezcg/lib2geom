#ifndef SEEN_PATH_H
#define SEEN_PATH_H

#include "point.h"
#include "rect.h"
#include "maybe.h"
#include <vector>

/* Need some kind of efficient iterator / position reference. */

/** Ideas:
 * path consists of handles and commands
 * reversing the handles and commands reverses the path
 * affine transforming the handles affine transforms the path
 * paths are immutable
 * all methods are O(n) or better, and bounds are given for each method and function.
 * library is as self contained as possible
*/

/** To be solved:
 * How do we show elements through the iterator?
 */

namespace Geom {

class CurveType;

typedef CurveType *PathOp;

class OldPath {
    bool closed; // should this shape be closed (join endpoints with a line)
    std::vector<Point> handles;
    std::vector<PathOp> cmd;

public:
    bool is_closed() const { return closed; }
    void set_closed(bool value) { closed = value; }
	
    std::vector<Point>const & get_handles() const { return handles; }
    std::vector<PathOp>const & get_cmd() const { return cmd; }
	
    class Elem {
    public:
        PathOp op;
        typedef std::vector<Point>::const_iterator const_iterator;
        const_iterator s, e;
        const_iterator begin() const {return s;}
        const_iterator end() const {return e;}
        Point first() const { return *s;}
        Point last() const { return e[-1];}
        size_t size() { return e - s; }

        Elem() {}
        Elem(PathOp op,
             const_iterator s, const_iterator e)
        : op(op), s(s), e(e) {}
        
        Point operator[]( const int i) {return s[i];}
        Point operator[]( const int i) const {return s[i];}
        
        void point_tangent_acc_at(double t, Point &p, Point &t, Point &a) const;
        Point point_at(double t) const;
        //Point tangent_at(double t);
        bool nearest_location(Point p, double& dist, double& t) const;
    };

    class ConstIter {
    public:
        std::vector<PathOp>::const_iterator c;
        std::vector<Point>::const_iterator h;
        
        ConstIter() {}
        ConstIter(std::vector<PathOp>::const_iterator c,
                  std::vector<Point>::const_iterator h)
        : c(c), h(h) {}
        void operator++();
        void operator--();
        Elem operator*() const;

        std::vector<Point>::const_iterator begin() const;
        std::vector<Point>::const_iterator end() const;
        PathOp cmd() { return *c; }
    };

    typedef ConstIter const_iterator;

    class Location {
    public:
        ConstIter it;
        double t; // element specific meaning [0,1)
        Location(ConstIter it, double t) : it(it), t(t) {}
        Location() {}
    };
    
    class HashCookie {
        unsigned long value;
    public:
        friend class OldPath;
        
        unsigned long get_value() { return value;}
    };

    OldPath() {}
    OldPath(OldPath const & sp)
    : cmd(sp.cmd), handles(sp.handles), closed(sp.closed) {}

    ConstIter begin() const { return ConstIter(cmd.begin(), handles.begin()+1);}
    ConstIter end() const { return ConstIter(cmd.end(), handles.end());}
    
    bool empty() const { return cmd.empty(); }
    unsigned size() const { return cmd.size(); }

    Point initial_point() const { return handles.front(); }
    Point final_point() const { return handles.back(); }

    /** returns the point at the position given by walking along the path. */
    Location location_at_arc_length(double s);

    Point point_at_arc_length(double s) {
        return point_at(location_at_arc_length(s));
    }

    /** return the last nearest point on the path. */
    Location nearest_location(Point p, double& dist) const;

    /** return a new path over [begin, end). */
    OldPath subpath(ConstIter begin, ConstIter end);
    
    /** return a new path over [begin, end). */
    OldPath subpath(Location begin, Location end);

    /** compute the bounding box of this path. */
    Maybe<Rect> bbox() const;

    OldPath insert_nodes(Location* b, Location* e);
    /** a new path with an extra node inserted at at without changing the curve. */
    OldPath insert_node(Location at);

    /** coords of point on path. */
    Point point_at(Location at) const;

    void point_tangent_acc_at (Location at, Point & pos, Point & tgt, Point &acc) const;
    
    void push_back(Elem e);
    void insert(ConstIter before, ConstIter s, ConstIter e);

// mainly for debugging
    ConstIter indexed_elem(int i) const {
        ConstIter it = begin();
        while(--i >= 0) ++it;
        return it;
    }
    
    operator HashCookie();
    
    unsigned total_segments() const;
    friend class OldPathSetBuilder;
    template <class T> friend OldPath operator*(OldPath const &p, T const &m);
};

inline bool operator==(OldPath::HashCookie a, OldPath::HashCookie b) {
    return a.get_value() == b.get_value();
}




class CurveType {
public:
    unsigned const n_handles;
    
    virtual Point point_at(Geom::OldPath::Elem const & elm, double t)=0;
    virtual MultidimSBasis<2> to_sbasis(Geom::OldPath::Elem const & elm)=0;
    CurveType(unsigned n_handles) : n_handles(n_handles) {}
};

class LineTo : public CurveType {
public:
    virtual Point point_at(Geom::OldPath::Elem const & elm, double t);
    virtual MultidimSBasis<2> to_sbasis(Geom::OldPath::Elem const & elm);
    LineTo() : CurveType(1) {}
};

class QuadTo : public CurveType {
public:
    virtual Point point_at(Geom::OldPath::Elem const & elm, double t);
    virtual MultidimSBasis<2> to_sbasis(Geom::OldPath::Elem const & elm);
    QuadTo() : CurveType(2) {}
};

class CubicTo : public CurveType {
public:
    virtual Point point_at(Geom::OldPath::Elem const & elm, double t);
    virtual MultidimSBasis<2> to_sbasis(Geom::OldPath::Elem const & elm);
    CubicTo() : CurveType(3) {}
};

extern LineTo * const lineto;
extern QuadTo * const quadto;
extern CubicTo * const cubicto;

inline void OldPath::ConstIter::operator++() {      h += (*c)->n_handles; c++; }
inline void OldPath::ConstIter::operator--() { c--; h -= (*c)->n_handles; }
inline OldPath::Elem OldPath::ConstIter::operator*() const {
    assert(*c >= 0); 
    return OldPath::Elem(*c, h - 1, h + (*c)->n_handles);
}

inline std::vector<Point>::const_iterator OldPath::ConstIter::begin() const {
    return h;
}
inline std::vector<Point>::const_iterator OldPath::ConstIter::end() const {
    return h + (*c)->n_handles;
}


class OldPathSetBuilder;

class OldPathSet {
public:
    typedef std::vector<OldPath>::const_iterator OldPathSetConstIter;
    typedef OldPathSetConstIter const_iterator;
    
    const_iterator begin() const { return _subpaths.begin(); }
    const_iterator end() const { return _subpaths.end(); }
    OldPath const &front() const { return _subpaths.front(); }
    OldPath const &back() const { return _subpaths.back(); }
    
    struct OldPathSetLocation {
        OldPath::ConstIter it;
        double t; // element specific meaning [0,1)
        OldPathSetLocation(OldPath::ConstIter it, double t) : it(it), t(t) {}
    };

    OldPathSet() {}
    OldPathSet(OldPath sp) { _subpaths.push_back(sp); }
    
    unsigned total_segments() const;

    template <typename F>
    OldPathSet map(F f) const {
        OldPathSet pr;
        for ( const_iterator it = begin() ; it != end() ; it++ ) {
            pr._subpaths.push_back(f(*it));
        }
        return pr;
    }

private:
    std::vector<OldPath> _subpaths;

    friend class OldPathSetBuilder;
    template <class T> friend OldPathSet operator*(OldPathSet const &p, T const &m);
};

inline bool operator!=(const OldPath::ConstIter &a, const OldPath::ConstIter &b) 
{ assert(a.c <= b.c);  assert(a.h <= b.h); return (a.c!=b.c) || (a.h != b.h);}

inline bool operator<(const OldPath::ConstIter &a, const OldPath::ConstIter &b) 
{ return (a.c < b.c) && (a.h < b.h);}

inline ptrdiff_t operator-(const OldPath::ConstIter &a, const OldPath::ConstIter &b) 
{ return a.c - b.c;}

//OldPath operator * (OldPath, Matrix);

template <class T> OldPath operator*(OldPath const &p, T const &m) {
    OldPath pr(p);
    for(int i = 0; i < pr.handles.size(); i++)
        pr.handles[i] = pr.handles[i] * m;
    return pr;
}

template <class T> OldPathSet operator*(OldPathSet const &p, T const &m) {
    struct multiply_by {
        T const &_m;
        multiply_by(T const &m) : _m(m) {}
        OldPath operator()(OldPath const &sp) { return sp * _m; }
    };
    return p.map(multiply_by(m));
}

template <typename Point, unsigned order>
struct BezImpl {
    static inline Point compute(double t, Point *b) {
        Point child[order];
        for ( unsigned i=0 ; i < order ; i++ ) {
            child[i] = BezImpl<Point, 1>(t, b + i);
        }
        return BezImpl<Point, order-1>::compute(t, child);
    }
};

template <typename Point>
struct BezImpl<Point, 1> {
    static inline Point compute(double t, Point *b) {
        return ( 1 - t ) * b[0] + t * b[1];
    }
};

template <typename Point>
struct BezImpl<Point, 0> {
    static inline Point compute(double t, Point *b) {
        return *b;
    }
};

template <unsigned order, typename Point>

inline Point bezier_at(double t, Point *b) {
    return BezImpl<Point, order>::compute(t, b);
}

}; // namespace Geom

#endif // SEEN_PATH_H

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

