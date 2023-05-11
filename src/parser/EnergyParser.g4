parser grammar EnergyParser;

options {
	tokenVocab = EnergyLexer;
}

@parser::header {
#pragma GCC diagnostic ignored "-Wattributes"
}

program: toplevel+ EOF;

toplevel: functionDeclaration
        | functionDefinition;

statement: variableDeclaration SEMICOLON
         | returnStatement SEMICOLON
         | ifStatement
         | block;

ifStatement: IFKEYWORD expression statement;
functionDeclaration: id parameterList '->' TYPENAME;
functionDefinition: id parameterList ASSIGNMENT (block | statement);
functionCall: id argumentList;
variableDeclaration: TYPENAME id ASSIGNMENT expression;
returnStatement: RETURNKEYWORD expression;

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

binop : EQUALS
      | LESSTHAN
      | PLUS
      | MINUS
      | MUL
      | GREATERTHAN;

expression: id
          | literal
          | '(' expression ')'
          | expression binop expression
          | functionCall;
