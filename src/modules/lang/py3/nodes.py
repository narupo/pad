from context import Context


class Node:
    pass


class BlockNode(Node):
    def __init__(self):
        self.text_block = None
        self.code_block = None
        self.ref_block = None
        self.block = None
        self.syms = {} # symbol table


class ExprLineNode(Node):
    def __init__(self):
        self.expr_list = None # Node
        self.newline = None # str


class ExprListNode(Node):
    def __init__(self):
        self.expr = None # Node
        self.expr_list = None # Node


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
        self.expr = None


class CodeBlockNode(Node):
    def __init__(self):
        self.formula = None


class TextBlockNode(Node):
    def __init__(self):
        self.text = None


class ForStmtNode(Node):
    def __init__(self):
        self.init_expr_list = None # Node
        self.comp_expr = None # Node
        self.update_expr_list = None # Node
        self.block = None # Node
        self.formula = None # Node


class AssignStmtLineNode(Node):
    def __init__(self):
        self.assign_stmt = None # Node


class AssignStmtNode(Node):
    def __init__(self):
        self.identifier_list = None # Node
        self.expr = None # Node


class IdentifierListNode(Node):
    def __init__(self):
        self.identifier = None # str
        self.identifier_list = None # Node

    def to_list(self):
        identifiers = []
        if self.identifier:
            identifiers.append(self.identifier)
        self._to_list(self.identifier_list, identifiers)
        return identifiers

    def _to_list(self, identifier_list, identifiers):
        if identifier_list is None:
            return
        if identifier_list.identifier:
            identifiers.append(identifier_list.identifier)
        self._to_list(identifier_list.identifier_list, identifiers)


class DefFuncNode(Node):
    def __init__(self):
        self.identifier = None # str
        self.dmy_args = None # Node
        self.formula = None # Node
        self.syms = {} # symbol table


class DmyArgs:
    def __init__(self):
        self.dmy_arg = None # Node
        self.dmy_args = None # Node

    def dmy_args_to_list(self):
        lis = []
        if self.dmy_arg:
            lis.append(self.dmy_arg.identifier)
        self._dmy_args_to_list(self.dmy_args, lis)
        return lis

    def _dmy_args_to_list(self, dmy_args, lis):
        if dmy_args is None:
            return
        if dmy_args.dmy_arg:
            lis.append(dmy_args.dmy_arg.identifier)
        self._dmy_args_to_list(dmy_args.dmy_args, lis)


class DmyArg:
    def __init__(self):
        self.identifier = None # str


class ReturnStmtNode(Node):
    def __init__(self):
        self.expr_line = None # Node


class FormulaNode(Node):
    def __init__(self):
        self.expr_line = None # Node
        self.assign_expr = None # Node
        self.assign_stmt_line = None # Node
        self.if_stmt = None # Node
        self.for_stmt = None # Node
        self.return_ = None # Node
        self.def_func = None # Node
        self.import_stmt = None # Node
        self.formula = None # Node
        self.block = None # Node
        self.call_stmt = None # Node
        self.newline = None # str


class ImportStmtNode(Node):
    def __init__(self):
        self.identifier = None


class CallerNode(Node):
    def __init__(self):
        self.identifiers = []
        self.args = []


class IfStmtNode(Node):
    def __init__(self):
        self.expr = None
        self.block = None
        self.formula = None
        self.elif_stmt = None
        self.else_stmt = None


class ElseStmtNode(Node):
    def __init__(self):
        self.block = None
        self.formula = None


class DigitNode(Node):
    def __init__(self):
        self.value = None


class ExprNode(Node):
    def __init__(self):
        self.gorasu = None
        self.op = None
        self.expr = None


class GorasuNode(Node):
    def __init__(self):
        self.kamiyu = None # Node
        self.op = None # str
        self.gorasu = None # Node


class KamiyuNode(Node):
    def __init__(self):
        self.mod = None # Node
        self.op = None # str
        self.kamiyu = None # Node


class ModNode(Node):
    def __init__(self):
        self.term = None # Node
        self.op = None # str
        self.mod = None # Node


class TermNode(Node):
    def __init__(self):
        self.kamiyu = None # Node
        self.op = None # str
        self.term = None # Node


class FactorNode(Node):
    def __init__(self):
        self.digit = None # int
        self.identifier = None # str
        self.string = None # str
        self.expr = None # Node
        self.callable = None # Node
        self.assign_expr = None # Node
        self.id_expr = None # Node
        self.not_expr = None # Node


class NotExprNode(Node):
    def __init__(self):
        self.expr = None # Node


class IdExprNode(Node):
    def __init__(self):
        self.front_or_back = None # str. 'front' to front, 'back' to back
        self.identifier = None # str
        self.operator = None # str


class CallableNode(Node):
    def __init__(self):
        self.name_list = None # Node
        self.args = None # Node
        self.syms = {} # symbol table


class NameListNode(Node):
    def __init__(self):
        self.identifier = None # str
        self.name_list = None # Node


class ArgsNode(Node):
    def __init__(self):
        self.arg = None # Node
        self.args = None # Node


class ArgNode(Node):
    def __init__(self):
        self.expr = None # Node

