import random
import numpy as np
import pandas as pd
from tqdm import tqdm
from time import time
import networkx as nx
from networkx.algorithms import isomorphism as iso

_ = random.seed(123)
_ = np.random.seed(234)

n = 50

G = nx.erdos_renyi_graph(n, p=0.05)

Q = nx.Graph()
_ = [Q.add_edge(*e) for e in [(0, 1), (1, 2), (2, 3), (0, 3)]]

t = time()
GM = iso.GraphMatcher(G, Q)
a = []
for i, z in enumerate(GM.subgraph_isomorphisms_iter()):
  a.append(z)
  # print(i, z)

print(time() - t)

# --

class Matcher:
  def __init__(self, G, Q):
    
    self.G = G
    self.Q = Q
    
    self.M_G = {}
    self.M_Q = {}
    self.T_G = set([])
    self.T_Q = set([])
    
    self.query_size = len(Q)
    
    self.G_nodes = set(G.nodes())
    self.Q_nodes = set(Q.nodes())
    
  def pairs(self):
    if self.T_G and self.T_Q:
      C_G = self.T_G
      C_Q = self.T_Q
    
    else:
      C_G = self.G_nodes - set(self.M_G)
      C_Q = self.Q_nodes - set(self.M_Q)
    
    Q_node = min(C_Q)
    for G_node in C_G:
      yield G_node, Q_node
  
  def _boundary(self, G, M):
    # !! Can update these incrementally, but this is simpler for now
    
    tmp = set([])
    for node in M:
      tmp |= set(G.neighbors(node))
    
    tmp -= set(M)
    return tmp
  
  def match(self, depth=0):
    if len(self.M_G) == self.query_size:
      yield self.M_G.copy()
    else:  
      for G_node, Q_node in self.pairs():
        
        if not self.sem_feasible(G_node, Q_node): continue
        if not self.syn_feasible(G_node, Q_node): continue
        
        # Add to partial match + recompute boundary
        self.M_G[G_node] = Q_node
        self.M_Q[Q_node] = G_node
        self.T_G = self._boundary(self.G, self.M_G)
        self.T_Q = self._boundary(self.Q, self.M_Q)
        
        yield from self.match(depth + 1)
        
        # Remove from partial match
        del self.M_G[G_node]
        del self.M_Q[Q_node]
        self.T_G = self._boundary(self.G, self.M_G)
        self.T_Q = self._boundary(self.Q, self.M_Q)
        print('.' * len(self.M_G))
  
  def sem_feasible(self, G_node, Q_node):
    return True
  
  def syn_feasible(self, G_node, Q_node):
    # !! Assert no self edges, no multi-edges
    
    # --
    # Neighbors of A_node must match to neighbors of B_node (R_{pre,suc})
    
    for neib in self.G.neighbors(G_node):
      if neib in self.M_G:
        if self.M_G[neib] not in self.Q.neighbors(Q_node):
          return False
    
    for neib in self.Q.neighbors(Q_node):
      if neib in self.M_Q:
        if self.M_Q[neib] not in self.G.neighbors(G_node):
          return False
    
    # --
    # G_node must have at least as many neighbors in T as Q_node (R_{in,out})
    
    G_num = 0
    for neib in self.G.neighbors(G_node):
      if neib in self.T_G:
        G_num += 1
    
    Q_num = 0
    for neib in self.Q.neighbors(Q_node):
      if neib in self.T_Q:
        Q_num += 1
    
    if G_num < Q_num:
      return False
    
    # --
    # G_node must have at least as many neighbors outside of M + T as Q_node
    
    G_num = 0
    for neib in self.G[G_node]:
      if (neib not in self.T_G) and (neib not in self.M_G):
        G_num += 1
    
    Q_num = 0
    for neib in self.Q[Q_node]:
      if (neib not in self.T_Q) and (neib not in self.M_Q):
        Q_num += 1
    
    if G_num < Q_num:
      return False
  
    return True
    

print('-' * 50)
b = []

t = time()

for i, z in enumerate(Matcher(G, Q).match()):
  b.append(z)
  # print(i, z)

print(time() - t)

# --
# Check matches

print(len(a), len(b))
assert len(a) == len(b)

a = sorted(a, key=lambda x: tuple(x.keys()))
b = sorted(b, key=lambda x: tuple(x.keys()))
assert a == b