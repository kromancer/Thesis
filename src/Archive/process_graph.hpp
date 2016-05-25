/*
#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_matrix.hpp>


typedef boost::adjacency_matrix<boost::directedS> Graph;

using namespace boost;

enum{ P0, P1, P2 };

Graph g(P2);

bool  g2(9);

void make_graph()
{
    add_edge(P0, P1, g);
    add_edge(P0, P2, g);
    add_edge(P1, P0, g);
    add_edge(P1, P2, g);
    add_edge(P1, P2, g);
    add_edge(P2, P0, g);
    add_edge(P2, P1, g);

}
*/

#include <array>

using namespace std;

array<bool, 9> g;

void make_graph()
{
    g[0] = false;
    g[1] = true;
    g[2] = true;

    g[3] = true;
    g[4] = false;
    g[5] = true;

    g[6] = true;
    g[7] = true;
    g[8] = false;    
}
