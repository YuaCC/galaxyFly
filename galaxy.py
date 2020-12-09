import numpy as np
import sys

''' 
build_finite_field
'''
def build_finite_field(q):
    if q in [1,2]:
        return {0:0,1:1},{0:0,1:1},1
    from finitefield import GF
    field = GF(q)
    elts_map = {}
    generate_element = None
    for (i, v) in enumerate(field):
        elts_map[i] = v
        generate_set = set([v**k for k in range(q)])
        if len(generate_set)==q-1:
            generate_element = v
    rev_elts_map = {v: k for k, v in elts_map.items()}
    return elts_map,rev_elts_map,rev_elts_map[generate_element]
'''
计算X,X_
'''
def get_X(q, xi,elts_map,rev_elts_map):
    if q in [1,2]:
        return {1,},{1,}
    if q % 4 == 1:
        X = set(range(0, q - 1, 2))
        X_ = set(range(1, q - 1, 2))
    elif q % 4 == 3:
        l = (q + 1) // 4
        X1 = set(range(0, 2 * l, 2))
        X2 = set(range(2 * l - 1, 4 * l - 1, 2))
        X1_ = set(range(1, 2 * l, 2))
        X2_ = set(range(2 * l, 4 * l - 1, 2))
        X = X1.union(X2)
        X_ = X1_.union(X2_)
    elif q % 4 == 0:
        l = q // 4
        X = set(range(0, 4 * l  , 2))
        X_ = set(range(1, 4 * l , 2))
    else:
        raise ValueError("q should be prime pow and >=3")
    X = {rev_elts_map[elts_map[xi]**v] for v in X}
    X_ = {rev_elts_map[elts_map[xi]**v] for v in X_}
    return X, X_


'''
H x->x_(u) 同构函数将G中节点映射为G_中节点
because X*xi = X_  mod q, 
so if H x->x_(u) = v, then u * xi = v  mod q
so v = u * xi  mod q 
'''
def get_H(q, xi,elts_map,rev_elts_map):
    if q in [1,2]:
        return {0:0,1:1}
    h = { u:rev_elts_map[elts_map[u]*elts_map[xi]] for u in range(q)  }
    return h

'''
Algorithm1,为每个cluster生成一个Coordinate matrix。
'''
def get_M(H, q, n):
    M = []
    q_vector = [v for v in range(q)]
    Hq_vector = [H[v] for v in range(q)]
    for k in range(n):
        Mk = np.zeros((q, n), dtype=np.int)
        for i in range(n):
            if i >= k:
                Mk[:, i] = q_vector
            else:
                Mk[:, i] = Hq_vector
        M.append(Mk)
    return M

''' 
Algorithm2,在每个cluster内部的super node 之间生成边。 任意cluster内部super node链接方式是相同的
返回值:
    edges类型为set<tuple>,表示node_i 与node_j之间有边相连
'''
def get_intra_cluster_edges(q, M, X,k,elts_map,rev_elts_map):
    if q ==2:
        return {(1,0),}
    if q ==1:
        return set()
    edges = set()
    for i in range(q):
        for j in range(i + 1, q):
            if rev_elts_map[elts_map[M[k][j][k]] - elts_map[M[k][i][k]]] in X:
                edges.add((j, i))
    return edges

'''
  Algorithm3,在两个cluster的super node 之间生成边。 任意两个cluster的super node 之间边的连接方式都是相同的
  返回值:
    edges类型为set<tuple>,每个tuple包含两个元素a,b，表示cluster_i中的节点a与cluster_j中的节点b相连， 
'''
def get_inter_cluster_edges(q, M,i,j):
    if q ==2:
        return {(0,0),(1,1)}
    if q ==1:
        return {(0,0),}
    edges = set()
    for r in range(q):
        for s in range(q):
            if M[i][r][j] == M[j][s][i]:
                edges.add((r, s))
    return edges
'''
计算图直径
'''
def get_diameter(edges_intra,edges_inter,q):
    dis = np.ones((2*q,2*q))*-1
    for i in range(2*q):
        dis[i][i]=0
    for (i,j) in edges_intra:
        dis[i][j] =dis[j][i]=1
        dis[i+q][j+q] =dis[j+q][i+q]=1
    for (i,j) in edges_inter:
        dis[i][j+q] =dis[j+q][i]=1
    degree = dis==1
    degree = np.sum(degree,1)
    max_degree = np.max(degree )
    max_ij =(0,0)
    for k in range(0,2*q):
        for i in range(2*q):
            for j in range(2*q):
                if (dis[i][k]!=-1 and dis[k][j]!=-1 ) and (dis[i][j]==-1 or dis[i][j] > dis[i][k] +dis[k][j]):
                    dis[i][j] = dis[i][k] +dis[k][j]
                    if dis [i][j] > dis[max_ij[0]][max_ij[1]]:
                        max_ij = (i,j)
    # print(dis)
    return dis[max_ij[0]][max_ij[1]],max_ij,max_degree



