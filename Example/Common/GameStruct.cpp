#include "GameStruct.h"
#include <stdlib.h>
#include "DataStream.h"
#include <windows.h>

// a*level^c+d*level+f
float CaculateValue(const char* str, int level)
{
	//char a[20],b[20],c[20],d[20],e[20],f[20];
	//sscanf(str,"%[^*]*%[^^]^%[^+]+%[^*]*%[^+]+%s",a,b,c,d,e,f);
	//return (float)(pow((double)level, atoi(c)) * atof(a) + level * atof(d) + atof(f));
	char a[20] = {0};
	char b[20] = {0};
	char c[20] = {0};
	char d[20] = {0};
	char e[20] = {0};
	char f[20] = {0};
	char g[20] = {0};
	char h[20] = {0};
	char i[20] = {0};
	if (sscanf(str,"%[^*]*%[^^]^%[^+]+%[^*]*%[^^]^%[^+]+%[^*]*%[^+]+%s",a,b,c,d,e,f,g,h,i)!=9)
		ERROR_LOG("CaculateValue >%s, error >%u", str, GetLastError());	
	return (float)(pow((double)level, atoi(c)) * atof(a) + pow((double)level, atoi(f)) * atof(d) + level * atof(g) + atof(i));
}

int ToWorldBattleLevel(const BattleLevelConfig &config, int starCount, int &levelStar)
{
	for (int i=0; i<config.size()-1; ++i)
	{
		int x = starCount-config[i];
		if (x<0)
		{
			levelStar = starCount;
			return i+1;
		}
		starCount = x;
	}
	levelStar = starCount;
	return (int)config.size();
}
