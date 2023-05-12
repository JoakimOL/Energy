lexer grammar EnergyLexer;

@lexer::header {
#pragma GCC diagnostic ignored "-Wattributes"
}

TYPENAME: 'int'
        | 'string'
        | LISTOPEN TYPENAME LISTCLOSE;
RETURNARROW: '->';
PARENOPEN: '(';
PARENCLOSE: ')';
BLOCKOPEN: '{';
BLOCKCLOSE: '}';
LISTOPEN: '[';
LISTCLOSE: ']';
COMMA: ',';
EQUALS: '==';
ASSIGNMENT: '=';
LESSTHAN: '<';
GREATERTHAN: '>';
PLUS: '+';
MINUS: '-';
MUL: '*';
STRINGLITERAL: '"' ~'"'* '"';
SEMICOLON: ';';
RETURNKEYWORD: 'return';
IFKEYWORD: 'if';

INT: Digit+;
Digit: [0-9];

ID: LETTER (LETTER | '0'..'9' | [._-])*;
fragment LETTER : [a-zA-Z\u0080-\uFFFF];

WS: [ \t]+ -> skip;
LINETERMINATOR: [\r\n] -> channel(HIDDEN);
// Comments
Comment: '//' CommentLine* -> skip;
fragment CommentLine: ~ [\\\r\n];
