#include "common.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//ATK-ESP8266 WIFI模块 WIFI AP驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/4/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void TIM3_PWM_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	 	 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStrue;
	TIM_OCInitTypeDef TIM_OCInitTypeStrue;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3,ENABLE);  ///复用使能
	
	TIM_TimeBaseInitStrue.TIM_Period = arr;  //自动装载值
	TIM_TimeBaseInitStrue.TIM_Prescaler =  psc;  //预分频系数
	TIM_TimeBaseInitStrue.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInitStrue.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStrue);
	
	TIM_OCInitTypeStrue.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitTypeStrue.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitTypeStrue.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitTypeStrue.TIM_Pulse = 100;
	TIM_OC2Init(TIM3,&TIM_OCInitTypeStrue);

      TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);

      TIM_Cmd(TIM3,ENABLE);
}
//ATK-ESP8266 WIFI AP测试
//用于测试TCP/UDP连接
//返回值:0,正常
//    其他,错误代码
extern u8 ipbuf[16];
extern u8 *p;
u8 atk_8266_wifiap_test(void)
{
	//u8 netpro=0;	//网络模式
	u8 key;
	u8 timex=0; 
	//u8 ipbuf[16]; 	//IP缓存
	//u8 *p;
	u16 t=999;		//加速第一次获取链接状态
	u8 res=0;
	u16 rlen=0;
	u8 constate=0;	//连接状态
	//p=mymalloc(SRAMIN,32);							//申请32字节内存

	           

	/*LCD_Clear(WHITE);
	POINT_COLOR=RED;
	Show_Str_Mid(0,30,"ATK-ESP WIFI-AP 测试",16,240);
	Show_Str(30,50,200,16,"正在配置ATK-ESP模块,请稍等...",12,0);			
	LCD_Fill(30,50,239,50+12,WHITE);			//清除之前的显示
	Show_Str(30,50,200,16,"配置ATK-ESP模块成功!",12,0);
	delay_ms(200);
	Show_Str(30,50,200,16,"WK_UP:退出测试  KEY0:发送数据",12,0);
	LCD_Fill(30,80,239,80+12,WHITE);*/
	
	//atk_8266_get_wanip(ipbuf);//服务器模式,获取WAN IP
	//sprintf((char*)p,"IP地址:%s 端口:%s",ipbuf,(u8*)portnum);
	
	/*Show_Str(30,65,200,12,p,12,0);				//显示IP地址和端口	
	Show_Str(30,80,200,12,"状态:",12,0); 		//连接状态
	Show_Str(120,80,200,12,"模式:",12,0); 		//连接状态
	Show_Str(30,100,200,12,"发送数据:",12,0); 	//发送数据
	Show_Str(30,115,200,12,"接收数据:",12,0);	//接收数据*/
	//atk_8266_wificonf_show(30,180,"请用设备连接WIFI热点:",(u8*)wifiap_ssid,(u8*)wifiap_encryption,(u8*)wifiap_password);
	//POINT_COLOR=BLUE;
	//Show_Str(120+30,80,200,12,(u8*)ATK_ESP8266_WORKMODE_TBL[netpro],12,0); 		//连接状态
	
	//USART3_RX_STA=0;
	
	//while(1)
	{
		//key=KEY_Scan(0);

		if(key==KEY0_PRES)	//KEY0 发送数据 
		{
				

				atk_8266_quit_trans();
				atk_8266_send_cmd("AT+CIPSEND","OK",20);       //开始透传
				sprintf((char*)p,"ATK-8266%02d\r\n",t/10);//测试数据
				Show_Str(30+54,100,200,12,p,12,0);
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
			LCD_Fill(30+54,115,239,130,WHITE);
			POINT_COLOR=BRED;
			Show_Str(30+54,115,156,12,p,12,0); 			//显示接收到的数据长度
			POINT_COLOR=BLUE;
			LCD_Fill(30,130,239,319,WHITE);
			Show_Str(30,130,180,190,USART3_RX_BUF,12,0);//显示接收到的数据  
			USART3_RX_STA=0;
			if(constate!='+')t=1000;		//状态为还未连接,立即更新连接状态
			else t=0;                   //状态为已经连接了,10秒后再检查
		}  
		if(t==1000)//连续10秒钟没有收到任何数据,检查连接是不是还存在.
		{
			constate=atk_8266_consta_check();//得到连接状态
			if(constate=='+')Show_Str(30+30,80,200,12,"连接成功",12,0);  //连接状态
			else Show_Str(30+30,80,200,12,"连接失败",12,0); 	  	 
			t=0;
		}
		atk_8266_at_response(1);
	}
	//myfree(SRAMIN,p);		//释放内存 
	//return res;		
} 







