#ifndef _INCLUDE_TICKETCODE_H_
#define _INCLUDE_TICKETCODE_H_

#include "Auto.h"

//struct	TICKET_CODE
//{
//	TICKET_CODE()
//		: mTicketCode(0)
//	{
//
//	}
//
//	TICKET_CODE(UInt64 ticket)
//	{
//		mTicketCode = ticket;		
//	}
//
//	int GetTime(){ return (int)(mTicketCode>>32 & 0xFFFFFFFF); }
//	int GetCode(){ return (int)(mTicketCode & 0xFFFFFFFF); }
//
//	operator unsigned __int64 ()
//	{
//		return mTicketCode;
//	}
//
//protected:
//	unsigned __int64	mTicketCode;
//};

typedef UInt64		TICKET_CODE;

//-----------------------------------------------------------------------------------



#endif //_INCLUDE_TICKETCODE_H_