import pygame as pg

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