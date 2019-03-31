class Node:
    pass


class BinNode(Node):
    def __init__(self):
        self.lhs = None
        self.rhs = None


class CodeBlockNode(Node):
    def __init__(self):
        self.formula = None


class TextBlockNode(Node):
    def __init__(self):
        self.text = None


class FormulaNode(Node):
    def __init__(self):
        self.import_ = None
        self.caller = None
        self.formula = None


class ImportNode(Node):
    def __init__(self):
        self.identifier = None


class CallerNode(Node):
    def __init__(self):
        self.identifiers = []
        self.args = []

