class Node:
    pass


class BinNode(Node):
    def __init__(self):
        self.lhs = None
        self.rhs = None


class BlockNode(Node):
    def __init__(self):
        self.text_block = None
        self.code_block = None
        self.ref_block = None


class ExprNode(Node):
    def __init__(self):
        self.assign_expr = None


class AssignExprNode(Node):
    def __init__(self):
        self.assignable_operand = None
        self.assign_operator = None
        self.operand = None


class AssignableOperandNode(Node):
    def __init__(self):
        self.identifier = None


class AssignOperatorNode(Node):
    def __init__(self):
        self.operator = None


class OperandNode(Node):
    def __init__(self):
        self.identifier = None
        self.string = None


class RefBlockNode(Node):
    def __init__(self):
        self.identifier = None


class CodeBlockNode(Node):
    def __init__(self):
        self.formula = None


class TextBlockNode(Node):
    def __init__(self):
        self.text = None


class FormulaNode(Node):
    def __init__(self):
        self.expr = None
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

