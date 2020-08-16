#include "key.h"
#include "sys.h"
#include "oled.h"
#include "24cxx.h"

unsigned char keysta[4][4]={
 {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};
const unsigned char keycodemap[4][4] = {
	{ 1, 4, 7,  0},
	{ 2, 5, 8, 'e' },
	{ 3, 6, 9, 'n' },
	{ 'u', 'l', 'd', 'r' }
};
extern float Kp;
extern float KI;
extern float KD;
extern float cycle;     //波形周期
extern u8 max,min;      //波峰，波谷
extern float Targetspeed;     //目标转速
extern u8 page,yici,storbit,directionbit;   //页面索引值page，刷新一次页面索引值yici，储存索引值storbit，改变转向索引值directionbit
static unsigned char strspeed[4]={0,6,0};   //速度数组
static unsigned char str1[4]={0,8,0},str2[4]={0,0,0,2},str3[4];  //PID数组依次为十位，个位，小数点（为零），十分位，用于更改PID
u8 sw=0;   //选择索引值选中显示'<'，并分别给page1界面的各个值选择赋值

void EEROM_SpeedPIDRead(void)
{
	unsigned char test;
	AT24CXX_Init();
	test=AT24CXX_ReadOneByte(0);//如果第一个空间不是'R'说明该设备是新设备
	if(test=='R')
	{
		AT24CXX_Read(1,strspeed,3);
		AT24CXX_Read(4,str1,4);
		AT24CXX_Read(8,str2,4);
		AT24CXX_Read(12,str3,4);
		Targetspeed=strspeed[0]*100+strspeed[1]*10+strspeed[2];   //读取EEPROM
		Kp=str1[0]*10+str1[1]+str1[3]*0.1;
		KI=str2[0]*10+str2[1]+str2[3]*0.1;
		KD=str3[0]*10+str3[1]+str3[3]*0.1;
	}
	else    //如果是新设备，在第一位空间写R标记此设备，并写入初始化数据
	{
		AT24CXX_WriteOneByte(0,'R');  
		AT24CXX_Write(1,strspeed,3);
		AT24CXX_Write(4,str1,4);
		AT24CXX_Write(8,str2,4);
		AT24CXX_Write(12,str3,4);
	}
}

//当按键触发用于储存所有值
void StorAllData(void)  
{
	AT24CXX_Write(1,strspeed,3);
	AT24CXX_Write(4,str1,4);
	AT24CXX_Write(8,str2,4);
	AT24CXX_Write(12,str3,4);
}

//portB的高八位用于驱动矩阵按键
void KEY_Init(void) 
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);//使能PORTB时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	 GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	 GPIO_SetBits(GPIOB,GPIO_Pin_12);						 

	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	    		
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	  			
	 GPIO_SetBits(GPIOB,GPIO_Pin_13); 					
	 
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;	    	
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	  			
	 GPIO_SetBits(GPIOB,GPIO_Pin_14); 						
	 
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	    		
	 GPIO_Init(GPIOB, &GPIO_InitStructure);	  			
	 GPIO_SetBits(GPIOB,GPIO_Pin_15); 
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭 jtag，使能 SWD，可以用 SWD 模式调试，需要使能AFIO  PA15,PA12用于控制电机方向
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_15);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}

//按键扫描函数，在TIM4中断中执行
void keyscan(void)
{
	unsigned char i;
	static unsigned char keyout=0;
	static unsigned char keybuf[4][4]={
	{0xff,0xff,0xff,0xff},{0xff,0xff,0xff,0xff},
	{0xff,0xff,0xff,0xff},{0xff,0xff,0xff,0xff}
	};
	keyout=keyout&0x03;
	switch(keyout)
	{
		case 0: GPIOB->ODR|=0x7f00;PBout(15)=0;break;
		case 1: GPIOB->ODR|=0xbf00;PBout(14)=0;break;
		case 2: GPIOB->ODR|=0xdf00;PBout(13)=0;break;
		case 3: GPIOB->ODR|=0xef00;PBout(12)=0;break;
		default:break;
	}
	keybuf[keyout][0]=(keybuf[keyout][0]<<1)|PBin(11);
	keybuf[keyout][1]=(keybuf[keyout][1]<<1)|PBin(10);
	keybuf[keyout][2]=(keybuf[keyout][2]<<1)|PBin(9);
	keybuf[keyout][3]=(keybuf[keyout][3]<<1)|PBin(8);
	for(i=0;i<4;i++)
	{
		if((keybuf[keyout][i]&0x0f)==0x00)
		{
			keysta[keyout][i]=0;
		}
		else if((keybuf[keyout][i]&0x0f)==0x0f)
		{
			keysta[keyout][i]=1;
		}
	}
	keyout++;
	
}


