# 准备：
#   pip insall pygame

import pygame as pg
import math

EXIT = 0
KEY_DOWN = 1
KEY_UP = 2
MOUS_DOWN = 3
MOUS_UP = 4

class Event(object):
    def __init__(self) -> None:
        self.type = KEY_DOWN
        self.press = 'I'
        self.pos = None

EVENT = [ # https://blog.csdn.net/weixin_45020839/article/details/117886708
    [None, 'N'],
    [pg.K_LEFT, 'L'],  # left
    [pg.K_RIGHT, 'R'], # right
    [pg.K_UP, 'U'],    # up
    [pg.K_DOWN, 'D'],  # down
    
    [pg.K_SPACE, 'S'], # space
    [pg.K_ESCAPE, 'Q'],
    
    [pg.K_a, 'a'],
    [pg.K_b, 'b'],
    [pg.K_c, 'c'],
    [pg.K_d, 'd'],
    [pg.K_e, 'e'],
    [pg.K_f, 'f'],
    [pg.K_g, 'g'],
    [pg.K_h, 'h'],
    [pg.K_i, 'i'],
    [pg.K_j, 'j'],
    [pg.K_k, 'k'],
    [pg.K_l, 'l'],
    [pg.K_m, 'm'],
    [pg.K_n, 'n'],
    [pg.K_o, 'o'],
    [pg.K_p, 'p'],
    [pg.K_q, 'q'],
    [pg.K_r, 'r'],
    [pg.K_s, 's'],
    [pg.K_t, 't'],
    [pg.K_u, 'u'],
    [pg.K_v, 'v'],
    [pg.K_w, 'w'],
    [pg.K_x, 'x'],
    [pg.K_y, 'y'],
    [pg.K_z, 'z'],
]

RECT = 1
CIRCLE = 2
LINE = 3
LINES = 4
TEXT = 5

class Obj(object):
    def __init__(self, screen, type, args) -> None:
        self.screen = screen
        self.obj = None
        self.type = None # 1-矩形 2-圆形 3-线段
        self.x = 0
        self.y = 0
        self.wid = 0 # 矩形
        self.hig = 0
        self.r = 0 # 圆
        self.ex = 0 # 线段
        self.ey = 0
        self.points = [] # 多线段
        self.po = True # 线段闭环
        self.color = pg.Color('green')
        self.bgcolor = None
        self.border = 1
        self.txtojb = None
        self.txt = ''
        self.font = None
        self.set(type, args)

    def draw(self):
        t = self.type
        if t == RECT:
            self.obj = pg.draw.rect(self.screen, self.color, (self.x, self.y, self.wid, self.hig), self.border)
        elif t == CIRCLE:
            self.obj = pg.draw.circle(self.screen, self.color, (self.x, self.y), self.r, self.border)
        elif t == LINE: # aaline
            self.obj = pg.draw.line(self.screen, self.color, (self.x, self.y), (self.ex, self.ey), self.border)
        elif t == LINES:
            self.obj = pg.draw.aalines(self.screen, self.color, self.po, self.points, self.border)
        elif t == TEXT:
            self.txtojb = self.font.render(self.txt, True, self.color, self.bgcolor) # 平滑字体、背景
            self.obj = self.txtojb.get_rect()
            self.obj.center = (self.x, self.y)
            self.screen.blit(self.txtojb, self.obj)

    def update(self, txt):
        if self.type == TEXT:
            self.txt = txt
        self.draw()

    # 绘制图形：画出后，不会消失 https://xymgf.blog.csdn.net/article/details/118313663
    def set(self, type, args):
        self.type = type
        t = self.type
        n = len(args)
        if t == RECT: # 矩形 [横坐标、纵坐标、宽、高、颜色、边框]
            if n > 0:
                self.x, self.y, self.wid, self.hig = args[0], args[1], args[2], args[3]
                if n >= 5:
                    self.color = pg.Color(args[4])
                    if n >= 6:
                        self.border = args[5]
        elif t == CIRCLE: # 圆 [横坐标、纵坐标、半径、颜色、边框]
            if n > 0:
                self.x, self.y, self.r = args[0], args[1], args[2]
                if n >= 4:
                    self.color = pg.Color(args[3])
                    if n >= 5:
                        self.border = args[4]
            
        elif t == LINE: # 线 [起始x、起始y、终止x、终止y、颜色、边框]
            if n > 0:
                self.x, self.y, self.ex, self.ey = args[0], args[1], args[2], args[3]
                if n >= 5:
                    self.color = pg.Color(args[4])
                    if n >= 6:
                        self.border = args[5]
        elif t == LINES: # 连续多条线 [[点坐标]、闭环、颜色、边框]
            if n > 0:
                self.points = args[0]
                if n >= 2:
                    self.po = args[1]
                    if n >= 3:
                        self.color = pg.Color(args[2])
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
                    if n >= 6:
                        self.bgcolor = args[5]
            self.font = pg.font.SysFont('宋体', self.r, bold=False, italic =False) # 加粗、斜体

