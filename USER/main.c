#include "output.h"
#include "delay.h"
#include "sys.h"
#include "rc522.h"
#include "lcd.h"			 //显示模块
#include "key.h"             //矩阵键盘模块
#include "usart.h"
#include "string.h" 
#include "timer.h"
#include "stmflash.h"

#include "usmart.h"
#include "exti.h"
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"
#include "piclib.h"
#include "math.h"	 
#include "ov7725.h" 
#include "ov7670.h"

#include "tpad.h"
#include "string.h"

#include "touch.h"		
#include "usart3.h"
#include "common.h" 

#include "vs10xx.h"
#include "mp3player.h"	
 

#define  OV7725 1
#define  OV7670 2
//////////////////////////////////////////////////////////
//M1卡分為16個扇區，每個扇區由4塊（塊0、塊1、塊2、塊3）組成
//我們也將16個扇區的64個塊按絕對地址編號0~63
//第0扇區的塊0（即絕對地址0塊），他用於存放廠商代碼，已經固化，不可更改
//每個扇區的塊0、塊1、塊2為數據塊，可用於存放數據
//每個扇區的塊3為控制塊（絕對地址塊3、7、11....），包括了密碼A，存取控制、密碼B。

/*******************************
*连线说明：
*1--SS  <----->PF0
*2--SCK <----->PB13
*3--MOSI<----->PB15
*4--MISO<----->PB14
*5--悬空
*6--GND <----->GND
*7--RST <----->PF1
*8--VCC <----->VCC
************************************/
/*全局变量*/

extern u8 ov_sta;	//在exit.c里面定义
extern u8 ov_frame;	//在timer.c里面定义

const u8* portnum="8086";
FIL fil;
FRESULT res;
UINT bww;
char buf[20];
char buf_save[20];

u8 ipbuf[16]; 	//IP缓存
u8 *p;

int flag = 0;
int flagg = 0;
char num[10];
//unsigned char num;
unsigned char CT[2];//卡类型
//unsigned char SN[5]; //卡号
char SN[5];//卡号
char name[7] = {"000.txt"};
//unsigned char *SN4; //卡号的集合
unsigned char RFID[16];			//存放RFID 
unsigned char lxl_bit=0;
unsigned char card1_bit=0;
unsigned char card2_bit=0;
unsigned char card3_bit=0;
unsigned char card4_bit=0;
unsigned char total=0;
unsigned char lxl[4]={6,109,250,186};
unsigned char card_1[4]={66,193,88,0};
unsigned char card_2[4]={66,191,104,0};
unsigned char card_3[4]={62,84,28,11};
unsigned char card_4[4]={126,252,248,12};
u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char RFID1[16]={0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x07,0x80,0x29,0xff,0xff,0xff,0xff,0xff,0xff};
//函数声明
void ShowID(u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor);	 //显示卡的卡号，以十六进制显示
void PutNum(u16 x,u16 y, u32 n1,u8 n0, u16 charColor, u16 bkColor);	//显示余额函数
void Store(u8 *p,u8 store,u8 cash);//最重要的一个函数

void BEEP_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);  
	 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_ResetBits(GPIOB,GPIO_Pin_8);
}

void LED_Init(void)
{
	  GPIO_InitTypeDef  GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE,ENABLE);  //LED0 PB5 && LED1 PE5
	 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_SetBits(GPIOB,GPIO_Pin_5);
	 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_Init(GPIOE,&GPIO_InitStructure);
		GPIO_SetBits(GPIOE,GPIO_Pin_5);
}

//更新LCD显示(OV7725)
void OV7725_camera_refresh(void)
{
	u32 i,j;
 	u16 color;	 
	if(ov_sta)//有帧中断更新
	{
		LCD_Scan_Dir(U2D_L2R);//从上到下,从左到右
		LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//将显示区域设置到屏幕中央
		if(lcddev.id==0X1963)
			LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,240,320);//将显示区域设置到屏幕中央
		LCD_WriteRAM_Prepare();     //开始写入GRAM	
		OV7725_RRST=0;				//开始复位读指针 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//复位读指针结束 
		OV7725_RCK_H; 
		for(i=0;i<240;i++)
		{
			for(j=0;j<320;j++)
			{
				OV7725_RCK_L;
				color=GPIOC->IDR&0XFF;	//读数据
				OV7725_RCK_H; 
				color<<=8;  
				OV7725_RCK_L;
				color|=GPIOC->IDR&0XFF;	//读数据
				OV7725_RCK_H; 
				LCD->LCD_RAM=color;  
			}
		}
 		ov_sta=0;					//清零帧中断标记
		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向 
	} 
}

