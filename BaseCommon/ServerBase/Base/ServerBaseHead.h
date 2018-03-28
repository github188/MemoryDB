


#ifndef ServerBase_Dll_Export
#	ifdef STATE_SERVERBASE_LIB
#		define ServerBase_Dll_Export
#		define ServerBase_Export_H
#	else
#		ifdef SERVERBASE_EXPORTS
#			define ServerBase_Dll_Export __declspec(dllexport)
#			define ServerBase_Export_H __declspec(dllexport)
#		else
#			define ServerBase_Dll_Export __declspec(dllimport)
#			define ServerBase_Export_H
#		endif
#	endif
#endif

#ifndef _INCLUDE_SERVERBASEHEAD_
#define _INCLUDE_SERVERBASEHEAD_


typedef void *  HANDLE;

//-------------------------------------------------------------------------
typedef	unsigned long SM_KEY;

#if defined(__LINUX__)
typedef		INT		SMHandle;
#elif defined(__WINDOWS__)
typedef		void*	SMHandle;
#endif
struct SMHead
{
	SM_KEY			m_Key;
	unsigned long	m_Size;
	unsigned int	m_HeadVer;//×îºó´æÅÌ°æ±¾
	SMHead()
	{
		m_Key		=	0;
		m_Size		=	0;
		m_HeadVer	=	0;
	}
};
//-------------------------------------------------------------------------
#endif