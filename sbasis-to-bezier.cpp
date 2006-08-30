/* From Sanchez-Reyes 1997
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)choose(2k+1,k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

This is wrong, it should read
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

*/
#include "sbasis-to-bezier.h"
#include "choose.h"

double W(unsigned n, unsigned j, unsigned k) {
    unsigned q = (n+1)/2;
    if((n & 1) == 0 && j == q && k == q)
        return 1;
    if(k > n-k) return W(n, n-j, n-k);
    assert(!(k < 0));
    if(k < 0) return 0;
    assert(!(k >= q));
    if(k >= q) return 0;
    //assert(!(j >= n-k));
    if(j >= n-k) return 0;
    //assert(!(j < k));
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k) /
        choose<double>(n,j);
}

// this produces a degree k bezier from a degree k sbasis
std::vector<double>
sbasis_to_bezier(SBasis const &B, unsigned q) {
    std::vector<double> result;
    if(q > B.size())
        q = B.size();
    unsigned n = q*2;
    result.resize(n, 0);
    n--;
    for(int k = 0; k < q; k++) {
        for(int j = 0; j <= n-k; j++) {
            result[j] += (W(n, j, k)*B[k][0] +
                          W(n, n-j, k)*B[k][1]);
        }
    }
    return result;
}

// this produces a degree k bezier from a degree q sbasis
std::vector<Geom::Point>
sbasis_to_bezier(multidim_sbasis<2> const &B, unsigned q) {
    std::vector<Geom::Point> result;
    if(q > B.size())
        q = B.size();
    unsigned n = q*2;
    result.resize(n, Geom::Point(0,0));
    n--;
    for(int dim = 0; dim < 2; dim++) {
        for(int k = 0; k < q; k++) {
            for(int j = 0; j <= n-k; j++) {
                result[j][dim] += (W(n, j, k)*B[dim][k][0] +
                             W(n, n-j, k)*B[dim][k][1]);
                }
        }
    }
    return result;
}

std::vector<Geom::Point>
sbasis2_to_bezier(multidim_sbasis<2> const &B, unsigned q) {
    if(q > B.size())
        q = B.size();
    if(q != 2)
	    return sbasis_to_bezier(B, B.size());
    std::vector<Geom::Point> result;
    const double third = 1./3;
    result.resize(4, Geom::Point(0,0));
    for(int dim = 0; dim < 2; dim++) {
        const SBasis &Bd(B[dim]);
        result[0][dim] = Bd[0][0];
        result[1][dim] = third*(2*Bd[0][0] +   Bd[0][1] + Bd[1][0]);
        result[2][dim] = third*(  Bd[0][0] + 2*Bd[0][1] + Bd[1][1]);
        result[3][dim] = Bd[0][1];
    }
    assert(result.size() == 4);
    return result;
}

// mutating
void
subpath_from_sbasis(Geom::PathBuilder &pb, multidim_sbasis<2> const &B, double tol) {
    if(B.tail_error(2) < tol || B.size() == 2) { // nearly cubic enough
        if(B.size() == 1) {
            pb.push_line(Geom::Point(B[0][0][0], B[1][0][0]),
                         Geom::Point(B[0][0][1], B[1][0][1]));
        } else {
            std::vector<Geom::Point> bez = sbasis_to_bezier(B, 2);
            reverse(bez.begin(), bez.end());
            pb.push_cubic(bez[0], bez[1], bez[2], bez[3]);
        }
    } else {
        subpath_from_sbasis(pb, compose(B, BezOrd(0, 0.5)), tol);
        subpath_from_sbasis(pb, compose(B, BezOrd(0.5, 1)), tol);
    }
}

#include <iostream>

/***
/* This version works by inverting a reasonable upper bound on the error term after subdividing the
curve at $a$.  We keep biting off pieces until there is no more curve left.
* 
* Derivation: The tail of the power series is $a_ks^k + a_{k+1}s^{k+1} + \ldots = e$.  A
* subdivision at $a$ results in a tail error of $e*A^k, A = (1-a)a$.  Let this be the desired
* tolerance tol $= e*A^k$ and invert getting $A = e^{1/k}$ and $a = 1/2 - \sqrt{1/4 - A}$
*/
void
subpath_from_sbasis_incremental(Geom::PathBuilder &pb, multidim_sbasis<2> B, double tol) {
    const unsigned k = 2; // cubic bezier
    double te = B.tail_error(k);
    
    //std::cout << "tol = " << tol << std::endl;
    while(1) {
        double A = sqrt(tol/te); // pow(te, 1./k)
        double a = A;
        if(A < 1) {
            A = std::min(A, 0.25);
            a = 0.5 - sqrt(0.25 - A); // quadratic formula
            if(a > 1) a = 1; // clamp to the end of the segment
        } else
            a = 1;
        assert(a > 0);
        //std::cout << "te = " << te << std::endl;
        //std::cout << "A = " << A << "; a=" << a << std::endl;
        multidim_sbasis<2> Bs = compose(B, BezOrd(0, a));
        assert(Bs.tail_error(k));
        std::vector<Geom::Point> bez = sbasis_to_bezier(Bs, 2);
        reverse(bez.begin(), bez.end());
        pb.push_cubic(bez[0], bez[1], bez[2], bez[3]);
        
// move to next piece of curve
        if(a >= 1) break;
        B = compose(B, BezOrd(a, 1)); 
        te = B.tail_error(k);
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :