#include <stdio.h>
#include "sbi.h"

FILE* f;
byte lastslot=0;
byte buffer[32];

byte getfch(void)
{
	byte slot=((int)(p/sizeof(buffer)));
	byte i;
	if (slot!=lastslot)
	{
		fseek (f, slot*sizeof(buffer), SEEK_SET);
		for (i=0; i<sizeof(buffer); i++) buffer[i]=fgetc(f);
	}
	lastslot=slot;
	return buffer[(p++)-(lastslot*sizeof(buffer))];
}

int main(int argc, char** argv)
{
	printf("\nSBI Runtime for Linux\n\n");
	
	// Open executable
	if (argc!=2) { printf("Wrong arguments\n\nUse:\n %s <program.sbi>\n", argv[0]); return 8; }
	printf("Loading %s...\n", argv[1]);
	f = fopen((char*)argv[1], "rb");
	
	if (!f) { printf("Can't open file!\n"); return 1; }
	
	// Init
	_getfch=&getfch;

	byte i;
	for (i=0; i<sizeof(buffer); i++) buffer[i]=fgetc(f);
	_sbi_init();
	
	int ret = _sbi_begin();
	if (ret==1) printf("Initialization error (no function pointers)\n");
	if (ret==2) printf("Initialization error (old format version)\n");
	if (ret==3) printf("Initialization error (invalid program file)\n");
	if (ret>3) printf("Initialization error (unknow: %i)\n", ret);
	
	if (ret>0) return 1;
	
	printf("Running...\n");
	
	while (ret==0)
	{
		ret = _sbi_step();
	}
	
	fclose(f);
	
	if (ret==1) printf("Program reached end (no exit found)\n");
	if (ret==2) printf("Program exited (no errors)\n");
	if (ret==3) printf("Program exited (wrong instruction code)\n");
	if (ret==4) printf("Program exited (can't understand byte)\n");
	if (ret==5) printf("Program exited (user error)\n");
	
	if (ret<2) return 0; else return 1;
}
