// vf2.cpp

// There's some small bug here, I think

#pragma GCC diagnostic ignored "-Wunused-result" 

#include <omp.h>

#include <chrono>
#include <iostream>

#include <map>
#include <algorithm>

typedef int64_t Int;
typedef std::map<Int, Int> Map;
typedef std::vector<Int> Set;

Int g_nodes;
Int g_edges;
Int* G_indptr;
Int* G_indices;

Int q_nodes;
Int q_edges;
Int* Q_indptr;
Int* Q_indices;

using namespace std;
using namespace std::chrono;

void compute_boundary(Set& T, Map& M, Int* indptr, Int* indices) {
  // I'm sure this could be faster
  
  // Clear T
  T.clear();
  
  // Add all neighbors of M to T
  for(auto it = M.begin(); it != M.end(); ++it) {
    for(Int offset = indptr[it->first]; offset < indptr[it->first + 1]; offset++) {
      T.push_back(indices[offset]);  
    }
  }  
  
  // Sort + uniq
  std::sort(T.begin(), T.end());
  auto last = std::unique(T.begin(), T.end());
  T.erase(last, T.end());
  
  // Remove elements in M from T
  for(auto it = M.begin(); it != M.end(); ++it) {
    T.erase(std::remove(T.begin(), T.end(), it->first), T.end());
  }
}

bool sem_feasible(Int G_node, Int Q_node, Map& M_G, Map& M_Q, Set& T_G, Set& T_Q) {
  return true; // placeholder
}

bool syn_feasible(Int G_node, Int Q_node, Map& M_G, Map& M_Q, Set& T_G, Set& T_Q) {
  Int G_num;
  Int Q_num;
  Int neib;
  
  Int G_start = G_indptr[G_node];
  Int G_end   = G_indptr[G_node + 1];
  
  Int Q_start = Q_indptr[Q_node];
  Int Q_end   = Q_indptr[Q_node + 1];
  
  // --
  // Neighbors of A_node must match to neighbors of B_node (R_{pre,suc})
  
  for(Int offset = Q_start; offset < Q_end; offset++) {
    neib = Q_indices[offset];
    if(M_Q.count(neib) == 1) {
      if(!std::binary_search(G_indices + G_start, G_indices + G_end, M_Q[neib])) {
        return false;
      }      
    }
  }

  for(Int offset = G_start; offset < G_end; offset++) {
    neib = G_indices[offset];
    if(M_G.count(neib) == 1) {
      if(!std::binary_search(Q_indices + Q_start, Q_indices + Q_end, M_G[neib])) {
        return false;
      }      
    }
  }
  
  // --
  // G_node must have at least as many neighbors in T as Q_node (R_{in,out})
  
  G_num = 0;
  Q_num = 0;
  
  for(Int offset = G_start; offset < G_end; offset++) {
    neib = G_indices[offset];
    if(std::binary_search(T_G.begin(), T_G.end(), neib)) {
      G_num++;
    }
  }

  for(Int offset = Q_start; offset < Q_end; offset++) {
    neib = Q_indices[offset];
    if(std::binary_search(T_Q.begin(), T_Q.end(), neib)) {
      Q_num++;
    }
  }
  
  if(G_num < Q_num) return false;
  
  // --
  // G_node must have at least as many neighbors outside of M + T as Q_node
  
  G_num = 0;
  Q_num = 0;
  
  for(Int offset = G_start; offset < G_end; offset++) {
    neib = G_indices[offset];
    if(
      M_G.count(neib) == 0 &&
      !std::binary_search(T_G.begin(), T_G.end(), neib)
    ) {
      G_num++;
    }
  }

  for(Int offset = Q_start; offset < Q_end; offset++) {
    neib = Q_indices[offset];
    if(
      M_Q.count(neib) == 0 &&
      !std::binary_search(T_Q.begin(), T_Q.end(), neib)
    ) {
      Q_num++;
    }
  }
  
  if(G_num < Q_num) return false;
  
  return true;
}

Int _do_match(Map& M_G, Map& M_Q, Set& T_G, Set& T_Q) {  
  if((Int)M_Q.size() == q_nodes) {
    return 1;
  }
  
  Int out    = 0;
  Int Q_node = T_Q[0]; // Minimum element
  
  Set T_G_(T_G);
  Set T_Q_(T_Q);
  for(const auto& G_node : T_G_) {
    if(!sem_feasible(G_node, Q_node, M_G, M_Q, T_G, T_Q)) continue;
    if(!syn_feasible(G_node, Q_node, M_G, M_Q, T_G, T_Q)) continue;
    
    M_G[G_node] = Q_node;
    M_Q[Q_node] = G_node;
    
    compute_boundary(T_G, M_G, G_indptr, G_indices);
    compute_boundary(T_Q, M_Q, Q_indptr, Q_indices);
    
    out += _do_match(M_G, M_Q, T_G, T_Q);
    
    M_G.erase(G_node);
    M_Q.erase(Q_node);

    T_G = T_G_;
    T_Q = T_Q_;
  }
  
  return out;
}

Int do_match(Int G_node, Int Q_node) {
  Map M_G, M_Q;
  Set T_G, T_Q;
  
  M_G[G_node] = Q_node;
  M_Q[Q_node] = G_node;
  
  compute_boundary(T_G, M_G, G_indptr, G_indices);
  compute_boundary(T_Q, M_Q, Q_indptr, Q_indices);
  
  return _do_match(M_G, M_Q, T_G, T_Q);
}


int main(int argc, char *argv[]) {

    FILE *fptr = NULL;
    
    // --
    // Load target

    fptr = fopen("target.bin", "rb");

    fread(&g_nodes, sizeof(Int), 1, fptr);
    fread(&g_edges, sizeof(Int), 1, fptr);
    
    G_indptr  = (Int*)malloc((g_nodes + 1) * sizeof(Int));
    G_indices = (Int*)malloc(g_edges * sizeof(Int));

    fread(G_indptr,  sizeof(Int), g_nodes + 1, fptr);
    fread(G_indices, sizeof(Int), g_edges, fptr);
    
    fclose(fptr);
    
    // --
    // Load query

    fptr = fopen("query.bin", "rb");

    fread(&q_nodes, sizeof(Int), 1, fptr);
    fread(&q_edges, sizeof(Int), 1, fptr);
    
    Q_indptr  = (Int*)malloc((q_nodes + 1) * sizeof(Int));
    Q_indices = (Int*)malloc(q_edges * sizeof(Int));

    fread(Q_indptr,  sizeof(Int), q_nodes + 1, fptr);
    fread(Q_indices, sizeof(Int), q_edges, fptr);

    fclose(fptr);
    
    // --
    // Run
    
    auto t1 = high_resolution_clock::now();
    
    Int n_matches = 0;
    
    #pragma omp parallel for schedule(dynamic) reduction(+: n_matches)
    for(Int G_node = 0; G_node < g_nodes; G_node++) {
      Int tmp = do_match(G_node, 0);
      n_matches += tmp;
    }

    auto elapsed = high_resolution_clock::now() - t1;
    long long ms = duration_cast<milliseconds>(elapsed).count();
    
    // --
    // Log
    
    std::cout << "g_nodes:"   << g_nodes   << " ";
    std::cout << "g_edges:"   << g_edges   << " ";
    std::cout << "q_nodes:"   << q_nodes   << " ";
    std::cout << "q_edges:"   << q_edges   << " ";
    std::cout << "n_matches:" << n_matches << " ";
    std::cout << "elapsed:"   << ms        << std::endl;
    
    return 0;
}
