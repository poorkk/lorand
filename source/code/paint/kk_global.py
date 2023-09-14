import pygame as pg

class Config(object):
    def __init__(self) -> None:
        self.SCREEN = None
        self.WIDTH = 800
        self.HIGH = 800
        self.init()

    def init(self):
        pg.init()
        pg.display.set_caption("kkgame")
        self.SCREEN = pg.display.set_mode((self.WIDTH, self.HIGH))