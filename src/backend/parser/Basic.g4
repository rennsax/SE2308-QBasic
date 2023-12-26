grammar Basic;

/** lexer */
fragment LETTER: [a-zA-Z\u0080-\u{10FFFF}];

LET: 'LET';
GOTO: 'GOTO';
IF: 'IF';
THEN: 'THEN';
REM: 'REM';
PRINT: 'PRINT';
INPUT: 'INPUT';
END: 'END';
MOD: 'MOD';

ID: LETTER (LETTER | [0-9])*;
INT: [0-9] | [1-9][0-9]*;

POWER: '**';
MULT: '*';
DIV: '/';
PLUS: '+';
MINUS: '-';
GT: '>';
LT: '<';
EQUAL: '=';
LPAREN: '(';
RPAREN: ')';

// COMMENT: REM .*? NL -> skip;

WS: [ \t\r]+ -> skip;
NL: '\n';

/** parser */
main: prog EOF;

/** stm0: statement with the line number and a new line */
prog: stm0*;

/** Greedy: try to match a statement */
stm0:
	line_num stm NL // "\n" or "\r\n" is necessary
	| line_num REM .*? NL; // Match a comment.

line_num: INT # LineNum;

stm:
	let_stm
	| print_stm
	| input_stm
	| goto_stm
	| if_stm
	| end_stm;

empty_stm: NL;

let_stm: LET ID EQUAL expr # LetStm;
print_stm: PRINT expr # PrintStm;
input_stm: INPUT ID # InputStm;
goto_stm: GOTO INT # GotoStm;
cmp_op: EQUAL | LT | GT;
if_stm: IF expr cmp_op expr THEN INT # IfStm;
end_stm: END # EndStm;

expr:
	MINUS expr							# NegExpr
	| <assoc = right> expr POWER expr	# PowerExpr
	| expr MULT expr					# MultExpr
	| expr DIV expr						# DivExpr
	| expr MOD expr						# ModExpr
	| expr PLUS expr					# PlusExpr
	| expr MINUS expr					# MinusExpr
	| LPAREN expr RPAREN				# ParenExpr
	| INT								# IntExpr
	| ID								# VarExpr;