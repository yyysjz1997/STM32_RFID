#include "common.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-ESP8266 WIFIģ�� WIFI AP��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/4/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3,ENABLE);  ///����ʹ��
	
	TIM_TimeBaseInitStrue.TIM_Period = arr;  //�Զ�װ��ֵ
	TIM_TimeBaseInitStrue.TIM_Prescaler =  psc;  //Ԥ��Ƶϵ��
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
//ATK-ESP8266 WIFI AP����
//���ڲ���TCP/UDP����
//����ֵ:0,����
//    ����,�������
extern u8 ipbuf[16];
extern u8 *p;
u8 atk_8266_wifiap_test(void)
{
	//u8 netpro=0;	//����ģʽ
	u8 key;
	u8 timex=0; 
	//u8 ipbuf[16]; 	//IP����
	//u8 *p;
	u16 t=999;		//���ٵ�һ�λ�ȡ����״̬
	u8 res=0;
	u16 rlen=0;
	u8 constate=0;	//����״̬
	//p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�

	           

	/*LCD_Clear(WHITE);
	POINT_COLOR=RED;
	Show_Str_Mid(0,30,"ATK-ESP WIFI-AP ����",16,240);
	Show_Str(30,50,200,16,"��������ATK-ESPģ��,���Ե�...",12,0);			
	LCD_Fill(30,50,239,50+12,WHITE);			//���֮ǰ����ʾ
	Show_Str(30,50,200,16,"����ATK-ESPģ��ɹ�!",12,0);
	delay_ms(200);
	Show_Str(30,50,200,16,"WK_UP:�˳�����  KEY0:��������",12,0);
	LCD_Fill(30,80,239,80+12,WHITE);*/
	
	//atk_8266_get_wanip(ipbuf);//������ģʽ,��ȡWAN IP
	//sprintf((char*)p,"IP��ַ:%s �˿�:%s",ipbuf,(u8*)portnum);
	
	/*Show_Str(30,65,200,12,p,12,0);				//��ʾIP��ַ�Ͷ˿�	
	Show_Str(30,80,200,12,"״̬:",12,0); 		//����״̬
	Show_Str(120,80,200,12,"ģʽ:",12,0); 		//����״̬
	Show_Str(30,100,200,12,"��������:",12,0); 	//��������
	Show_Str(30,115,200,12,"��������:",12,0);	//��������*/
	//atk_8266_wificonf_show(30,180,"�����豸����WIFI�ȵ�:",(u8*)wifiap_ssid,(u8*)wifiap_encryption,(u8*)wifiap_password);
	//POINT_COLOR=BLUE;
	//Show_Str(120+30,80,200,12,(u8*)ATK_ESP8266_WORKMODE_TBL[netpro],12,0); 		//����״̬
	
	//USART3_RX_STA=0;
	
	//while(1)
	{
		//key=KEY_Scan(0);

		if(key==KEY0_PRES)	//KEY0 �������� 
		{
				

				atk_8266_quit_trans();
				atk_8266_send_cmd("AT+CIPSEND","OK",20);       //��ʼ͸��
				sprintf((char*)p,"ATK-8266%02d\r\n",t/10);//��������
				Show_Str(30+54,100,200,12,p,12,0);
				u3_printf("%s",p);
				timex=100;


		}
			
		if(timex)timex--;
		if(timex==1)LCD_Fill(30+54,100,239,112,WHITE);
		t++;
		delay_ms(5);
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{ 
			rlen=USART3_RX_STA&0X7FFF;	//�õ����ν��յ������ݳ���
			USART3_RX_BUF[rlen]=0;		//��ӽ����� 
			printf("%s",USART3_RX_BUF);	//���͵�����   
			
			
			sprintf((char*)p,"�յ�%d�ֽ�,��������",rlen);//���յ����ֽ��� 
			LCD_Fill(30+54,115,239,130,WHITE);
			POINT_COLOR=BRED;
			Show_Str(30+54,115,156,12,p,12,0); 			//��ʾ���յ������ݳ���
			POINT_COLOR=BLUE;
			LCD_Fill(30,130,239,319,WHITE);
			Show_Str(30,130,180,190,USART3_RX_BUF,12,0);//��ʾ���յ�������  
			USART3_RX_STA=0;
			if(constate!='+')t=1000;		//״̬Ϊ��δ����,������������״̬
			else t=0;                   //״̬Ϊ�Ѿ�������,10����ټ��
		}  
		if(t==1000)//����10����û���յ��κ�����,��������ǲ��ǻ�����.
		{
			constate=atk_8266_consta_check();//�õ�����״̬
			if(constate=='+')Show_Str(30+30,80,200,12,"���ӳɹ�",12,0);  //����״̬
			else Show_Str(30+30,80,200,12,"����ʧ��",12,0); 	  	 
			t=0;
		}
		atk_8266_at_response(1);
	}
	//myfree(SRAMIN,p);		//�ͷ��ڴ� 
	//return res;		
} 







