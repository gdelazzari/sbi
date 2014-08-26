/* ========================================================================== */
/*                                                                            */
/*   sbi.c                                                                    */
/*   (c) 2012 Gi@cky98                                                        */
/*                                                                            */
/* ========================================================================== */

#include "sbi.h"

#include <stdlib.h>

// function macros for easy access 
#define _debug(d)				ctx->debugn(d,ctx)
#define _error(d)				ctx->errorn(d,ctx)
#define _getfch()               ctx->getfch(ctx)
#define _setfpos(p)             ctx->setfpos(p,ctx)
#define _getfpos(p)             ctx->getfpos(ctx)


// Variables, labels and subroutines
byte _t[VARIABLESNUM];
unsigned int* _labels;
unsigned int _returnaddresses[RETURNADDRESSESN];

// Interrupts
unsigned int* _interrupts;
unsigned int _exec = 0;
unsigned int _intinqueue = 0;
unsigned int _queuedint = 0;

// User functions
void (*_sbifuncs[USERFUNCTIONSN])(byte[]);
unsigned int _userfid = 0;

/*
	Gets the value of a parameter
 */
byte _getval(const byte type, const byte val)
{
	if (type==_varid) return _t[val]; else return val;
}

/*
	Sets the value of a variable
	Useful for user functions
		Return:
			0: 	All ok
			1: 	The specified parameter
			is not a variable
 */
unsigned int _setval(const byte type, const byte num, const byte val) 
{
	if (type==_varid)
	{
		_t[num]=val;
		return 0;
	} else {
		return 1;
	}
}

/*
	Common variables
 */
byte rd;
unsigned int var1t;
unsigned int var1;
unsigned int var2t;
unsigned int var2;
unsigned int var3t;
unsigned int var3;
byte b[16];
unsigned int i;

/*
	Initializes the interpreter
 */
void _sbi_init(sbi_context_t* c)
{
    // reserve for future init uses.
}

/*
	Begins program execution
 */
unsigned int _sbi_begin(sbi_context_t *ctx) 		// Returns:
												// 					0: 	No errors
												//					1: 	No function pointers for _getfch,
												//							_setfpos and _getfpos
												//					2:	Old version of executable format
												//					3:	Invalid program file
{
	// Check function pointers
	if ((ctx->getfch==0)||(ctx->setfpos==0)||(ctx->getfpos==0)) return 1;
	
	// Read head
	rd = _getfch();
	if (rd!=HEADER_0) return 3;
	rd = _getfch();
	if (rd!=HEADER_1) {
		if ((rd==0x1B)||(rd==0x2B)||(rd==0x3B))
			return 2;
		else
			return 3;
	}
	
	// Getting labels
	if (_getfch()!=LABELSECTION) return 3;
	unsigned int ln = _getfch();
	_labels = (unsigned int*)malloc(ln * sizeof(int));
	unsigned int c = 0;
	while (ln--)
	{
    _labels[c] = _getfch() | (_getfch() << 8);
    c++;
	}
	if (_getfch()!=SEPARATOR) return 3;
	
	// Getting interrupts addresses
	if (_getfch()!=INTERRUPTSECTION) return 3;
	ln = _getfch();
	_interrupts = (unsigned int*)malloc(ln + ln); //ln * sizeof(unsigned int) -> ln * 2 -> ln+ln
	c = 0;
	while (ln--)
	{
    _interrupts[c] = _getfch() | (_getfch() << 8);
    c++;
	}
	if (_getfch()!=SEPARATOR) return 3;
	
	// Done
	return 0;
}

/*
	Executes the program
 */
