#include "dso.h"
#include "oled.h"
#include "delay.h"
//交换a和b
#ifndef _swap_char
#define _swap_char(a, b)     \
    {                        \
        u8 t = a; \
        a = b;               \
        b = t;               \
    }
#endif

u8 point[128];   //用于储存128个转速值
extern float Frequency_vale;

u8 dsopsc;       //用于控制示波器速度

		
		
//绘制两点线段至缓存
//t=1绘制线段绘制线段至缓存
//t=0清除线段绘制线段至缓存
void OLED_DrawLine(u8 x0, u8 y0, u8 x1, u8 y1,u8 t)
{
    signed char dx, dy, ystep;
    int err;
    u8 swapxy = 0;

    if (x0 > 127)
        x0 = 127;
    if (y0 > 63)
        y0 = 63;
    if (x1 > 127)
        x1 = 127;
    if (y1 > 63)
        y1 = 63;

    dx = abs(x1 - x0);//求绝对值
    dy = abs(y1 - y0);

    if (dy > dx)
    {
        swapxy = 1;
        _swap_char(dx, dy);
        _swap_char(x0, y0);
        _swap_char(x1, y1);
    }

    if (x0 > x1)
    {
        _swap_char(x0, x1);
        _swap_char(y0, y1);
    }
    err = dx >> 1;

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
        if (swapxy == 0)
            OLED_DrawPoint(x0, y0,t);
        else
            OLED_DrawPoint(y0, x0,t);
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void DSO_play(void)
{
	u8 x;
	if(dsopsc==1)   //用于控制示波器速度
	{
		
		point[127]=63-(int)(Frequency_vale*63.0/130.0);
		dsopsc=0;
		for(x=20;x<127;x++)
		{
			point[x]=point[x+1];              //把数组值向左移动
			OLED_DrawPoint(x,point[x],1);     //把显示过的点向左移动
			OLED_DrawPoint(x+1,point[x+1],0); //清除显示过的显存点
		}
		
		for(x=20;x<127;x++)
		{
			OLED_DrawLine(x,point[x],x+1,point[x+1],1);
			OLED_DrawLine(x+1,point[x],x+2,point[x+1],0);
		}
		
		OLED_ShowNum(0,0,(int)Frequency_vale,3,12);
		OLED_Refresh_Gram();
	}
}
 
