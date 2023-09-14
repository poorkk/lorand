from kk_global import *

'''
客户端 c | 服务端 s | kms k
c->s: 1. 初始化
        s->k: 2. 连接
            k->c: 3. 返回成功
c: 4. 结束
'''

'''
      c                s              k
    [     ][      ][      ][      ][      ][      ]

idx     1     1_2     2      2_3      3      3_
t   (c,0,m) (_,0,m) (s,0,m) (_,0,m) (k,0,m) (_,0,m)

c            (初,0,l)
c                           (连,0,l)
c                           (返,0,r)
c            (结,0,l)

l   (c, s, 1, 0)

'''

class Title(object):
    def __init__(self, txt, alia, len) -> None:
        self.txt = txt
        self.alia = alia
        self.len = len
        self.x = 0
        self.y = 0

class Column(object):
    def __init__(self) -> None:
        self.txt = None
        self.dir = 'm'
        self.x = 0
        self.y = 0

class Line(object):
    def __init__(self) -> None:
        self.left = 0
        self.right = 0
        self.dir = '->'

class Table(object):
    def __init__(self) -> None:
        self.content = []

        self.title = []
        self.column = []
        self.line = []
    
    def idx(self, idx):
        if '_' in idx:
            start = idx.split('_')[0]
            return start * 2
        else:
            return (int(idx) - 1) * 2 + 1
    
    def parse_title(self, title): # 客户端 c | 服务端 s | kms k
        tit = title.split('|')
        for tt in tit:
            ttl = tt.split(' ')
            self.title.append(ttl[0], ttl[1] if len(ttl) > 1 else None, len(ttl[0]))
            self.title.append(None, None, 1)
        
        for 

    def parse_column(self, col): # c->s: 1. 初始化
        

    def parse(self):
        self.parse_title(self.content[0])
        for i in range(1, len(self.content)):
            