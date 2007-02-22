#include "s-basis.h"
#include "pw-sb.h"

#include "path-cairo.h"
#include "toy-framework.cpp"
#include "bezier-to-sbasis.h"
#include <iterator>
#include <map>

using namespace Geom;

static void cairo_pw(cairo_t *cr, pw_sb p, double time_scale=1) {
    for(int i = 0; i < p.size(); i++) {
        MultidimSBasis<2> B;
        B[0] = BezOrd(p.cuts[i]*time_scale, p.cuts[i+1]*time_scale);
        B[1] = p[i];
        cairo_md_sb(cr, B);
    }
}

class PwToy: public Toy {
    unsigned segs, handles_per_seg, handles_per_curve, curves;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
       
        md_pw_sb pws<2>;
        int hdle_idx=0;
        for(int i = 0; i < segs; i++) {
            MultidimSBasis<2> foo = Geom::bezier_to_sbasis<2,3>(handles.begin()+hdle_idx);
            hdle_idx += 4;
            cairo_md_sb(cr, foo);
            for(int d = 0; d < 2; d++) {
                pws[d].cuts.push_back(150*i);
                pws[d].segs.push_back(foo[d]);
            }
        }
        for(int d = 0; d < 2; d++)
            pws[d].cuts.push_back(150*segs);
        
        handles[hdle_idx  ][1]=450;
        handles[hdle_idx+1][1]=450;
        handles[hdle_idx+2][1]=450;
        handles[hdle_idx+3][1]=450;
	
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        MultidimSBasis<2> foo = Geom::bezier_to_sbasis<2,3>(handles.begin()+hdle_idx);
        SBasis g = foo[0] - BezOrd(150);
        cairo_md_sb(cr, foo);
	    for(int i=0;i<20;i++){
            double t=i/20.;
            draw_handle(cr, point_at(foo,t));
        }
        cairo_stroke(cr);

        for(int a = 0; a < curves; a++) {
            cairo_pw2(cr, pws[2*a],pws[2*a+1]);
        }

        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
        pw_sb x= compose(pws[0],g);
        pw_sb y= compose(pws[1],g);
        cairo_md_pw_sb(cr, composeEach(pws, g));
        for(int i=0;i<20;i++){
            double t=(x.cuts.back()-x.cuts.front())*i/20.;
            draw_handle(cr, Point(x(t),y(t)));
        }
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
        
    public:
    PwToy () {
        segs = 2;
        handles_per_seg = 4;
        handles_per_curve = handles_per_seg * segs;
        curves = 1;
        for(int a = 0; a < curves; a++)
            for(unsigned i = 0; i < handles_per_curve; i++)
                handles.push_back(Point(150 + 300*i/(4*segs), uniform() * 150 + 150 - 150 * a));
        for(unsigned i = 0; i < 4; i++)
            handles.push_back(Point(150 + 100*i,100));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "Piecewise Toy", new PwToy());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