//更新LCD显示(OV7670)
void OV7670_camera_refresh(void)
{
	u32 j;
 	u16 color;	 
	if(ov_sta)//有帧中断更新
	{
		LCD_Scan_Dir(U2D_L2R);//从上到下,从左到右  
		if(lcddev.id==0X1963)LCD_Set_Window((lcddev.width-240)/2,(lcddev.height-320)/2,240,320);//将显示区域设置到屏幕中央
		else if(lcddev.id==0X5510||lcddev.id==0X5310)LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//将显示区域设置到屏幕中央
		LCD_WriteRAM_Prepare();     //开始写入GRAM	
		OV7670_RRST=0;				//开始复位读指针 
		OV7670_RCK_L;
		OV7670_RCK_H;
		OV7670_RCK_L;
		OV7670_RRST=1;				//复位读指针结束 
		OV7670_RCK_H;
		for(j=0;j<76800;j++)
		{
			OV7670_RCK_L;
			color=GPIOC->IDR&0XFF;	//读数据
			OV7670_RCK_H; 
			color<<=8;  
			OV7670_RCK_L;
			color|=GPIOC->IDR&0XFF;	//读数据
			OV7670_RCK_H; 
			LCD->LCD_RAM=color;    
		}   							  
 		ov_sta=0;					//清零帧中断标记
		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向 
	} 
}
 
//文件名自增（避免覆盖）
//组合成:形如"0:PHOTO/PIC13141.bmp"的文件名
void camera_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:PHOTO/PIC%05d.bmp",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
}

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO tfileinfo;	//临时文件信息	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//长文件名最大长度
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	return rval;
}

