#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ , UEQ , logical_AND , logical_OR,
	logical_NOT , Dec_integer , Register, Variable ,Hex,
	Eip
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// space
	{"\\+", '+'},					// plus
	{"-",'-'},					//subtraction
	{"\\*",'*'},					//multiplication
	{"/",'/'},					//division
	{"==", EQ},					// equal
	{"!=", UEQ},					//unequal
	{"&&",logical_AND},				//logical and
	{"\\|\\|",logical_OR},				//logical or
	{"!",logical_NOT},				//logical not
	{"\\(",'('},					//left parenthesis
	{"\\)",')'},                   			//right parenthesis
	{"[0-9]{1,10}",Dec_integer},			//decimal integer
	{"\\$[a-dA-D][hlHL]|\\$[eE]?(ax|dx|cx|bx|bp|si|di|sp)",Register}, //register
	{"[a_zA_Z_][a-zA-Z0-9]*",Variable},		//variable
	{"0[xX][A-Fa-f0-9]{1,8}",Hex},			//hex
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
						case 257:
							tokens[nr_token].type = 257 ;
							strcpy(tokens[nr_token].str , "==");
							break;
                                                case 258:
                                                        tokens[nr_token].type = 258 ;
                                                        strcpy(tokens[nr_token].str , "!=");
                                                        break;
                                                case 259:
                                                        tokens[nr_token].type = 259 ;
                                                        strcpy(tokens[nr_token].str , "&&");
                                                        break;
                                                case 260:
                                                        tokens[nr_token].type = 260 ;
                                                        strcpy(tokens[nr_token].str , "||");
                                                        break;
                                                case 261:
                                                        tokens[nr_token].type = 261 ;
                                                        strcpy(tokens[nr_token].str , "!");
                                                        break;
                                                case 262:	//Die_integer
                                                        tokens[nr_token].type = 262 ;
                                                        break;
                                                case 263:	//Register
                                                        tokens[nr_token].type = 263 ;
                                                        strncpy(tokens[nr_token].str , &e[position-substr_len] , substr_len);
                                                        break;
                                                case 264:	//Variable
                                                        tokens[nr_token].type = 264 ;
                                                        strncpy(tokens[nr_token].str , &e[position-substr_len] , substr_len);
                                                        break;
                                                case 265:	//Hex
                                                        tokens[nr_token].type = 265 ;
                                                        strncpy(tokens[nr_token].str , &e[position-substr_len] , substr_len);
                                                        break;
                                                case 266:	//Eip
                                                        tokens[nr_token].type = 266 ;
                                                        strncpy(tokens[nr_token].str , &e[position-substr_len] , substr_len);
                                                        break;
                                                case 40:
                                                        tokens[nr_token].type = 40 ;	//'('
                                                        break;
                                                case 41:
                                                        tokens[nr_token].type = 41 ;	//')'
                                                        break;
                                                case 42:
                                                        tokens[nr_token].type = 42 ;	//'*'
                                                        break;
                                                case 43:
                                                        tokens[nr_token].type = 43 ;	//'+'
                                                        break;
                                                case 45:
                                                        tokens[nr_token].type = 45 ;	//'-'	
                                                        break;
                                                case 47:
                                                        tokens[nr_token].type = 47 ;	//'/'
                                                        break;
					default: panic("please implement me");
				}
				nr_token ++;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	
	return true; 
}

bool check_parentheses(int m ,int n){
        int left = 0;
        int flag = 0;
        if(tokens[m].type == 40){
                left ++;
                int i;
                for(i = m+1 ; i<=n ; i++){
                        if(tokens[i].type == 40){
                                left ++ ;
                        }
                        else if(tokens[i].type == 41){
                                left -- ;
                                if(left==0 && i != n)
                                        flag = 1;
                                if(left < 0)
                                        assert(0);
                        }
        }
                if(left == 0 && flag != 1 && tokens[n].type ==41)
                        return 1;
                else if(left == 0)
                        return 0;
                else
                        assert(0);
}
        else
                return 0;

}

int dominant_operator(int p ,int q ){
	int theop = p;
	int j = p;
	int m = 0;
	for( ; j<=q ;j++){
		if(tokens[j].type<257 && tokens[j].type != '!'){
			if(tokens[j].type == 40){
				m++;
				for(j=j+1 ; tokens[j].type != 41 || m!=1 ;j++){
					if(tokens[j].type == 40)	m++;
					if(tokens[j].type == 41)	m--;
					}
					m = 0;

}
}
}
	return theop;
}


/* int eval(int p ,int q){
//	int i=0;
	if(p > q )	assert(0);
//	else if(p == q){
//		if(tokens[p].type)
//}
	else{
		if(strcmp(tokens[p].str+1 , "al")==0)
			return reg_b(0);
		if(strcmp(tokens[p].str+1 , "cl")==0)
                        return reg_b(1);
		if(strcmp(tokens[p].str+1 , "dl")==0)
                        return reg_b(2);
		if(strcmp(tokens[p].str+1 , "bl")==0)
                        return reg_b(3);
		if(strcmp(tokens[p].str+1 , "ah")==0)
                        return reg_b(4);
		if(strcmp(tokens[p].str+1 , "ch")==0)
                        return reg_b(5);
		if(strcmp(tokens[p].str+1 , "dh")==0)
                        return reg_b(6);
		if(strcmp(tokens[p].str+1 , "bh")==0)
                        return reg_b(7);


}
//	if(j == 8)	assert(0)

*/

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

