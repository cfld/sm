// apsp.cpp

#pragma GCC diagnostic ignored "-Wunused-result" 

#include <omp.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <chrono>
#include <map>
#include <vector>
#include <algorithm>

typedef int64_t Int;
typedef std::map<Int, Int> Dict;
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

void _boundary(Set& T, Dict& M, Int* indptr, Int* indices) {
  // Clear T
  T.clear();
  
  // Add all neighbors of M to T
  for(auto it = M.begin(); it != M.end(); ++it) {
    for(Int offset = indptr[it->first]; offset < indptr[it->first + 1]; offset++) {
      T.push_back(indices[offset]);  
    }
  }  
  
  // Uniqify
  std::sort(T.begin(), T.end());
  auto last = std::unique(T.begin(), T.end());
  T.erase(last, T.end());
  
  // Remove elements in M
  for(auto it = M.begin(); it != M.end(); ++it) {
    T.erase(std::remove(T.begin(), T.end(), it->first), T.end());
  }
  
}

bool sem_feasible(Int G_node, Int Q_node, Dict& M_G, Dict& M_Q, Set& T_G, Set& T_Q) {
  return true; // placeholder
}

bool syn_feasible(Int G_node, Int Q_node, Dict& M_G, Dict& M_Q, Set& T_G, Set& T_Q) {
  Int G_start = G_indptr[G_node];
  Int G_end   = G_indptr[G_node + 1];
  
  Int Q_start = Q_indptr[Q_node];
  Int Q_end   = Q_indptr[Q_node + 1];
  
  // --
  // Neighbors of A_node must match to neighbors of B_node (R_{pre,suc})
  
  for(Int offset = Q_start; offset < Q_end; offset++) {
    Int neib = Q_indices[offset];
    if(M_Q.count(neib) == 1) {
      if(!std::binary_search(G_indices + G_start, G_indices + G_end, M_Q[neib])) {
        return false;
      }      
    }
  }

  for(Int offset = G_start; offset < G_end; offset++) {
    Int neib = G_indices[offset];
    if(M_G.count(neib) == 1) {
      if(!std::binary_search(Q_indices + Q_start, Q_indices + Q_end, M_G[neib])) {
        return false;
      }      
    }
  }
  
  // --
  // G_node must have at least as many neighbors in T as Q_node (R_{in,out})
  
  Int G_num = 0;
  Int Q_num = 0;
  
  for(Int offset = G_start; offset < G_end; offset++) {
    Int neib = G_indices[offset];
    if(std::binary_search(T_G.begin(), T_G.end(), neib)) {
      G_num++;
    }
  }

  for(Int offset = Q_start; offset < Q_end; offset++) {
    Int neib = Q_indices[offset];
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
    Int neib = G_indices[offset];
    if(
      M_G.count(neib) == 0 &&
      !std::binary_search(T_G.begin(), T_G.end(), neib)
    ) {
      G_num++;
    }
  }

  for(Int offset = Q_start; offset < Q_end; offset++) {
    Int neib = Q_indices[offset];
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

Int _do_match(Dict& M_G, Dict& M_Q, Set& T_G, Set& T_Q) {  
  if((Int)M_Q.size() == q_nodes) {
    return 1;
  }
  
  Int out    = 0;
  Int Q_node = T_Q[0]; // Minimum element
  
  Set T_G_copy(T_G);
  for(const auto& G_node : T_G_copy) {
    
    if(!sem_feasible(G_node, Q_node, M_G, M_Q, T_G, T_Q)) continue;
    if(!syn_feasible(G_node, Q_node, M_G, M_Q, T_G, T_Q)) continue;
    
    M_G[G_node] = Q_node;
    M_Q[Q_node] = G_node;
    _boundary(T_G, M_G, G_indptr, G_indices);
    _boundary(T_Q, M_Q, Q_indptr, Q_indices);
    
    out += _do_match(M_G, M_Q, T_G, T_Q);
    
    M_G.erase(G_node);
    M_Q.erase(Q_node);
    _boundary(T_G, M_G, G_indptr, G_indices);
    _boundary(T_Q, M_Q, Q_indptr, Q_indices);
  }
  
  return out;
}

Int do_match(Int G_node, Int Q_node) {
  Dict M_G;
  Dict M_Q;
  Set T_G;
  Set T_Q;
  
  M_G[G_node] = Q_node;
  M_Q[Q_node] = G_node;
  
  _boundary(T_G, M_G, G_indptr, G_indices);
  _boundary(T_Q, M_Q, Q_indptr, Q_indices);
  
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
    // Load target

    fptr = fopen("query.bin", "rb");

    fread(&q_nodes, sizeof(Int), 1, fptr);
    fread(&q_edges, sizeof(Int), 1, fptr);
    
    Q_indptr  = (Int*)malloc((q_nodes + 1) * sizeof(Int));
    Q_indices = (Int*)malloc(q_edges * sizeof(Int));

    fread(Q_indptr,  sizeof(Int), q_nodes + 1, fptr);
    fread(Q_indices, sizeof(Int), q_edges, fptr);

    fclose(fptr);
    
    printf("g_nodes=%ld | g_edges=%ld | q_nodes=%ld | q_edges=%ld\n", g_nodes, g_edges, q_nodes, q_edges);
    
    // --
    // Run
    auto t1 = high_resolution_clock::now();
    
    Int n_matches = 0;
    Int Q_node    = 0;
    for(Int G_node = 0; G_node < g_nodes; G_node++) {
      // std::cout << "G_node: " << G_node << std::endl;
      Int match = do_match(G_node, Q_node);
      n_matches += match;
    }

    auto elapsed = high_resolution_clock::now() - t1;
    long long ms = duration_cast<milliseconds>(elapsed).count();
    printf("elapsed=%lld\n", ms);
    
    // --
    // Log
    
    printf("n_matches=%ld\n", n_matches);
    
    return 0;
}