int main(void)
{		
	
     u8 timex=0; 
	//u8 ipbuf[16]; 	//IP缓存
	//u8 *p;
	//u16 t=999;		//加速第一次获取链接状态
	u16 rlen=0;
	u8 constate=0;	//连接状态
	
	
	
	unsigned char status;
	unsigned char s=0x08;
	u16 led0pwmval=0;
	//u8 t;
	u8 key;
	u8 j;
	u16 curindex;		//图片当前索引
	u8 *fn;   			//长文件名
      u16 temp;  
	
	
	
	DIR picdir;	 		//图片目录
	FILINFO picfileinfo;//文件信息
	int i = 0;
	u32 total,free;
      u8 t=0; 
      u8 res=0;  
      u8 *pname;              //带路径的文件名
      u8 sensor=0; 	
	u16 totpicnum; 		//图片文件总数
	u16 *picindextbl;	//图片索引表 


	p=mymalloc(SRAMIN,32);							//申请32字节内存
	
 	delay_init();	    	 //延时函数初始化	  
      NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
 	//OUTPUT_Init();			 //输出模块初始化
	uart_init(115200);				
	LCD_Init();
	KEY_Init();
	InitRc522();				//初始化射频卡模块
	BEEP_Init();
	LED_Init();
	tp_dev.init();              //初始化触摸屏
	usart3_init(115200);        //初始化串口3 
	usmart_dev.init(72);        //初始化USMART    
	W25QXX_Init();              //初始化W25Q128
	VS_Init();	  				//初始化VS1053 
      my_mem_init(SRAMIN);        //初始化内部内存池
	SD_Init();
	
	exfuns_init();                          //为fatfs相关变量申请内存                 
      f_mount(fs[0],"0:",1);                  //挂载SD卡 
      res=f_mount(fs[1],"1:",1);              //挂载FLASH.  
	//sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。
	//LEDA=1;
       OV7725_Init();
	font_init();
	res=f_mkdir("0:/PHOTO");        //创建PHOTO文件夹
	pname=mymalloc(SRAMIN,30);  //为带路径的文件名分配30个字节的内存
      sensor=OV7725;	

      
	
	TPAD_Init(6);			//初始化触摸按键
     
	piclib_init();										//初始化画图	 
      ai_load_picfile("0:/PICTURE/1.jpg",0,0,lcddev.width,lcddev.height,1);//显示图片 	
	mp3_play();
	while(!TPAD_Scan(0))//成功捕获到了一次上升沿(此函数执行时间至少15ms
	{
		
		ai_load_picfile("0:/PICTURE/1.jpg",0,0,lcddev.width,lcddev.height,1);//显示图片 
	     Show_Str(80,60,240,24,"请用手触碰右下按键，开启系统!",12,0);
           delay_ms(1000);
		LCD_Fill(80,60,300,84,WHITE);
		
	}	
	printf("there is a gay open the system!!");
	
	
	LCD_Clear(WHITE);
	    // BEEP=!BEEP;		//LED1取反
	LEDB =!LEDB;
	OV7725_Window_Set(320,240,1);//VGA模式输出
     
	OV7725_Light_Mode(0);
            OV7725_Color_Saturation(0);
            OV7725_Brightness(0);
            OV7725_Contrast(0);
            OV7725_Special_Effects(0);
            OV7725_CS=0;
	mp3_play2();	
	ipbuf[0] = '1';ipbuf[1] = '9';ipbuf[2] = '2';	
	ipbuf[3] = '.';ipbuf[4] = '1';ipbuf[5] = '6';
	ipbuf[6] = '8';ipbuf[7] = '.';ipbuf[8] = '4';
	ipbuf[9] = '.';ipbuf[10] = '2';ipbuf[11] = '\0';
	atk_8266_send_cmd("AT+CIPMUX=0","OK",20);   //0：单连接，1：多连接
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标TCP服务器
	atk_8266_send_cmd(p,"OK",200);
	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //传输模式为：透传		

	LEDB =!LEDB;
	//BEEP=!BEEP;
	atk_8266_get_wanip(ipbuf);//服务器模式,获取WAN IP
	sprintf((char*)p,"IP地址:%s 端口:%s",ipbuf,(u8*)portnum);
		LCD_ShowString(0,0,200,16,16,"RFID_Beginning!!");
           USART3_RX_STA=0;
   // p=mymalloc(SRAMIN,32);							//申请32字节内存
      mp3_play3();
    
    
    /***********************************************/
	//按下KEY0存储卡号，名称为00x.txt
	//保存所有的识别卡号到user.txt中
	//串口已经写好(消息均正确)
	//蓝牙串口已经加上了，注意RX和TX互相插（连接不是很稳定，注意不要接触不良）。
	//波特率115200
	//已经加上了舵机（用3.3V供电）。
	//已加上了照相机，有照相以及存储的功能。
	//已加上TPAD按键，加上开机图片（一开始显示开机图片，按下TPAD键后，进入主页面）。
	//改上位机。(发送字符'1'可以转动舵机)
	//WIFI模块已经加上，手机上面选择为TCP SERVER，STM32端为TCP 客户端。自动连接
	//已经加上语音模块。
	//加上了与APP的信息传递。
	
	//接下来：
	
	/************************************************/
  	while(1) 
	{		
		key=KEY_Scan(0);
         /////////////////////////////////////////////////////////////////////
		if(key==KEY_LEFT||key==KEY_DOWN||key==KEY_RIGHT)	//KEY0 发送数据 
		{
				

				atk_8266_quit_trans();
				atk_8266_send_cmd("AT+CIPSEND","OK",20);       //开始透传
				sprintf((char*)p,"杨毅远,李义");//测试数据
				//Show_Str(30+54,100,200,12,p,12,0);
				u3_printf("%s",p);
				timex=100;


		}
			
		if(timex)timex--;
		if(timex==1)LCD_Fill(30+54,100,239,112,WHITE);
		t++;
		delay_ms(5);
		if(USART3_RX_STA&0X8000)		//接收到一次数据了
		{ 
			rlen=USART3_RX_STA&0X7FFF;	//得到本次接收到的数据长度
			USART3_RX_BUF[rlen]=0;		//添加结束符 
			printf("%s",USART3_RX_BUF);	//发送到串口   
			
			
			sprintf((char*)p,"收到%d字节,内容如下",rlen);//接收到的字节数 
			//LCD_Fill(30+54,115,239,130,WHITE);
			//POINT_COLOR=BRED;
			//Show_Str(30+54,115,156,12,p,12,0); 			//显示接收到的数据长度
			POINT_COLOR=BLUE;
			LCD_Fill(0,130,45,220,WHITE);
			Show_Str(0,130,180,190,USART3_RX_BUF,12,0);//显示接收到的数据  
			USART3_RX_STA=0;
			if(constate!='+')t=1000;		//状态为还未连接,立即更新连接状态
			else t=0;                   //状态为已经连接了,10秒后再检查
			if(USART3_RX_BUF[0] == '1')
			{
				Show_Str(0,150,180,190,"wifi开门",12,0);//显示接收到的数据 
				TIM3_PWM_Init(9999,71);  //50Hz 10000极值
					TIM_SetCompare2(TIM3,9425);
					delay_ms(3000);
  	
					for(led0pwmval=9425;led0pwmval>7750;led0pwmval-=100)		  
					{
		 
							TIM_SetCompare2(TIM3,led0pwmval);
							delay_ms(50);
					}	
					TIM_SetCompare2(TIM3,9425);
					delay_ms(700);
					
					TIM_Cmd(TIM3,DISABLE);
			}
		}  
		if(t==1000)//连续10秒钟没有收到任何数据,检查连接是不是还存在.
		{
			constate=atk_8266_consta_check();//得到连接状态
			if(constate=='+')Show_Str(30+30,80,200,12,"连接成功",12,0);  //连接状态
			else Show_Str(30+30,80,200,12,"连接失败",12,0); 	  	 
			t=0;
		}
		atk_8266_at_response(1);
		/////////////////////////////////////////////////////////////////////////////////////
                  //atk_8266_wifiap_test();

			if(USART_RX_BUF[0] == '1'&&flagg == 0)
			{
					TIM3_PWM_Init(9999,71);  //50Hz 10000极值
					TIM_SetCompare2(TIM3,9425);
					delay_ms(3000);
  	
					for(led0pwmval=9425;led0pwmval>7750;led0pwmval-=100)		  
					{
		 
							TIM_SetCompare2(TIM3,led0pwmval);
							delay_ms(50);
					}	
					TIM_SetCompare2(TIM3,9425);
					delay_ms(700);
					
					TIM_Cmd(TIM3,DISABLE);
					flagg++;
			}
			
			
		
		      
	       	

			TIM6_Int_Init(10000,7199);          //10Khz计数频率,1秒钟中断                                     
			EXTI8_Init();  
		      OV7725_camera_refresh();//更新显示

               
	 	
			status = PcdRequest(PICC_REQALL,CT);//尋卡  //此处有问题

			
			if(status==MI_OK)//尋卡成功
			{
				
				LCD_ShowString(0,0,200,16,16,"PcdRequest_MI_OK");
				status=MI_ERR;
				status = PcdAnticoll(SN);//防冲撞		
			}
			
			f_open(&fil,"0:/TXT/num.txt",FA_READ);
			f_read(&fil, buf ,1,&bww);
			f_close(&fil);
				
			num[0] = (buf[0]);

			if((TPAD_Scan(0))&&(i==0))
			{

						name[2]=0X30+num[0]-'0'; //转成ASCII字符
						f_open(&fil,name, FA_WRITE|FA_OPEN_ALWAYS);
						f_write (&fil,SN ,4, &bww);
						f_close(&fil);
						LCD_ShowString(0,420,230,24,24,"have saved!!!");
				            BEEP = 1;
						num[0] ++;
				
						f_open(&fil,"0:/TXT/num.txt", FA_WRITE|FA_OPEN_ALWAYS);
						f_write (&fil, num,1, &bww);
						f_close(&fil);
						printf("the new user have saved in SD_card!\r\n");
						i++;
						
					
					
					
			}

			if (status==MI_OK )//防衝撞成功
			{

				LCD_ShowString(150,0,200,16,16,"PcdAnticoll_MI_OK");
				/****************************舵机程序（有BUG）********************************/
				      TIM3_PWM_Init(9999,71);  //50Hz 10000极值
					TIM_SetCompare2(TIM3,9425);
					delay_ms(3000);
  	
					for(led0pwmval=9425;led0pwmval>7750;led0pwmval-=100)		  
					{
		 
							TIM_SetCompare2(TIM3,led0pwmval);
							delay_ms(50);
					}	
					TIM_SetCompare2(TIM3,9425);
					delay_ms(700);
                              TIM_Cmd(TIM3,DISABLE);
				/***************************************************************/
				LCD_ShowString(150,0,200,16,16,"PcdAnticoll_MI_OK");
				status=MI_ERR;		

				LEDB=1;
				ShowID(0,20,SN,BLUE,WHITE); //在液晶屏上显示卡的ID号
				printf("The Card ID is:");

				LEDB=0;  //B亮
				delay_ms(300);
				
				LCD_ShowString(0,56,140,16,16,"The Card ID is:");
				for(j=0;j<4;j++)
				{
					 LCD_ShowNum(150+32*j,56,SN[j],3,16);
					 
                        		
				}
				
				SN[4] = '\0';
				
			      printf("ID:%02x %02x %02x %02x\r\n",SN[0],SN[1],SN[2],SN[3]);//发送卡号
				printf("ID:%s\r\n",SN);
				
				f_open(&fil,"0:/TXT/user.txt", FA_WRITE|FA_OPEN_ALWAYS);
				f_lseek(&fil, f_size(&fil)); 
				f_write (&fil, "user :",6, &bww);
				f_lseek(&fil, f_size(&fil)); 
				f_write (&fil, SN,4, &bww);
				f_close(&fil);
				printf("your message already in SD_card!!!\r\n");
				
				
				camera_new_pathname(pname);//得到文件名
				bmp_encode(pname,(lcddev.width-240)/2,(lcddev.height-320)/2,240,320,0);

				
				BEEP = 1;

                        //////////////////////////////////////////////////////
				atk_8266_quit_trans();
				atk_8266_send_cmd("AT+CIPSEND","OK",20);       //开始透传
				sprintf((char*)p,"%02x%02x%02x%02x",SN[0],SN[1],SN[2],SN[3]);//测试数据
				if(p[0] == '6'&& p[1] == '0')
				{
					p[0] = '1';
					p[1] = '\0';
					
					
				}
				else if(p[0] == '2'&& p[1] == '3')
				{
					p[0] = '2';
					p[1] = '\0';
				}
				else if(p[0] == 'a' && p[1] == '3')
				{
					p[0] = '3';
					p[1] = '\0';
				}
				else
                        {
					p[0] = '4';
					p[1] = '\0';
				}
				//Show_Str(30+54,100,200,12,p,12,0);
				u3_printf("%s",p);
				//timex=100;
				//////////////////////////////////////////////////////				
				LCD_ShowString(0,450,230,24,24,"Finish!!!");

				
				Reset_RC522();
				myfree(SRAMIN,picfileinfo.lfname);	//释放内存			    
				myfree(SRAMIN,pname);				//释放内存			    
				myfree(SRAMIN,picindextbl);			//释放内存
				

                       			
			}
			else
			{

				LEDB=1;
				BEEP = 0;
			}
			
		      if(status==MI_OK)//選卡成功
			{
				LCD_ShowString(0,48,200,16,16,"PcdSelect_MI_OK  ");
				status=MI_ERR;
				status =PcdAuthState(0x60,0x09,KEY,SN);
			}
			if(status==MI_OK)//驗證成功
			{
				LCD_ShowString(0,64,200,16,16,"PcdAuthState_MI_OK  ");
				status=MI_ERR;
				status=PcdRead(s,RFID);

			}

			if(status==MI_OK)//讀卡成功
			{
				LCD_ShowString(0,80,200,16,16,"READ_MI_OK");
				status=MI_ERR;
				delay_ms(100);
			}

	
	}
		
}


