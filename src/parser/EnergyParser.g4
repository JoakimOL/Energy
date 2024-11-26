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

statement: variableDeclaration SEMICOLON
         | returnStatement SEMICOLON
         | expressionStatement SEMICOLON
         | ifStatement
         | block;

ifStatement: IFKEYWORD expression statement;
functionDeclaration: id parameterList RETURNARROW type;
functionDefinition: id parameterList ASSIGNMENT (block | statement);
functionCall: id argumentList;
variableDeclaration: type id ASSIGNMENT expression;
returnStatement: RETURNKEYWORD expression;

typeDefinition: NEWTYPE id '=' parameterList;

argumentList: '(' ')'
            | '(' args+=expression (COMMA args+=expression)* ')';

parameterList: '(' ')'
             | '(' params+=typedValue(COMMA params+=typedValue)* ')';

expressionStatement: expression;

indexingExpression: id INDEX id
                  | indexingExpression INDEX id
				  | functionCall INDEX id
;
 
block: '{' statement* '}';

typedValue: type id;
type: TYPENAME
    | id;
id: ID;
literal : INT
        | STRINGLITERAL;


binop : EQUALS
      | LESSTHAN
      | PLUS
      | MINUS
      | MUL
      | GREATERTHAN
	  ;

expression: id
          | literal
          | '(' expression ')'
          | expression binop expression
          | functionCall
		  | indexingExpression
		  ;
