#ifndef _INCLUDE_STRINGDATASTREAM_H_
#define _INCLUDE_STRINGDATASTREAM_H_



//class StringDataStream : public DataBuffer
//{
//public:
//	StringDataStream()
//	{
//	}
//
//	StringDataStream(DSIZE lenght)
//		: DataBuffer(lenght)
//	{
//
//	}
//
//	const char* GetString(){ return data(); }
//
//public:
//	bool write(const int nVal)
//	{
//		const char* sz = mStringTool.ToString(nVal);
//
//		return _write((void*)&nVal, sizeof(int)); 
//	}
//	bool write(const long lVal){ return _write((void*)&lVal, sizeof(long)); }
//	bool write(const float fVal){ return _write((void*)&fVal, sizeof(float)); }
//	bool write(const double dVal){ return _write((void*)&dVal, sizeof(double)); }
//	bool write(const short sVal){ return _write((void*)&sVal, sizeof(short)); }
//	bool write(const char cVal){ return _write((void*)&cVal, sizeof(char)); }
//	bool write(const bool bVal){ return _write((void*)&bVal, sizeof(bool)); }
//	bool write(const Int64 n64Val){ return _write((void*)&n64Val, sizeof(Int64)); }
//
//	bool write(const unsigned int nVal){ return _write((void*)&nVal, sizeof(unsigned int)); }
//	bool write(const unsigned long lVal){ return _write((void*)&lVal, sizeof(unsigned long)); }
//	bool write(const unsigned short sVal){ return _write((void*)&sVal, sizeof(unsigned short)); }
//	bool write(const unsigned char cVal){ return _write((void*)&cVal, sizeof(unsigned char)); }
//	bool write(const UInt64 n64Val){ return _write((void*)&n64Val, sizeof(UInt64)); }
//
//	bool write(const EasyString &str){ return writeString(str); }
//	bool write(const AString &str){ return writeString(str); }
//
//
//	bool read(int &nVal){ return _read(&nVal, sizeof(int)); }
//	bool read(long &lVal){ return _read(&lVal, sizeof(long)); }
//	bool read(float &fVal){ return _read(&fVal, sizeof(float)); }
//	bool read(double &dVal){ return _read(&dVal, sizeof(double)); }
//	bool read(short &sVal){ return _read(&sVal, sizeof(short)); }
//	bool read(char &cVal){ return _read(&cVal, sizeof(char)); }
//	bool read(bool &bVal){ return _read(&bVal, sizeof(bool)); }
//	bool read(Int64 &n64Val){ return _read(&n64Val, sizeof(Int64)); }
//
//	bool read(unsigned int &nVal){ return _read(&nVal, sizeof(unsigned int)); }
//	bool read(unsigned long &lVal){ return _read(&lVal, sizeof(unsigned long)); }
//	bool read(unsigned short &sVal){ return _read(&sVal, sizeof(unsigned short)); }
//	bool read(unsigned char &cVal){ return _read(&cVal, sizeof(unsigned char)); }
//	bool read(UInt64 &n64Val){ return _read(&n64Val, sizeof(UInt64)); }
//
//protected:
//	StringTool		mStringTool;
//};


#endif //_INCLUDE_STRINGDATASTREAM_H_