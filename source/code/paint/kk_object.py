import pygame as pg

from kk_global import *

RECT = 1
CIRCLE = 2
LINE = 3
LINES = 4
TEXT = 5

class Obj(object):
    def __init__(self, cfg, name, type, args) -> None:        
        self.cfg = cfg
        self.name = name
        self.x = 0
        self.y = 0

        self.obj = None
        self.type = None # 1-矩形 2-圆形 3-线段

        self.color = None
        self.pgcolor = pg.Color('green')
        self.bgcolor = None
        self.border = 1
        
        self.txtojb = None

        self.wid = 0 # 矩形
        self.hig = 0

        self.r = 0 # 圆

        self.ex = 0 # 线段
        self.ey = 0

        self.points = [] # 多线段
        self.po = True # 线段闭环

        self.txt = '' # 文本
        self.font = None

        self.obj_init(type, args)

    # 绘制图形：画出后，不会消失 https://xymgf.blog.csdn.net/article/details/118313663
    def obj_init(self, type, args):
        self.type = type
        t = self.type
        n = len(args)
        if t == RECT: # 矩形 [横坐标、纵坐标、宽、高、颜色、边框]
            if n > 0:
                self.x, self.y, self.wid, self.hig = args[0], args[1], args[2], args[3]
                if n >= 5:
                    self.color = args[4]
                    self.pgcolor = pg.Color(self.color)
                    if n >= 6:
                        self.border = args[5]
        elif t == CIRCLE: # 圆 [横坐标、纵坐标、半径、颜色、边框]
            if n > 0:
                self.x, self.y, self.r = args[0], args[1], args[2]
                if n >= 4:
                    self.color = args[3]
                    self.pgcolor = pg.Color(self.color)
                    if n >= 5:
                        self.border = args[4]
            
        elif t == LINE: # 线 [起始x、起始y、终止x、终止y、颜色、边框]
            if n > 0:
                self.x, self.y, self.ex, self.ey = args[0], args[1], args[2], args[3]
                if n >= 5:
                    self.color = args[4]
                    self.pgcolor = pg.Color(self.color)
                    if n >= 6:
                        self.border = args[5]
        elif t == LINES: # 连续多条线 [[点坐标]、闭环、颜色、边框]
            if n > 0:
                self.points = args[0]
                if n >= 2:
                    self.po = args[1]
                    if n >= 3:
                        self.color = args[2]
                        self.pgcolor = pg.Color(self.color)
                        if n >= 4:
                            self.border = args[3]
        elif t == TEXT: # 文本 [横坐标、纵坐标、文本、大小、颜色、背景颜色]
            # fonts = pg.font.get_fonts()
            self.r = 20
            if n > 0:
                self.x, self.y, self.txt = args[0], args[1], args[2]
                if n >= 4:
                    self.r = args[3]
                if n >= 5:
                    self.color = args[4]
                    self.pgcolor = args[4]
                    if n >= 6:
                        self.bgcolor = args[5]
            self.font = pg.font.SysFont('宋体', self.r, bold=False, italic =False) # 加粗、斜体

    def load(self):
        print("load", self.name)
        t = self.type
        if t == RECT:
            self.obj = pg.draw.rect(self.cfg.SCREEN, self.pgcolor, (self.x, self.y, self.wid, self.hig), self.border)
        elif t == CIRCLE:
            self.obj = pg.draw.circle(self.cfg.SCREEN, self.pgcolor, (self.x, self.y), self.r, self.border)
        elif t == LINE: # aaline
            self.obj = pg.draw.line(self.cfg.SCREEN, self.pgcolor, (self.x, self.y), (self.ex, self.ey), self.border)
        elif t == LINES:
            self.obj = pg.draw.aalines(self.cfg.SCREEN, self.pgcolor, self.po, self.points, self.border)
        elif t == TEXT:
            self.txtojb = self.font.render(self.txt, True, self.pgcolor, self.bgcolor) # 平滑字体、背景
            self.obj = self.txtojb.get_rect()
            self.obj.center = (self.x, self.y)
            self.cfg.SCREEN.blit(self.txtojb, self.obj)

    def update(self, txt):
        if self.type == TEXT:
            self.txt = txt
        self.load()

class Node(object):
    def __init__(self, cfg, parent, name, type, args) -> None:
        self.cfg = cfg
        
        self.parent = parent # Node
        self.obj = Obj(cfg, name, type, args)
        self.child = [] # Node
    
    def add(self, name, type, args):
        cnode = Node(self.cfg, self, name, type, args)
        cobj = cnode.obj
        cobj.x += self.obj.x
        cobj.y += self.obj.y
        self.child.append(cnode)
        return cnode

    def load(self):
        self.obj.load()
        for cnode in self.child:
            cnode.load()

class TempNode(object):
    def __init__(self, cfg) -> None:
        self.cfg = cfg

    def xy_node(self, parent):
        node = Node(self.cfg, parent, 'frame_xy', RECT, [0, 0, self.cfg.WIDTH, self.cfg.HIGH])
        for x in range(0, self.cfg.WIDTH, 50): # | | |
            yline = node.add('x-', LINE, [x, 0, x, self.cfg.HIGH, 'grey', 1])
        for y in range(0, self.cfg.HIGH, 50): # =
            xline = node.add('y-', LINE, [0, y, self.cfg.WIDTH, y, 'grey', 1])
        return node
    
    def root_node(self):
        root = Node(self.cfg, None, 'root', RECT, [0, 0, self.cfg.WIDTH, self.cfg.HIGH, 'white'])
        return root