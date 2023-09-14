# 准备：
#   pip insall pygame

import pygame as pg
import math

from kk_global import *
from kk_event import *
from kk_object import *

class Kgame(object):
    def __init__(self) -> None:
        self.cfg = Config()
        self.clock = pg.time.Clock()
    
    def event(self):
        for event in pg.event.get():
            eve = Event()
            print(event)
            if event == pg.QUIT:
                eve.type = EXIT
                eve.press = 'Q'
            elif event.type == pg.KEYDOWN: # 按下键盘
                eve.type= KEY_DOWN
                for e in EVENT:
                    if event.key == e[0]:
                        eve.press = e[1]
            elif event.type == pg.KEYUP: # 弹起键盘
                eve.type = KEY_UP
            elif event.type == pg.MOUSEBUTTONDOWN:
                eve.type = MOUS_DOWN
                eve.pos = event.pos
                if event.button == 1:
                    eve.press = 'L'
                elif event.button == 3:
                    eve.press = 'R'
            print(eve)
            return eve

    # 操作图形 https://xymgf.blog.csdn.net/article/details/118313215 pg.Rect.move(r, [10, 10])
    def mrect(self): 
        r = pg.Rect(200, 200, 100, 100)

    def loop(self):
        temp = TempNode(self.cfg)
        
        run = True
        self.cfg.SCREEN.fill([255, 255, 255])

        root = temp.root_node()
        xy = temp.xy_node(None)
        
        frame = root.add('frame_game', RECT, [100, 100, 400, 400, 'blue', 0])
        sk = frame.add('sk', CIRCLE, [100, 100, 50, 'red'])


        root.load()
        xy.load()

        # loop
        obj = sk.obj
        while run:
            eve = self.event()
            if eve == None:
                continue
            print(eve)
            if eve.type == KEY_DOWN:
                pre = eve.press
                if pre == 'Q':
                    run = False
                elif pre == 'L':
                    obj.x -= 10
                elif pre == 'R':
                    obj.x += 10
                elif pre == 'U':
                    obj.y -= 10
                elif pre == 'D':
                    obj.y += 10
            elif eve.type == MOUS_DOWN:
                pos = eve.pos
                x = pos[0]
                y = pos[1]
            
            root.load()
            pg.display.update()
            self.clock.tick(60)

K = Kgame()
K.loop()