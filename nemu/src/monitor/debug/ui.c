#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	int step = 0;
	char *arg = strtok(NULL," ");
	if (arg == NULL)	step =1;
	else	sscanf(arg , "%d" , &step);
	if(step == -1)		cpu_exec(-1);
	if(step < -1)	printf("Wrong input\n");
	if(step > -1)
	while(step--)
	cpu_exec(1);
	return 0;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL," ");
	if(*arg == 'r'){
		printf("eax 0x%x\n",cpu.eax);
		printf("ecx 0x%x\n",cpu.ecx);
		printf("edx 0x%x\n",cpu.edx);
		printf("ebx 0x%x\n",cpu.ebx);
		printf("esp 0x%x\n",cpu.esp);
		printf("ebp 0x%x\n",cpu.ebp);
		printf("esi 0x%x\n",cpu.esi);
		printf("edi 0x%x\n",cpu.edi);
	}
	return 0;	
}

static int cmd_x(char *args){
	char *arg1 = strtok(NULL," ");
	char *arg2 = strtok(NULL," ");
	int len ;
	int address;
//	lnaddr_t address;
	sscanf(arg1 , "%d" ,&len);
	sscanf(arg2 , "%x" ,&address);
//	printf("0x:%x\n",address);
//	int i;
//	for(i = 0; i<len ;i++){
//		printf("0x%x\n",lnaddr_read(address,4));
//		address+=4;
//	}
//		return 0;
	
	if(len !=0){
		printf("%#x: ", address);
		int i,j;
		for( i=0 ; i<len ;i++){
			for(j=0 ;j<4;j++){
				printf("%02X ",swaddr_read(address+4*i+j,1));
}
}
		printf("\n");
}	
		return 0;

}


static int cmd_p(char *args)
{
	bool success;
	int i;
	i=expr(args,&success);
	printf("%d\n",i);
	return 0;
}

/*static int cmd_w(char *args)
{
	new_wp(args);
	return 0;
}

static int cmd_d(char *args)
{
	int i;
	sscanf(args,"%d",&i);
	free_wp(i);
	return 0;
}

*/
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si", "Single step instructions", cmd_si },
	{ "info","Print register status" , cmd_info },
	{ "x" ,"scan memory" , cmd_x },
	{ "p", "p EXPR means evaluation of expression", cmd_p },
      //  {"w","Setting the monitoring point",cmd_w},
       // {"d","Delete monitoring point",cmd_d}, 
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
