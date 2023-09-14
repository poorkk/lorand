class Node(object):
    def __init__(self, parent, name) -> None:
        self.parent = parent
        self.name = name
        self.child = []
    
    def add_child(self, name):
        chi = Node(self, name)
        self.child.append(chi)
        return chi
    
    def travl(self):
        for c in self.child:
            c.travl()
        print(self.name)

g = Node(None, 'yeye')
b = g.add_child('baba')
b.add_child('bo')
b.add_child('kun')

g.travl()