void keydriver(void)//抬手执行
{
	unsigned  char i=0, j=0;
	static unsigned char backup[4][4] = {
	{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}
	};
	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			if (backup[i][j] !=keysta[i][j])
			{
				if (backup[i][j] == 0)
				{
					keyaction(keycodemap[i][j]);
				}
				backup[i][j] = keysta[i][j];
			}
		}
	}
}

//在page1界面更改targetspeed的按键函数
void gaispeed(unsigned char i)
{
	static unsigned char steap=0;
	if(i!=' ')
	{
		if(steap==0)
		{
			OLED_ShowString(80,0,"   ",12);    //清空数值
		}
		strspeed[steap]=i;
		OLED_ShowNum(80+steap*6,0,i,1,12);
		steap++;
		if(steap==3)   //只能输入3次,下次从头
			steap=0;
	}
	else  //如果i=' ',说明更新键按下把设定值更新
	{
		steap=0;
		Targetspeed=strspeed[0]*100+strspeed[1]*10+strspeed[2];
	}
}
//在DSO波形界面更改targetspeed的按键函数
void gaispeed2(unsigned char i)
{
	static unsigned char steap=0;
	if(steap==0)
	{
		OLED_ShowString(0,52,"   ",12);   //清空数值
	}
	strspeed[steap]=i;
	OLED_ShowNum(0+steap*6,52,i,1,12);
	steap++;
	if(steap==3)  //连续按三个键值就可以更改
	{
		OLED_ShowString(0,52,"ok!",12);
		steap=0;
		Targetspeed=strspeed[0]*100+strspeed[1]*10+strspeed[2];
		OLED_ShowNum(0,13,Targetspeed,3,12);
	}
}
//在page1界面更改PID的按键函数
void gaiPID(unsigned char i)
{
	static unsigned char steap=0,swback=0;
	if(swback!=sw)
	{
		steap=0;
		swback=sw;
	}
	if(i!=' ')
	{
		if(steap==0)
		{
			switch(sw)   //根据sw值选择更改KP,KI,KD
			{
				case 2:OLED_ShowString(18,13,"    ",12);break;    //清空数值
				case 3:OLED_ShowString(82,13,"    ",12);break;
				case 4:OLED_ShowString(18,27,"    ",12);break;
			}
		}
		
		if(i=='.'&&steap==1)   //如果第二个数是'.',数明要输入几点几
		{
			switch(sw)   //根据sw值选择更改KP,KI,KD，，把输入的第一个数在第二显示，第一显示为0，第三位显示'.'，下一步直接输入第四位，即小数点最后一位
			{
				case 2:str1[steap]=str1[0];str1[0]=0;OLED_ShowNum(18,13,0,1,12);OLED_ShowNum(18+steap*6,13,str1[1],1,12);OLED_ShowChar(30,13,'.',12,1);steap=2;break;
				case 3:str2[steap]=str2[0];str2[0]=0;OLED_ShowNum(82,13,0,1,12);OLED_ShowNum(82+steap*6,13,str2[1],1,12);OLED_ShowChar(94,13,'.',12,1);steap=2;break;
				case 4:str3[steap]=str3[0];str3[0]=0;OLED_ShowNum(18,27,0,1,12);OLED_ShowNum(18+steap*6,27,str3[1],1,12);OLED_ShowChar(30,27,'.',12,1);steap=2;break;
			}
		}
		else if(i=='.'||steap==2)   //如果第三个数是'.',数明要输入几十几点几，那么第三位显示'.'，并且第三步一定是'.',不管输入是不是数值
		{
			switch(sw)   //根据sw值选择更改KP,KI,KD
			{
				case 2:OLED_ShowChar(30,13,'.',12,1);break;
				case 3:OLED_ShowChar(94,13,'.',12,1);break;
				case 4:OLED_ShowChar(30,27,'.',12,1);break;
			}
		}
		else if(i=='.'&&steap==3)   ////如果第四个数是'.',数明要输入几百几十几点几，不能输入，所以格式错误，第四位小数点位变为0，最终变为几十几点零
		{
			switch(sw)
			{
				case 2:str1[steap]=0;OLED_ShowNum(18+steap*6,13,0,1,12);break;
				case 3:str2[steap]=0;OLED_ShowNum(82+steap*6,13,0,1,12);break;
				case 4:str3[steap]=0;OLED_ShowNum(18+steap*6,27,0,1,12);break;
			}
		}
		else    //其他的情况，直接把输入的数值赋值并显示
		{
			switch(sw)
			{
				case 2:str1[steap]=i;OLED_ShowNum(18+steap*6,13,i,1,12);break;
				case 3:str2[steap]=i;OLED_ShowNum(82+steap*6,13,i,1,12);break;
				case 4:str3[steap]=i;OLED_ShowNum(18+steap*6,27,i,1,12);break;
			}
		}
		steap++;
		if(steap==4)steap=0;   //只能输入4次,下次从头
	}
	else     //当按下更新时 i=' ',把PID更新
	{
		steap=0;
		Kp=str1[0]*10+str1[1]+str1[3]*0.1;
		KI=str2[0]*10+str2[1]+str2[3]*0.1;
		KD=str3[0]*10+str3[1]+str3[3]*0.1;
	}
}


