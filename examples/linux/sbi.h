/* ========================================================================== */
/*                                                                            */
/*   sbi.h                                                                    */
/*   (c) 2012 Gi@cky98                                                        */
/*                                                                            */
/* ========================================================================== */

#ifndef _SBI_H
	#define _SBI_H
	
	// Configuration
	#define VARIABLESNUM				64
	#define USERFUNCTIONSN				16
	#define RETURNADDRESSESN			16
	#define THREADMAXNUM				10
	
	// Multithreading configuration
	//	#define _SBI_MULTITHREADING_ENABLE
  		#define _SBI_MULTITHREADING_EQUALTIME
	
	// Define 'byte' variable type
	typedef unsigned char byte;
	
	// Define 'program counter' variable type
	typedef unsigned int PCOUNT;
	
	// Define 'label address' variable type
	typedef unsigned int LABEL;
	
	// Define 'return address' variable type
	typedef unsigned int RETADDR;
	
	// Define 'interrupt address' variable type
	typedef unsigned int INTERRUPT;
	
	// Define 'user function id' variable type
	typedef unsigned int USERFUNCID;
	
	// Define 'sbi thread number' variable type
	typedef unsigned int SBITHREADNUM;
	
	// Define 'program variable' variable type
	typedef unsigned char VARIABLE;
	
	// SBI format
	#define HEADER_0						0xAA
	#define HEADER_1						0x4B // 0x3B, 0x2B or 0x1B for older versions
	
	#define LABELSECTION						0xA3
	#define INTERRUPTSECTION					0xB3
	
	#define SEPARATOR						0xB7
	
	#define FOOTER_0						0x3A
	#define FOOTER_1						0xF0
	
	// Instructions
	#define _istr_assign						0x01
	#define _istr_move						0x02
	#define _istr_add						0x10
	#define _istr_sub						0x11
	#define _istr_mul						0x12
	#define _istr_div						0x13
	#define _istr_incr						0x20
	#define _istr_decr						0x21
	#define _istr_inv						0x22
	#define _istr_tob						0x23
	#define _istr_cmp						0x30
	#define _istr_high						0x31
	#define _istr_low						0x32
	#define _istr_jump						0x41
	#define _istr_cmpjump						0x42
	#define	_istr_ret						0x43
	#define _istr_debug						0x50
	#define _istr_error						0x52
	#define _istr_sint						0x60
	#define _istr_int						0x61
	#define _istr_exit						0xFF
	
	#define _varid							0x04
	#define _value							0xF4
	
	#ifdef _SBI_MULTITHREADING_ENABLE
		// SBI multithreading eums
		typedef enum
		{
			ERROR = 0,
			STOPPED,
			RUNNING
		} SBITHREADSTATUS;
		
		typedef enum
		{
			NOERROR = 0,
			REACHEDEND,
			EXITED,
			WRONGINSTR,
			WRONGBYTE,
			USERERROR
		} SBITHREADERROR;
		
		// SBI structures
		typedef struct
		{
			byte (*_getfch)(PCOUNT*);
			PCOUNT p;
		} SBISTREAM;
		
		typedef struct
		{
			SBISTREAM* stream;
			SBITHREADSTATUS status;
			SBITHREADERROR _lasterror;
			VARIABLE _t[VARIABLESNUM];
			LABEL* _labels;
			INTERRUPT* _interrupts;
			RETADDR _returnaddresses[RETURNADDRESSESN];
			USERFUNCID _userfid;
		} SBITHREAD;
		
		// Multithreading variables
		extern SBITHREAD* _sbi_threads[THREADMAXNUM];
		extern SBITHREAD* _sbi_currentthread;
		#ifdef _SBI_MULTITHREADING_EQUALTIME
			SBITHREADNUM _sbi_currentthreadn;
		#endif
	#else
		extern PCOUNT p;
	#endif
	
	// User functions
	extern void (*_sbifuncs[USERFUNCTIONSN])(byte[]);
	
	#ifndef _SBI_MULTITHREADING_ENABLE
		byte (*_getfch)(void);
	#endif
	
	byte _getval(const byte type, const byte val);
	unsigned int _setval(const byte type, const byte num, const byte val);
	
	void _sbi_init(void);
	
	void _interrupt(const INTERRUPT id);
	
	#ifndef _SBI_MULTITHREADING_ENABLE
		unsigned int _sbi_begin(void);
		unsigned int _sbi_step(void);
	#endif
	
	#ifdef _SBI_MULTITHREADING_ENABLE
		SBISTREAM* _sbi_createstream(byte (*_getfch)(PCOUNT*));
		
		SBITHREAD* _sbi_createthread(SBISTREAM* stream);
		unsigned int _sbi_loadthread(SBITHREAD* thread);
		void _sbi_removethread(SBITHREAD* thread);
		
		unsigned int _sbi_step(SBITHREAD* thread);
		unsigned int _sbi_stepall(void);
		
		unsigned int _sbi_running(void);
		
		SBITHREAD* _sbi_getthread(SBITHREADNUM n);
		
		void _sbi_startthread(SBITHREAD* thread);
		void _sbi_stopthread(SBITHREAD* thread);
		SBITHREADSTATUS _sbi_getthreadstatus(SBITHREAD* thread);
		SBITHREADERROR _sbi_getthreaderror(SBITHREAD* thread);
	#endif
	
	
	
#endif
