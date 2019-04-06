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
        self.if_ = None
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


class IfNode(Node):
    def __init__(self):
        self.expr = None
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