class ObjGroup(object):
    def __init__(self, name) -> None:
        self.name = name

class Kgame(object):
    def __init__(self) -> None:
        self.screen = None
        self.clock = None
        self.wid = 800
        self.hig = 800
    
    def init(self):
        pg.init()
        pg.display.set_caption("kkgame")
        self.screen = pg.display.set_mode((self.wid, self.hig))
        self.clock = pg.time.Clock()

    def new(self, type, args):
        return Obj(self.screen, type, args)

    def event(self):
        eo = Event()
        for event in pg.event.get():
            print(event)
            if event == pg.QUIT:
                eo.type = EXIT
                eo.press = 'Q'
            elif event.type == pg.KEYDOWN: # 按下键盘
                eo.type= KEY_DOWN
                for e in EVENT:
                    if event.key == e[0]:
                        eo.press = e[1]
            elif event.type == pg.KEYUP: # 弹起键盘
                eo.type = KEY_UP
            elif event.type == pg.MOUSEBUTTONDOWN:
                eo.type = MOUS_DOWN
                eo.pos = event.pos
                if event.button == 1:
                    eo.press = 'L'
                elif event.button == 3:
                    eo.press = 'R'
            print(eo)
            return eo
        return eo

    def draw_func(self):
        p = []
        for x in range(0, 100):
            y = int(0.2 * x  * x) - 2 * x + 20
            p.append([x, y])
        print(p)
        l = self.new(LINES, [p, False])
        l.draw()

    def show_xy(self):
        self.lines = []
        if len(self.lines) == 0:
            for x in range(0, self.wid, 50): # | | |
                yl = self.new(LINE, [x, 0, x, self.hig, 'grey', 1])
                self.lines.append(yl)
            for y in range(0, self.hig, 50): # =
                xl = self.new(LINE, [0, y, self.wid, y, 'grey', 1])
                self.lines.append(xl)
        for l in self.lines:
            l.draw()
   
    def update_text(self, txt):
        self.t.txt = txt;
        self.t.draw()

    # 操作图形 https://xymgf.blog.csdn.net/article/details/118313215 pg.Rect.move(r, [10, 10])
    def mrect(self): 
        r = pg.Rect(200, 200, 100, 100)

    def loop(self):
        run = True
        ojbs = []
        r = self.new(RECT, [200, 200, 100, 100])
        t = self.new(TEXT, [100, 100, 'shenkun', 20, 'red'])

        # init        
        self.screen.fill([255, 255, 255])
        self.show_xy()
        r.draw()
        pg.display.update()

        # loop
        while run:
            self.screen.fill([255, 255, 255])
            self.show_xy()
            eo = self.event()
            if eo.type == KEY_DOWN:
                pre = eo.press
                if pre == 'Q':
                    run = False
                elif pre == 'L':
                    r.x -= 10
                    r.draw()
                    t.update('left')
                    pg.display.update()
                elif pre == 'R':
                    r.x += 10
                    r.draw()
                    t.update('right')
                    pg.display.update()
                elif pre == 'U':
                    r.y -= 10
                    r.draw()
                    t.update('up')
                    pg.display.update()
                elif pre == 'D':
                    r.y += 10
                    r.draw()
                    t.update('down')
                    pg.display.update()
            elif eo.type == MOUS_DOWN:
                pos = eo.pos
                r.x = pos[0]
                r.y = pos[1]
                if eo.press == 'L':
                    r.color = 'red'
                else:
                    r.color = 'blue'
                r.draw()
                pg.display.update()
            self.clock.tick(60)
K = Kgame()
K.init()
K.loop()