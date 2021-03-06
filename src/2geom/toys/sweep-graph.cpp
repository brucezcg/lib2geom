#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/pathvector.h>
#include <2geom/sbasis-geometric.h>

#define SWEEP_GRAPH_DEBUG

#ifdef SWEEP_GRAPH_DEBUG
#include <2geom/toposweep.cpp>
#else
#include <2geom/toposweep.h>
#endif

#include <cstdlib>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <algorithm>
#include <queue>
#include <functional>
#include <limits>

using namespace Geom;

void set_rainbow(cairo_t *cr, unsigned i) {
    cairo_set_source_rgba(cr, colour::from_hsl(i*0.5, 1, 0.5, 0.75));
}

void draw_node(cairo_t *cr, Point h) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x, y, 2, 0, M_PI*2);
}

void draw_section(cairo_t *cr, Section const &s, PathVector const &ps) {
    Interval ti(s.f, s.t);
    Curve *curv = s.curve.get(ps).portion(ti.min(), ti.max());
    //draw_node(cr, s.fp);
    cairo_curve(cr, *curv);
    //draw_node(cr, s.tp);
    cairo_stroke(cr);
    delete curv;
}

void draw_graph(cairo_t *cr, TopoGraph const &graph) {
    for(unsigned i = 0; i < graph.size(); i++) {
        set_rainbow(cr, i);
        for(unsigned j = 0; j < graph[i].degree(); j++) {
            draw_ray(cr, graph[i].avg, 10*unit_vector(graph[graph[i][j].other].avg - graph[i].avg));
            cairo_stroke(cr);
        }
    }
}

void write_graph(TopoGraph const &graph) {
    for(unsigned i = 0; i < graph.size(); i++) {
        std::cout << i << " " << graph[i].avg << " [";
        for(unsigned j = 0; j < graph[i].degree(); j++)
            std::cout << graph[i][j].other << ", ";
        std::cout << "]\n";
    }
}

/*
void draw_edges(cairo_t *cr, std::vector<Edge> const &edges, PathVector const &ps, double t) {
    for(unsigned j = 1; j < edges.size(); j++) {
        const Section * const s1 = edges[j-1].section,
                       * const s2 = edges[j].section;
        Point fp = s1->curve.get(ps)(lerp(t, s1->f, s1->t));
        Point tp = s2->curve.get(ps)(lerp(t, s2->f, s2->t));
        draw_ray(cr, fp, tp - fp);
        cairo_stroke(cr);
    }
}

void draw_edge_orders(cairo_t *cr, std::vector<Vertex*> const &vertices, PathVector const &ps) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        set_rainbow(cr, i);
        draw_edges(cr, vertices[i]->enters, ps, 0.6);
        draw_edges(cr, vertices[i]->exits, ps, 0.4);
    }
}
*/

void draw_areas(cairo_t *cr, Areas const &areas, PathVector const &pa) {
    PathVector ps = areas_to_paths(pa, areas);
    for(unsigned i = 0; i < ps.size(); i++) {
        double area;
        Point centre;
        Geom::centroid(ps[i].toPwSb(), centre, area);
        double d = 5.;
        if(area < 0) cairo_set_dash(cr, &d, 1, 0);
        cairo_path(cr, ps[i]);
        cairo_stroke(cr);
        cairo_set_dash(cr, &d, 0, 0);
    }
}

void draw_area(cairo_t *cr, Area const &area, PathVector const &pa) {
    for(unsigned i = 0; i < area.size(); i++) draw_section(cr, *area[i], pa);
}

#ifdef SWEEP_GRAPH_DEBUG
void draw_context(cairo_t *cr, int cix, PathVector const &pa) {
    if(contexts.empty()) return;
    cix %= std::max(1,(int)contexts.size());
    for(unsigned i = 0; i < contexts[cix].size(); i++) {
        set_rainbow(cr, i);
        draw_section(cr, contexts[cix][i], pa);
        draw_number(cr, contexts[cix][i].curve.get(pa)
                        ((contexts[cix][i].t + contexts[cix][i].f) / 2), i);
        cairo_stroke(cr);
    }
    cairo_set_source_rgba(cr, 0,0,0,1);
    for(unsigned i = 0; i < monoss[cix].size(); i++) {
        draw_section(cr, monoss[cix][i], pa);
        cairo_stroke(cr);
    }
    for(unsigned i = 0; i < chopss[cix].size(); i++) {
        draw_section(cr, chopss[cix][i], pa);
        cairo_stroke(cr);
    }
}
#endif

class SweepWindow: public Toy {
    vector<Path> path, path2;
    std::vector<Toggle> toggles;
    PointHandle p, p2;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        if (p2.pos[X] < 0) p2.pos[X] = 0;

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 3);

#ifdef SWEEP_GRAPH_DEBUG
        monoss.clear();
        contexts.clear();
        chopss.clear();
#endif
        
        PathVector pa = path;
        PathVector pa2 = path2 + p.pos;
        concatenate(pa, pa2);
        
        TopoGraph output(pa,X, .00001);
        //output.assert_invariants();
        //double_whiskers(output);
        //output.assert_invariants();
        
        int cix = p2.pos[X] / 10;
        
#ifdef SWEEP_GRAPH_DEBUG
        draw_context(cr, cix, pa);
#endif
        write_graph(output);
        draw_graph(cr, output);
        
        *notify << contexts.size() << "\n";
        
        /*Areas areas = traverse_areas(output);
        //remove_area_whiskers(areas);
        //filter_areas(pa, areas, UnionOp(path.size(), false, false));
        for(unsigned i = 0; i < areas.size(); i++) {
            set_rainbow(cr, i);
            draw_area(cr, areas[i], pa);
        }
        //draw_areas(cr, areas, pa, X);
        */
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    
    void key_hit(GdkEventKey* e) {
        if(e->keyval == 'a') p2.pos[X] = 0;
        else if(e->keyval == '[') p2.pos[X] -= 10;
        else if(e->keyval == ']') p2.pos[X] += 10;
        redraw();
    }
    
    public:
    SweepWindow () {}
    void first_time(int argc, char** argv) {
        const char *path_name="sanitize_examples.svgd",
                     *path2_name="sanitize_examples.svgd";
        if(argc > 1)
            path_name = argv[1];
        if(argc > 2)
            path2_name = argv[2];
        path = read_svgd(path_name); //* Scale(3);
        path2 = read_svgd(path2_name);
        OptRect bounds = bounds_exact(path);
        if(bounds) path += Point(10,10)-bounds->min();
        bounds = bounds_exact(path2);
        if(bounds) path2 += Point(20,20)-bounds->min();
        p = PointHandle(Point(100,300));
        handles.push_back(&p);
        p2 = PointHandle(Point(200, 300));
        handles.push_back(&p2);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SweepWindow());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