unsigned int _sbi_run(sbi_context_t *ctx)       // Runs a SBI program
												// Returns:
												// 					0: 	No errors
												// 					1: 	Reached end (no exit found)
												//					2: 	Program exited
												//					3: 	Wrong instruction code
												//					4: 	Can't understand byte
												//					5: 	User error
{
	_exec = 1;
	
	rd = _getfch();
	switch (rd)
	{
		case _istr_assign:
			var1 = _getfch();
			_t[var1] = _getfch();
			break;
		case _istr_move:
			var1 = _getfch();
			_t[var1] = _t[_getfch()];
			break;
		case _istr_add:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			_t[_getfch()] = _getval(var1t, var1) + _getval(var2t, var2);
			break;
		case _istr_sub:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			_t[_getfch()] = _getval(var1t, var1) - _getval(var2t, var2);
			break;
		case _istr_mul:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			_t[_getfch()] = _getval(var1t, var1) * _getval(var2t, var2);
			break;
		case _istr_div:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			_t[_getfch()] = _getval(var1t, var1) / _getval(var2t, var2);
			break;
		case _istr_incr:
			_t[_getfch()]++;
			break;
		case _istr_decr:
			_t[_getfch()]--;
			break;
		case _istr_inv:
			var1 = _getfch();
			if (_t[var1]==0) _t[var1]=1; else _t[var1]=0;
			break;
		case _istr_tob:
			var1 = _getfch();
			if (_t[var1]>0) _t[var1]=1; else _t[var1]=0;
			break;
		case _istr_cmp:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			if (_getval(var1t, var1)==_getval(var2t, var2)) _t[_getfch()]=1; else _t[_getfch()]=0;
			break;
		case _istr_high:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			if (_getval(var1t, var1)>_getval(var2t, var2)) _t[_getfch()]=1; else _t[_getfch()]=0;
			break;
		case _istr_low:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			if (_getval(var1t, var1)<_getval(var2t, var2)) _t[_getfch()]=1; else _t[_getfch()]=0;
			break;
		case _istr_jump:
			var1t = _getfch();
			var1 = _getfch();
			if (_getfch() > 0)
			{
				for (i=RETURNADDRESSESN-2; i>0; i--) _returnaddresses[i+1] = _returnaddresses[i];
				_returnaddresses[1] = _returnaddresses[0];
				_returnaddresses[0] = _getfpos();
			}
			_setfpos(_labels[_getval(var1t, var1)]);
			break;
		case _istr_cmpjump:
			var1t = _getfch();
			var1 = _getfch();
			var2t = _getfch();
			var2 = _getfch();
			var3t = _getfch();
			var3 = _getfch();
			if (_getfch() > 0)
			{
				for (i=RETURNADDRESSESN-2; i>0; i--) _returnaddresses[i+1] = _returnaddresses[i];
				_returnaddresses[1] = _returnaddresses[0];
				_returnaddresses[0] = _getfpos();
			}
			if (_getval(var1t, var1)==_getval(var2t, var2))
			{
				_setfpos(_labels[_getval(var3t, var3)]);
			}
			break;
		case _istr_ret:
			_setfpos(_returnaddresses[0]);
			for (i=1; i<RETURNADDRESSESN; i++) _returnaddresses[i-1] = _returnaddresses[i];
			break;
		case _istr_debug:
			var1t = _getfch();
			_debug(_getval(var1t, _getfch()));
			break;
		case _istr_error:
			var1t = _getfch();
			_error(_getval(var1t, _getfch()));
			return 5;
			break;
		case _istr_sint:
			var1t = _getfch();
			_userfid=_getval(var1t, _getfch());
			break;
		case _istr_int:
			for (i=0; i<16; i++) b[i] = _getfch();
			_sbifuncs[_userfid](b);
			break;
		case _istr_exit:
			return 2;
			break;
		case FOOTER_0:
			if (_getfch()==FOOTER_1) return 1; else return 4;
		default:
			_error(0xB1);
			return 3;
			break;
	}
	
	_exec = 0;
	
	if (_intinqueue==1) _interrupt(_queuedint, ctx); // If there are interrupts in
																							// in the queue, do it
	
	return 0;
}

void _interrupt(const unsigned int id, sbi_context_t* ctx)
{
	if (_exec==1) // Some code in execution, queue interrupt
	{
		_intinqueue = 1;
		_queuedint = id;
		return;
	}
	
	for (i=RETURNADDRESSESN-2; i>0; i--) _returnaddresses[i+1] = _returnaddresses[i];
	_returnaddresses[1] = _returnaddresses[0];
	_returnaddresses[0] = _getfpos();
	
	_setfpos(_interrupts[id]); // Set the program counter to interrupt's address
	
	_intinqueue = 0; // Be sure to clean the queue
	
	return;
}
