%{

#include "build.h"
#include "textFilePreprocessorExpressionParser.h"

#ifdef PLATFORM_MSVC
	#pragma warning ( disable: 4702 ) //: unreachable code
#endif

static int tfpe_lex(bm::TextFilePreprocessorExpressionNode* outElem, bm::TextFilePreprocessorExpressionContext& ctx)
{
    return ctx.readToken(*outElem);
}

static int tfpe_error(bm::TextFilePreprocessorExpressionContext& ctx, const char* txt)
{
    ctx.reportError(bm::TempString("parser error: {}", txt));
    return 0;
}

%}

%require "2.7"
%defines
%define api.pure full
%define api.prefix {tfpe_};
%define api.value.type {bm::TextFilePreprocessorExpressionNode}
%define parse.error verbose

%parse-param { bm::TextFilePreprocessorExpressionContext& ctx }
%lex-param { bm::TextFilePreprocessorExpressionContext& ctx }

%token TOKEN_NUMBER

%token TOKEN_LEFT_OP
%token TOKEN_RIGHT_OP
%token TOKEN_LE_OP
%token TOKEN_GE_OP
%token TOKEN_EQ_OP
%token TOKEN_NE_OP
%token TOKEN_AND_OP
%token TOKEN_OR_OP

%% /* The grammar follows.  */

result
	: expression { ctx.writeResult($1.value); }
	;

expression
	: conditional_expression
	;

primary_expression
    : TOKEN_NUMBER { }
	| '(' expression ')' { $$ = $2; }
	;

unary_expression
	: primary_expression
    | '+' unary_expression {  }
    | '-' unary_expression { $$.value = -$2.value; }
    | '~' unary_expression { $$.value = ~$2.value; }
    | '!' unary_expression { $$.value = ($2.value == 0) ? 1 : 0; }
	;

multiplicative_expression
	: unary_expression
	| multiplicative_expression '*' unary_expression { $$.value = $1.value * $3.value; }
	| multiplicative_expression '/' unary_expression { $$.value = $3.value ? ($1.value / $3.value) : 0; } 
	| multiplicative_expression '%' unary_expression { $$.value = $3.value ? ($1.value % $3.value) : 0; } 
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression { $$.value = $1.value + $3.value; }
	| additive_expression '-' multiplicative_expression { $$.value = $1.value - $3.value; }
	;

shift_expression
	: additive_expression
	| shift_expression TOKEN_LEFT_OP additive_expression { $$.value = $1.value << $3.value; }
	| shift_expression TOKEN_RIGHT_OP additive_expression { $$.value = $1.value >> $3.value; }
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression { $$.value = ($1.value < $3.value) ? 1 : 0; }
    | relational_expression '>' shift_expression { $$.value = ($1.value > $3.value) ? 1 : 0; }
	| relational_expression TOKEN_LE_OP shift_expression { $$.value = ($1.value <= $3.value) ? 1 : 0; }
	| relational_expression TOKEN_GE_OP shift_expression { $$.value = ($1.value >= $3.value) ? 1 : 0; }
	;

equality_expression
	: relational_expression
	| equality_expression TOKEN_EQ_OP relational_expression { $$.value = ($1.value == $3.value) ? 1 : 0; }
	| equality_expression TOKEN_NE_OP relational_expression { $$.value = ($1.value != $3.value) ? 1 : 0; }
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression { $$.value = $1.value & $3.value; }
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression { $$.value = $1.value ^ $3.value; }
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression { $$.value = $1.value | $3.value; }
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression TOKEN_AND_OP inclusive_or_expression { $$.value = $1.value && $3.value; }

logical_or_expression
	: logical_and_expression
	| logical_or_expression TOKEN_OR_OP logical_and_expression { $$.value = $1.value || $3.value; }
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression { $$.value = $1.value ? $3.value : $5.value; }
	;

%%