/*************************************
*函数功能：显示卡的卡号，以十六进制显示
*参数：x，y 坐标
*		p 卡号的地址
*		charcolor 字符的颜色
*		bkcolor   背景的颜色
***************************************/

void ShowID(u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor)	 //显示卡的卡号，以十六进制显示
{
	u8 num[9];
	u8 i;

	for(i=0;i<4;i++)
	{
		num[i*2]=p[i]/16;
		num[i*2]>9?(num[i*2]+='7'):(num[i*2]+='0');
		num[i*2+1]=p[i]%16;
		num[i*2+1]>9?(num[i*2+1]+='7'):(num[i*2+1]+='0');
	}
	num[8]=0;
	POINT_COLOR=RED;	  
	LCD_ShowString(x,y,150,16,16,"The Card ID is:");	
	//DisplayString(x,y+16,num,charColor,bkColor);
 	for(i=0;i<8;i++)
	{
		  LCD_ShowNum(x+150+16*i,y,num[i],2,16);
	}	
}
/********************************
*函数功能：求p的n次幂
*/

int power(u8 p,u8 n)
{
	int pow=1;
	u8 i;
	for(i=0;i<n;i++)
	{
		pow*=p;	
	}
	return pow;
}
 
u8 ReadData(u8   addr,u8 *pKey,u8 *pSnr,u8 *dataout)
{
	u8 status,k;
	status=0x02;
	k=5;
	do
	{
		status=PcdAuthState(PICC_AUTHENT1A,addr,pKey,pSnr);
		k--;
		//printf("AuthState is wrong\n");						      
	}while(status!=MI_OK && k>0);

	status=0x02;//
	k=5;
	do
	{
		status=PcdRead(addr,dataout);
		k--;
		//printf("ReadData is wrong\n");							      
	}while(status!=MI_OK && k>0);
	return status;
}