//在page2界面更改cycle和max,min的按键函数，原理和PID按键函数一样
void gaiDSO(unsigned char i)
{
	static unsigned char steap=0,swback=0;
	static unsigned char str1[3]={1,0},str2[3]={1,2,0},str3[3];
	if(swback!=sw)
	{
		steap=0;
		swback=sw;
	}
	if(i!=' ')
	{
		if(steap==0)
		{
			switch(sw)
			{
				case 1:OLED_ShowString(0,16,"    ",16);break;
				case 2:OLED_ShowString(48,32,"    ",16);break;
				case 3:OLED_ShowString(48,48,"    ",16);break;
			}
		}
		
		if(sw==1&&(i=='.'||steap==1))
		{
			OLED_ShowChar(steap*8,16,'.',16,1);
		}
		else if(sw==1&&(i=='.'&&steap!=1))
		{
			str1[steap]=0;OLED_ShowNum(steap*8,16,0,1,16);
		}
		
		else if(i!='.')
		{
			switch(sw)
			{
				case 1:str1[steap]=i;OLED_ShowNum(steap*8,16,i,1,16);break;
				case 2:str2[steap]=i;OLED_ShowNum(48+steap*8,32,i,1,16);break;
				case 3:str3[steap]=i;OLED_ShowNum(48+steap*8,48,i,1,16);break;
			}
		}
		steap++;
		if(steap==3)steap=0;
	}
	else
	{
		steap=0;
		cycle=str1[0]+str1[2]*0.1;
		max=str2[0]*100+str2[1]*10+str2[2];
		min=str3[0]*100+str3[1]*10+str3[2];
	}
}

