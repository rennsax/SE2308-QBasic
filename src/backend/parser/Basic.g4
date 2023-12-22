grammar Basic;

/** lexer */
fragment LETTER : [a-zA-Z\u0080-\u{10FFFF}];

LET : 'LET' ;
GOTO : 'GOTO' ;
IF : 'IF' ;
THEN : 'THEN' ;
REM : 'REM' ;
PRINT : 'PRINT' ;
INPUT : 'INPUT' ;
END : 'END' ;
MOD : 'MOD' ;

ID : LETTER (LETTER | [0-9])* ;
INT : [1-9][0-9]* ;

POWER : '**' ;
MULT : '*' ;
DIV : '/' ;
PLUS : '+' ;
MINUS : '-' ;
GT : '>' ;
LT : '<' ;
EQUAL : '=' ;
LPAREN: '(' ;
RPAREN: ')' ;

COMMENT : REM .*? NL -> skip ;

WS : [ \t\r]+ -> skip ;
NL : '\n' ;

/** parser */
main : prog EOF ;

/** stm0: statement with a new line */
prog : stm0* ;

/** Greedy: try to match a statement */
stm0 : stm NL? // If stm exists, NL is optional (last statement).
    | NL ; // Match an empty line.

stm : let_stm
    | print_stm
    | input_stm
    | goto_stm
    | if_stm
    | end_stm
;

empty_stm : NL ;
let_stm : LET ID EQUAL expr ;
print_stm : PRINT expr ;
input_stm : INPUT ID ;
goto_stm : GOTO INT ;
cmp_op : EQUAL | LT | GT ;
if_stm : IF expr cmp_op expr THEN INT ;
end_stm : END ;

expr
    : MINUS expr
    | <assoc=right> expr POWER expr
    | expr MULT expr
    | expr DIV expr
    | expr MOD expr
    | expr PLUS expr
    | expr MINUS expr
    | LPAREN expr RPAREN
    | INT
    | ID
    ;

empty_line: NL ;