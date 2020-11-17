#!/usr/bin/env python

"""
  make-query.py
"""

import numpy as np
import networkx as nx

Q   = nx.Graph()
_   = [Q.add_edge(*e) for e in [(0, 1), (1, 2), (2, 3), (3, 0)]]

adj = nx.adjacency_matrix(Q)
adj = ((adj + adj.T) > 0).astype(np.float32)
adj.sort_indices()

# --

n_nodes = adj.shape[0]
n_edges = adj.nnz

packed_data = np.hstack([
    np.int64(n_nodes),
    np.int64(n_edges),
    adj.indptr.astype(np.int64),
    adj.indices.astype(np.int64),
    adj.data.astype(np.int64),
])

_  = open('query.bin', 'wb').write(bytearray(packed_data))
