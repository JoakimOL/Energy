parser grammar EnergyParser;

options {
	tokenVocab = EnergyLexer;
}

@parser::header {
#pragma GCC diagnostic ignored "-Wattributes"
}

program: toplevel+ EOF;

toplevel: functionDeclaration
        | functionDefinition
        | typeDefinition;

statement: functionCall SEMICOLON
         | variableDeclaration SEMICOLON
         | returnStatement SEMICOLON
         | block;

functionDeclaration: id parameterList '->' TYPENAME;
functionDefinition: id parameterList '=' block | statement;
functionCall: id argumentList;
variableDeclaration: TYPENAME id '=' expression;
returnStatement: RETURNKEYWORD expression;

typeDefinition: NEWTYPE id '=' parameterList;

argumentList: '(' ')'
            | '(' args+=expression (COMMA args+=expression)* ')';

parameterList: '(' ')'
             | '(' params+=typedValue(COMMA params+=typedValue)* ')';

block: '{' statement* '}';

// typedValueList: typedValue
//               | typedValue, typedValueList;

typedValue: TYPENAME id;
id: ID;
literal : INT
        | STRINGLITERAL;

expression: id
          | literal
          | '(' expression ')';
