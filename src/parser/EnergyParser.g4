parser grammar EnergyParser;

options {
	tokenVocab = EnergyLexer;
}

@parser::header {
#pragma GCC diagnostic ignored "-Wattributes"
}

program: statement+ EOF;

statement: functionDeclaration
         | functionDefinition
         | functionCall SEMICOLON
         | variableDeclaration SEMICOLON
         | returnStatement SEMICOLON;

functionDeclaration: id parameterList '->' TYPENAME;
functionDefinition: id parameterList '=' block;
functionCall: id argumentList;
variableDeclaration: TYPENAME id '=' expression;
returnStatement: RETURNKEYWORD expression;

argumentList: '(' ')'
            | '(' args+=expression (COMMA args+=expression)* ')';

parameterList: '(' ')'
             | '(' params+=typedValue(COMMA params+=typedValue)* ')';

block: statement
     | '{' statement* '}';

// typedValueList: typedValue
//               | typedValue, typedValueList;

typedValue: TYPENAME id;
id: ID;
literal : INT
        | STRINGLITERAL;

expression: id
          | literal
          | '(' expression ')';
