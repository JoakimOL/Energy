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
         | variableDeclaration SEMICOLON;

functionDeclaration: id parameterList '->' TYPENAME;
functionDefinition: id parameterList '=' block;
functionCall: id argumentList;
variableDeclaration: TYPENAME id '=' value;

argumentList: '(' ')'
            | '(' args+=value (COMMA args+=value)* ')';

parameterList: '(' ')'
             | '(' params+=typedValue(COMMA params+=typedValue)* ')';

block: statement
     | '{' statement* '}';

// typedValueList: typedValue
//               | typedValue, typedValueList;

typedValue: TYPENAME id;
id: ID;
value : id
      | INT
      | STRINGLITERAL;