u8 WriteData(u8   addr,u8 *pKey,u8 *pSnr,u8 *datain)
{
	u8 status,k;
	status=0x02;//
	k=5;
	do
	{
		status=PcdAuthState(PICC_AUTHENT1A,addr,pKey,pSnr);
		k--;
		//printf("AuthState is wrong\n");						      
	}while(status!=MI_OK && k>0);

	status=0x02;//
	k=5;
	do
	{
		status=PcdWrite(addr,datain);
		k--;
		//printf("ReadData is wrong\n");							      
	}while(status!=MI_OK && k>0);
	return status;
}

void PutNum(u16 x,u16 y, u32 n1,u8 n0, u16 charColor, u16 bkColor)
{
	
}
void Store(u8 *p,u8 store,u8 cash)
{

}








/*
 int main(void)
 {	 
 
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
 	LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD     
	W25QXX_Init();				//初始化W25Q128
 	VS_Init();	  				//初始化VS1053 
 	my_mem_init(SRAMIN);		//初始化内部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
 	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.
	POINT_COLOR=RED;       
 	/*while(font_init()) 				//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
	}
 	Show_Str(30,50,200,16,"战舰 STM32开发板",16,0);				    	 
	Show_Str(30,70,200,16,"音乐播放器实验",16,0);				    	 
	Show_Str(30,90,200,16,"正点原子@ALIENTEK",16,0);				    	 
	Show_Str(30,110,200,16,"2015年1月20日",16,0);
	Show_Str(30,130,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
	Show_Str(30,150,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);*/
	/*while(1)
	{
  		/*LED1=0; 	  
		Show_Str(30,170,200,16,"存储器测试...",16,0);
		printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
		Show_Str(30,170,200,16,"正弦波测试...",16,0); 	 	 
 		VS_Sine_Test();	   
		Show_Str(30,170,200,16,"<<音乐播放器>>",16,0); 		 
		LED1=1;*/
		/*mp3_play();
	} 	   										    
}
*/






































