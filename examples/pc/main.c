#include <stdio.h>
#include <sbi.h>

#include "pclib.h"


int main(int argc, char** argv)
{
	printf("\nSBI Runtime for Windows PC\n\n");
	
	// Open executable
	if (argc!=2) { printf("Wrong arguments\n\nUse:\n %s <program.sbi>\n", argv[0]); return 8; }
	printf("Loading %s...\n", argv[1]);
	f = fopen((char*)argv[1], "rb");
	
	if (!f) { printf("Can't open file!\n"); return 1; }

	// Init
    sbi_context_t ctx;

	ctx.getfch=getfch;
	ctx.setfpos=setfpos;
	ctx.getfpos=getfpos;
    ctx.debugn=debugn;
    ctx.errorn=errorn;
    ctx.sbi_user_funcs[0] = myfunc;
    ctx.sbi_user_funcs[1] = msgbox; 
    ctx.sbi_user_funcs[2] = getnum;

	void *v = _sbi_init(&ctx, 0);
	
	int ret = _sbi_begin(v);
	if (ret==1) printf("Initialization error (no function pointers)\n");
	if (ret==2) printf("Initialization error (old format version)\n");
	if (ret==3) printf("Initialization error (invalid program file)\n");
	if (ret>3) printf("Initialization error (unknow: %i)\n", ret);
	
	if (ret>0) return 1;
	
	printf("Running...\n");
	
	while (ret==0)
	{
		ret = _sbi_run(v);
		//if (_kbhit()) // Key press interrupt
		//{
		//	_interrupt(2);
		//	getch();
		//}
	}
	
	fclose(f);

    _sbi_cleanup(v);
	
	if (ret==1) printf("Program reached end (no exit found)\n");
	if (ret==2) printf("Program exited (no errors)\n");
	if (ret==3) printf("Program exited (wrong instruction code)\n");
	if (ret==4) printf("Program exited (can't understand byte)\n");
	if (ret==5) printf("Program exited (user error)\n");
	
	if (ret<2) return 0; else return 1;
}