void keyaction(unsigned char keycode)
{

		switch(keycode)  //切换到示波器S1键
		{
			case 'u':page=~page;yici=1;sw=0;break;   //当界面在page12时，可以来回切换到示波器界面S1键
			case 'l':if(page!=1&&page!=2)page=~page; page++; if(page==3)page=1;  yici=1;sw=0;break;  //当界面在示波器时，先取反，再切换到另一界面S2键，
																																																//当不在示波器界面，在page12来回切换
		}
		
		//在page1界面执行的按键函数，只在page1使用
		if(page==1)
		{
			switch(keycode)
			{
				case 'd':sw++;if(sw==6)sw=1;switch(sw)   //选择S3键
				{
					case 1:OLED_ShowChar(121,0,'<',12,1);OLED_ShowChar(121,40,' ',12,1);break;
					case 2:OLED_ShowChar(56,13,'<',12,1);OLED_ShowChar(121,0,' ',12,1);break;
					case 3:OLED_ShowChar(121,13,'<',12,1);OLED_ShowChar(56,13,' ',12,1);break;
					case 4:OLED_ShowChar(56,27,'<',12,1);OLED_ShowChar(121,13,' ',12,1);break;
					case 5:OLED_ShowChar(121,40,'<',12,1);OLED_ShowChar(56,27,' ',12,1);break;
				}break;
				case 'r':gaispeed(' ');gaiPID(' ');yici=1;sw=0;break;    //更新数据S4键
				case 'e':gaispeed(' ');gaiPID(' ');yici=1;sw=0;StorAllData();storbit=1;break;  //更新数据并储存S8键
			}
			
			if(sw==1)//设定speed
			{
				switch(keycode)
				{
					case 1:gaispeed(1);break;
					case 2:gaispeed(2);break;
					case 3:gaispeed(3);break;
					case 4:gaispeed(4);break;
					case 5:gaispeed(5);break;
					case 6:gaispeed(6);break;
					case 7:gaispeed(7);break;
					case 8:gaispeed(8);break;
					case 9:gaispeed(9);break;
					case 0:gaispeed(0);break;
				}
			}
			
			else if(sw==2)//设定Kp
			{
				switch(keycode)
				{
					case 1:gaiPID(1);break;
					case 2:gaiPID(2);break;
					case 3:gaiPID(3);break;
					case 4:gaiPID(4);break;
					case 5:gaiPID(5);break;
					case 6:gaiPID(6);break;
					case 7:gaiPID(7);break;
					case 8:gaiPID(8);break;
					case 9:gaiPID(9);break;
					case 0:gaiPID(0);break;
					case 'n':gaiPID('.');break;
				}
			}
			
			else if(sw==3)//设定KI
			{
				switch(keycode)
				{
					case 1:gaiPID(1);break;
					case 2:gaiPID(2);break;
					case 3:gaiPID(3);break;
					case 4:gaiPID(4);break;
					case 5:gaiPID(5);break;
					case 6:gaiPID(6);break;
					case 7:gaiPID(7);break;
					case 8:gaiPID(8);break;
					case 9:gaiPID(9);break;
					case 0:gaiPID(0);break;
					case 'n':gaiPID('.');break;
				}
			}
			else if(sw==4)//设定KD
			{
				switch(keycode)
				{
					case 1:gaiPID(1);break;
					case 2:gaiPID(2);break;
					case 3:gaiPID(3);break;
					case 4:gaiPID(4);break;
					case 5:gaiPID(5);break;
					case 6:gaiPID(6);break;
					case 7:gaiPID(7);break;
					case 8:gaiPID(8);break;
					case 9:gaiPID(9);break;
					case 0:gaiPID(0);break;
					case 'n':gaiPID('.');break;
				}
			}
			else if(sw==5)//设定转向
			{
				switch(keycode)
				{
					case 'n':PAout(15)=~PAout(15);PAout(12)=~PAout(12);directionbit=~directionbit;  //改变引脚电平，改变电机转向
					if(directionbit==1)
						OLED_ShowChar(66,40,'+',12,1);   //加号代表正转
					else 
						{OLED_ShowChar(66,40,' ',12,1);OLED_ShowChar(66,40,'-',12,1);}break;//清除加号显示减号
				}
			}
		}
		
		//在page2界面执行的按键函数，只在page2使用
		else if(page==2)   //在设定波形图界面
		{
			switch(keycode)
			{
				case 'd':sw++;if(sw==4)sw=1;switch(sw)   //选择S3键
				{
					case 1:OLED_ShowChar(88,16,'<',16,1);OLED_ShowChar(88,48,' ',16,1);break;
					case 2:OLED_ShowChar(88,32,'<',16,1);OLED_ShowChar(88,16,' ',16,1);break;
					case 3:OLED_ShowChar(88,48,'<',16,1);OLED_ShowChar(88,32,' ',16,1);break;
				}break;
				case 'r':gaiDSO(' ');yici=1;sw=0;break;    //更新数据S4键
			}
			
			if(sw==1)//设定cycle
			{
				switch(keycode)
				{
					case 1:gaiDSO(1);break;
					case 2:gaiDSO(2);break;
					case 3:gaiDSO(3);break;
					case 4:gaiDSO(4);break;
					case 5:gaiDSO(5);break;
					case 6:gaiDSO(6);break;
					case 7:gaiDSO(7);break;
					case 8:gaiDSO(8);break;
					case 9:gaiDSO(9);break;
					case 0:gaiDSO(0);break;
					case 'n':gaiDSO('.');break;
				}
			}
			
			else if(sw==2)//设定max
			{
				switch(keycode)
				{
					case 1:gaiDSO(1);break;
					case 2:gaiDSO(2);break;
					case 3:gaiDSO(3);break;
					case 4:gaiDSO(4);break;
					case 5:gaiDSO(5);break;
					case 6:gaiDSO(6);break;
					case 7:gaiDSO(7);break;
					case 8:gaiDSO(8);break;
					case 9:gaiDSO(9);break;
					case 0:gaiDSO(0);break;
				}
			}
			
			else if(sw==3)//设定min
			{
				switch(keycode)
				{
					case 1:gaiDSO(1);break;
					case 2:gaiDSO(2);break;
					case 3:gaiDSO(3);break;
					case 4:gaiDSO(4);break;
					case 5:gaiDSO(5);break;
					case 6:gaiDSO(6);break;
					case 7:gaiDSO(7);break;
					case 8:gaiDSO(8);break;
					case 9:gaiDSO(9);break;
					case 0:gaiDSO(0);break;
				}
			}
			
			
			
			
			
			
			
		}
		
		
		
		
		//在DSO示波器界面执行的按键函数
		else   //在示波器界面
		{
				switch(keycode)
				{
					case 1:gaispeed2(1);break;
					case 2:gaispeed2(2);break;
					case 3:gaispeed2(3);break;
					case 4:gaispeed2(4);break;
					case 5:gaispeed2(5);break;
					case 6:gaispeed2(6);break;
					case 7:gaispeed2(7);break;
					case 8:gaispeed2(8);break;
					case 9:gaispeed2(9);break;
					case 0:gaispeed2(0);break;
					case 'n':PAout(15)=~PAout(15);PAout(12)=~PAout(12);directionbit=~directionbit;
					if(directionbit==1)OLED_ShowChar(6,40,'+',12,1);else OLED_ShowChar(6,40,'-',12,1);break;  //按下S8改变转向
				}
			
		}
	
}

