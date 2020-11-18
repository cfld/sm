#!/usr/bin/env python

"""
  pack-target.py
  
  Generate target graph
"""

import sys
import random
import numpy as np
import networkx as nx
from scipy.io import mmread
from scipy import sparse as sp

_ = random.seed(123)
_ = np.random.seed(234)

n = int(sys.argv[1])

G   = nx.erdos_renyi_graph(n, p=0.01)
adj = nx.adjacency_matrix(G)

adj = ((adj + adj.T) > 0).astype(np.float)
adj.sort_indices()

n_nodes = adj.shape[0]
n_edges = adj.nnz

packed_data = np.hstack([
    np.int64(n_nodes),
    np.int64(n_edges),
    adj.indptr.astype(np.int64),
    adj.indices.astype(np.int64),
    adj.data.astype(np.int64),
])

_  = open('target.bin', 'wb').write(bytearray(packed_data))