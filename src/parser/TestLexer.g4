lexer grammar TestLexer;

@lexer::header {
#pragma GCC diagnostic ignored "-Wattributes"
}

TYPENAME: 'i8'
        | 'string'
        | '[' TYPENAME ']';
RETURNARROW: '->';
PARENOPEN: '(';
PARENCLOSE: ')';
BLOCKOPEN: '{';
BLOCKCLOSE: '}';
COMMA: ',';
EQUALS: '=';
STRINGLITERAL: '"' ~'"'* '"';
SEMICOLON: ';';

INT: Digit+;
Digit: [0-9];

ID: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z\u0080-\uFFFF];

WS: [ \t]+ -> skip;
LINETERMINATOR: [\r\n] -> channel(HIDDEN);