class Node:
    node_id =0
    def __init__(self):
        self.node_id = Node.node_id
        Node.node_id =  Node.node_id +1

    def __repr__(self):
        return 'node {}'.format(self.node_id)

class Router:
    router_id = 0
    def __init__(self,p):
        self.p=p
        self.router_id = Router.router_id
        Router.router_id =  Router.router_id +1
        self.linked_nodes = [Node() for i in range(p)]
        self.linked_routers = []

    def degree(self):
        return len(self.linked_nodes)+len(self.linked_routers)

    def __repr__(self):
        ret = 'router {}'.format(self.router_id)
        for node in self.linked_nodes:
            ret = ret + " {}".format(node)
        for router in self.linked_routers:
            ret = ret + " router {}".format(router.router_id)
        return ret

    def link(self,router):
        if router!=self:
            if router not in self.linked_routers:
                self.linked_routers.append(router)
            if self not in router.linked_routers:
                router.linked_routers.append(self)

class SuperNode:
    def __init__(self,a,p):
        self.a=a
        self.p=p
        self.routers = [Router(p) for i in range(a)]
        for i in range(a):
            for j in range(i+1,a):
                self.routers[i].link(self.routers[j])
    def link(self,supernode):
        if supernode!=self:
            degrees = [r.degree() for r in self.routers]
            other_degrees = [r.degree() for r in supernode.routers]
            min_idx = int(np.argmin(degrees) )
            router = self.routers[min_idx]
            other_min_idx = int(np.argmin(other_degrees))
            other_router = supernode.routers[other_min_idx]
            router.link(other_router)
    def __repr__(self):
        ret = ''
        for r in self.routers:
            ret = ret + (str(r)+'\n')
        return ret

class Cluster:
    def __init__(self,q,a,p,M,X,k,elts_map,rev_elts_map):
        self.p=p
        self.q=q
        self.a = a
        self.supernodes =[SuperNode(a,p) for i in range(q)]
        self.M = M
        self.X = X
        self.k = k
        intra_edges = get_intra_cluster_edges(q,M,X,k,elts_map,rev_elts_map)
        for u,v in intra_edges:
            self.supernodes[u].link(self.supernodes[v])
    def link(self,cluster):
        inter_edges = get_inter_cluster_edges(self.q,self.M,self.k,cluster.k)
        for u,v in inter_edges:
            self.supernodes[u].link(cluster.supernodes[v])
    def __repr__(self):
        ret = ''
        for s in self.supernodes:
            ret = ret + str(s)
        return ret




class GalaxyFly:
    def __init__(self,n,q,a,p):
        self.n=n
        self.q=q
        self.a=a
        self.p=p
        elts_map,rev_elts_map,xi = build_finite_field(q)
        X, X_ = get_X(q, xi,elts_map,rev_elts_map)
        H = get_H(q, xi,elts_map,rev_elts_map)
        self.M = get_M(H, q, n)
        self.clusters = [Cluster(q,a,p,self.M,X,k,elts_map,rev_elts_map) for k in range(n)]

        for i in range(n):
            for j in range(i+1,n):
                self.clusters[i].link(self.clusters[j])
    def __repr__(self):
        ret = ''
        for c in self.clusters:
            ret = ret + str(c)
        return ret

if __name__ == '__main__':
    n,q,a,p = [int(s) for s in sys.argv[1:]]
    galaxyfly = GalaxyFly(n,q,a,p)
    print(galaxyfly)

    # # for q in [3,5,7,9,11,13,17,19,23,25,29,31,37,64,139]:
    # for q in [41]:
    # # for i in range(10):
    # #     q = 4
    # #     while not quick_is_prime(q):
    # #         q = np.random.randint(2,200)
    #     n = 10
    #     a= 10
    #     p = 48-(a-1)-(n-1)-(q+1)//4*2
    #     print(' q ',q,' n ',n)
    #     xi = get_xi(q)
    #     X, X_ = get_X(q, xi)
    #     H = get_H(q, xi)
    #     print('xi', xi)
    #     print('X', X)
    #     print('X_', X_)
    #     print('H', H)
    #     M = get_M(H, q, n)
    #     for i in range(n):
    #         print('M{}'.format(i))
    #         print(M[i])
    #
    #     intra_edges = get_intra_cluster_edges(q, M, X,0)
    #     print('cluster0 内部连接方式',intra_edges)
    #
    #     inter_edges = get_inter_cluster_edges(q, M,0,1)
    #     print('cluster0 cluster1 之间连接方式',inter_edges)
    #
    #     dis , max_ij, max_super_degree =get_diameter(intra_edges,inter_edges ,q)
    #     print('supernode之间的最大距离 ',dis ,' supernode最大的度',  max_super_degree)
    #
    #     print('最大节点数',a*q*n*p)