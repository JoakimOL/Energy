parser grammar TestParser;

options {
	tokenVocab = TestLexer;
}

// These are all supported parser sections:

// Parser file header. Appears at the top in all parser related files. Use e.g. for copyrights.
@parser::header {/* parser/listener/visitor header section */}

// Appears before any #include in h + cpp files.
@parser::preinclude {/* parser precinclude section */}

// Follows directly after the standard #includes in h + cpp files.
@parser::postinclude {
/* parser postinclude section */
#pragma GCC diagnostic ignored "-Wunused-parameter"
}

// Directly preceeds the parser class declaration in the h file (e.g. for additional types etc.).
@parser::context {/* parser context section */}

// Appears in the private part of the parser in the h file.
@parser::members { }

// Appears in the public part of the parser in the h file.
@parser::declarations {/* private parser declarations section */}

// Appears in line with the other class member definitions in the cpp file.
@parser::definitions {/* parser definitions section */}

@parser::visitorpreinclude {/* visitor preinclude section */}
@parser::visitorpostinclude {/* visitor postinclude section */}
@parser::visitordeclarations {/* visitor public declarations/members section */}
@parser::visitormembers {/* visitor private declarations/members section */}
@parser::visitordefinitions {/* visitor definitions section */}

@parser::basevisitorpreinclude {/* base visitor preinclude section */}
@parser::basevisitorpostinclude {/* base visitor postinclude section */}
@parser::basevisitordeclarations {/* base visitor public declarations/members section */}
@parser::basevisitormembers {/* base visitor private declarations/members section */}
@parser::basevisitordefinitions {/* base visitor definitions section */}

// Actual grammar start.
// main: thing+ EOF;
// thing: id
//     | id integer
// ;
// id: ID;
// integer: INT;

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

