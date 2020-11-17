# pack-mm.py

import sys
import numpy as np
from scipy.io import mmread

# inpath  = sys.argv[1]
outpath = 'target.bin'

# >>
import random
import numpy as np
import pandas as pd
from tqdm import tqdm
from time import time
import networkx as nx
from networkx.algorithms import isomorphism as iso

_ = random.seed(123)
_ = np.random.seed(234)

n = 1000

G   = nx.erdos_renyi_graph(n, p=0.01)
adj = nx.adjacency_matrix(G)
# <<
# adj = mmread(inpath).tocsr()
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

_  = open(outpath, 'wb').write(bytearray(packed_data))