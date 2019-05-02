class Node:
    pass


class BlockNode(Node):
    def __init__(self):
        self.text_block = None
        self.code_block = None
        self.ref_block = None
        self.block = None


class ExprNode(Node):
    def __init__(self):
        self.assign_expr = None
        self.digit = None


class AssignExprNode(Node):
    def __init__(self):
        self.ideitfier = None # str
        self.assign_operator = None # str
        self.assign_expr = None # Node
        self.expr = None # Node


class AssignOperandLhsNode(Node):
    def __init__(self):
        self.identifier = None


class AssignOperatorNode(Node):
    def __init__(self):
        self.operator = None


class AssignOperandRhsNode(Node):
    def __init__(self):
        self.expr = None
        self.string = None
        self.identifier = None
        self.callable = None


class RefBlockNode(Node):
    def __init__(self):
        self.identifier = None
        self.callable = None


class CodeBlockNode(Node):
    def __init__(self):
        self.formula = None


class TextBlockNode(Node):
    def __init__(self):
        self.text = None


class FormulaNode(Node):
    def __init__(self):
        self.comparison = None
        self.assign_expr = None
        self.if_ = None
        self.import_ = None
        self.caller = None
        self.formula = None
        self.block = None


class ImportNode(Node):
    def __init__(self):
        self.identifier = None


class CallerNode(Node):
    def __init__(self):
        self.identifiers = []
        self.args = []


class ComparisonNode(Node):
    def __init__(self):
        self.expr = None
        self.op = None
        self.comparison = None


class IfNode(Node):
    def __init__(self):
        self.comparison = None
        self.block = None
        self.formula = None
        self.elif_ = None
        self.else_ = None


class ElseNode(Node):
    def __init__(self):
        self.block = None
        self.formula = None


class DigitNode(Node):
    def __init__(self):
        self.value = None


class ExprNode(Node):
    def __init__(self):
        self.term = None
        self.op = None
        self.expr = None


class TermNode(Node):
    def __init__(self):
        self.factor = None
        self.op = None
        self.term = None


class FactorNode(Node):
    def __init__(self):
        self.digit = None
        self.identifier = None
        self.string = None
        self.expr = None
        self.callable = None
        self.assign_expr = None
        self.id_expr = None


class IdExprNode(Node):
    def __init__(self):
        self.front_or_back = None # str. 'front' to front, 'back' to back
        self.identifier = None # str
        self.operator = None # str


class CallableNode(Node):
    def __init__(self):
        self.caller_list = None
        self.args = None


class CallerListNode(Node):
    def __init__(self):
        self.identifier = None
        self.caller_list = None


class ArgsNode(Node):
    def __init__(self):
        self.arg = None
        self.args = None


class ArgNode(Node):
    def __init__(self):
        self.digit = None
        self.string = None
        self.identifier = None

