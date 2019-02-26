#include "output.h"
#include "delay.h"
#include "sys.h"
#include "rc522.h"
#include "lcd.h"			 //��ʾģ��
#include "key.h"             //�������ģ��
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
//M1���֞�16���ȅ^��ÿ���ȅ^��4�K���K0���K1���K2���K3���M��
//�҂�Ҳ��16���ȅ^��64���K���^����ַ��̖0~63
//��0�ȅ^�ĉK0�����^����ַ0�K��������춴�ŏS�̴��a���ѽ��̻������ɸ���
//ÿ���ȅ^�ĉK0���K1���K2�锵���K������춴�Ŕ���
//ÿ���ȅ^�ĉK3����ƉK���^����ַ�K3��7��11....�����������ܴaA����ȡ���ơ��ܴaB��

/*******************************
*����˵����
*1--SS  <----->PF0
*2--SCK <----->PB13
*3--MOSI<----->PB15
*4--MISO<----->PB14
*5--����
*6--GND <----->GND
*7--RST <----->PF1
*8--VCC <----->VCC
************************************/
/*ȫ�ֱ���*/

extern u8 ov_sta;	//��exit.c���涨��
extern u8 ov_frame;	//��timer.c���涨��

const u8* portnum="8086";
FIL fil;
FRESULT res;
UINT bww;
char buf[20];
char buf_save[20];

u8 ipbuf[16]; 	//IP����
u8 *p;

int flag = 0;
int flagg = 0;
char num[10];
//unsigned char num;
unsigned char CT[2];//������
//unsigned char SN[5]; //����
char SN[5];//����
char name[7] = {"000.txt"};
//unsigned char *SN4; //���ŵļ���
unsigned char RFID[16];			//���RFID 
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
//��������
void ShowID(u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor);	 //��ʾ���Ŀ��ţ���ʮ��������ʾ
void PutNum(u16 x,u16 y, u32 n1,u8 n0, u16 charColor, u16 bkColor);	//��ʾ����
void Store(u8 *p,u8 store,u8 cash);//����Ҫ��һ������

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

//����LCD��ʾ(OV7725)
void OV7725_camera_refresh(void)
{
	u32 i,j;
 	u16 color;	 
	if(ov_sta)//��֡�жϸ���
	{
		LCD_Scan_Dir(U2D_L2R);//���ϵ���,������
		LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//����ʾ�������õ���Ļ����
		if(lcddev.id==0X1963)
			LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,240,320);//����ʾ�������õ���Ļ����
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
		OV7725_RRST=0;				//��ʼ��λ��ָ�� 
		OV7725_RCK_L;
		OV7725_RCK_H;
		OV7725_RCK_L;
		OV7725_RRST=1;				//��λ��ָ����� 
		OV7725_RCK_H; 
		for(i=0;i<240;i++)
		{
			for(j=0;j<320;j++)
			{
				OV7725_RCK_L;
				color=GPIOC->IDR&0XFF;	//������
				OV7725_RCK_H; 
				color<<=8;  
				OV7725_RCK_L;
				color|=GPIOC->IDR&0XFF;	//������
				OV7725_RCK_H; 
				LCD->LCD_RAM=color;  
			}
		}
 		ov_sta=0;					//����֡�жϱ��
		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ��ɨ�跽�� 
	} 
}

//����LCD��ʾ(OV7670)
void OV7670_camera_refresh(void)
{
	u32 j;
 	u16 color;	 
	if(ov_sta)//��֡�жϸ���
	{
		LCD_Scan_Dir(U2D_L2R);//���ϵ���,������  
		if(lcddev.id==0X1963)LCD_Set_Window((lcddev.width-240)/2,(lcddev.height-320)/2,240,320);//����ʾ�������õ���Ļ����
		else if(lcddev.id==0X5510||lcddev.id==0X5310)LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//����ʾ�������õ���Ļ����
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
		OV7670_RRST=0;				//��ʼ��λ��ָ�� 
		OV7670_RCK_L;
		OV7670_RCK_H;
		OV7670_RCK_L;
		OV7670_RRST=1;				//��λ��ָ����� 
		OV7670_RCK_H;
		for(j=0;j<76800;j++)
		{
			OV7670_RCK_L;
			color=GPIOC->IDR&0XFF;	//������
			OV7670_RCK_H; 
			color<<=8;  
			OV7670_RCK_L;
			color|=GPIOC->IDR&0XFF;	//������
			OV7670_RCK_H; 
			LCD->LCD_RAM=color;    
		}   							  
 		ov_sta=0;					//����֡�жϱ��
		ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ��ɨ�跽�� 
	} 
}
 
//�ļ������������⸲�ǣ�
//��ϳ�:����"0:PHOTO/PIC13141.bmp"���ļ���
void camera_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:PHOTO/PIC%05d.bmp",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
		if(res==FR_NO_FILE)break;		//���ļ���������=����������Ҫ��.
		index++;
	}
}

//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(SRAMIN,tfileinfo.lfsize);//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//ȡ����λ,�����ǲ���ͼƬ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	return rval;
}

int main(void)
{		
	
     u8 timex=0; 
	//u8 ipbuf[16]; 	//IP����
	//u8 *p;
	//u16 t=999;		//���ٵ�һ�λ�ȡ����״̬
	u16 rlen=0;
	u8 constate=0;	//����״̬
	
	
	
	unsigned char status;
	unsigned char s=0x08;
	u16 led0pwmval=0;
	//u8 t;
	u8 key;
	u8 j;
	u16 curindex;		//ͼƬ��ǰ����
	u8 *fn;   			//���ļ���
      u16 temp;  
	
	
	
	DIR picdir;	 		//ͼƬĿ¼
	FILINFO picfileinfo;//�ļ���Ϣ
	int i = 0;
	u32 total,free;
      u8 t=0; 
      u8 res=0;  
      u8 *pname;              //��·�����ļ���
      u8 sensor=0; 	
	u16 totpicnum; 		//ͼƬ�ļ�����
	u16 *picindextbl;	//ͼƬ������ 


	p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
	
 	delay_init();	    	 //��ʱ������ʼ��	  
      NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
 	//OUTPUT_Init();			 //���ģ���ʼ��
	uart_init(115200);				
	LCD_Init();
	KEY_Init();
	InitRc522();				//��ʼ����Ƶ��ģ��
	BEEP_Init();
	LED_Init();
	tp_dev.init();              //��ʼ��������
	usart3_init(115200);        //��ʼ������3 
	usmart_dev.init(72);        //��ʼ��USMART    
	W25QXX_Init();              //��ʼ��W25Q128
	VS_Init();	  				//��ʼ��VS1053 
      my_mem_init(SRAMIN);        //��ʼ���ڲ��ڴ��
	SD_Init();
	
	exfuns_init();                          //Ϊfatfs��ر��������ڴ�                 
      f_mount(fs[0],"0:",1);                  //����SD�� 
      res=f_mount(fs[1],"1:",1);              //����FLASH.  
	//sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣
	//LEDA=1;
       OV7725_Init();
	font_init();
	res=f_mkdir("0:/PHOTO");        //����PHOTO�ļ���
	pname=mymalloc(SRAMIN,30);  //Ϊ��·�����ļ�������30���ֽڵ��ڴ�
      sensor=OV7725;	

      
	
	TPAD_Init(6);			//��ʼ����������
     
	piclib_init();										//��ʼ����ͼ	 
      ai_load_picfile("0:/PICTURE/1.jpg",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ 	
	mp3_play();
	while(!TPAD_Scan(0))//�ɹ�������һ��������(�˺���ִ��ʱ������15ms
	{
		
		ai_load_picfile("0:/PICTURE/1.jpg",0,0,lcddev.width,lcddev.height,1);//��ʾͼƬ 
	     Show_Str(80,60,240,24,"�����ִ������°���������ϵͳ!",12,0);
           delay_ms(1000);
		LCD_Fill(80,60,300,84,WHITE);
		
	}	
	printf("there is a gay open the system!!");
	
	
	LCD_Clear(WHITE);
	    // BEEP=!BEEP;		//LED1ȡ��
	LEDB =!LEDB;
	OV7725_Window_Set(320,240,1);//VGAģʽ���
     
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
	atk_8266_send_cmd("AT+CIPMUX=0","OK",20);   //0�������ӣ�1��������
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��TCP������
	atk_8266_send_cmd(p,"OK",200);
	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //����ģʽΪ��͸��		

	LEDB =!LEDB;
	//BEEP=!BEEP;
	atk_8266_get_wanip(ipbuf);//������ģʽ,��ȡWAN IP
	sprintf((char*)p,"IP��ַ:%s �˿�:%s",ipbuf,(u8*)portnum);
		LCD_ShowString(0,0,200,16,16,"RFID_Beginning!!");
           USART3_RX_STA=0;
   // p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
      mp3_play3();
    
    
    /***********************************************/
	//����KEY0�洢���ţ�����Ϊ00x.txt
	//�������е�ʶ�𿨺ŵ�user.txt��
	//�����Ѿ�д��(��Ϣ����ȷ)
	//���������Ѿ������ˣ�ע��RX��TX����壨���Ӳ��Ǻ��ȶ���ע�ⲻҪ�Ӵ���������
	//������115200
	//�Ѿ������˶������3.3V���磩��
	//�Ѽ�������������������Լ��洢�Ĺ��ܡ�
	//�Ѽ���TPAD���������Ͽ���ͼƬ��һ��ʼ��ʾ����ͼƬ������TPAD���󣬽�����ҳ�棩��
	//����λ����(�����ַ�'1'����ת�����)
	//WIFIģ���Ѿ����ϣ��ֻ�����ѡ��ΪTCP SERVER��STM32��ΪTCP �ͻ��ˡ��Զ�����
	//�Ѿ���������ģ�顣
	//��������APP����Ϣ���ݡ�
	
	//��������
	
	/************************************************/
  	while(1) 
	{		
		key=KEY_Scan(0);
         /////////////////////////////////////////////////////////////////////
		if(key==KEY_LEFT||key==KEY_DOWN||key==KEY_RIGHT)	//KEY0 �������� 
		{
				

				atk_8266_quit_trans();
				atk_8266_send_cmd("AT+CIPSEND","OK",20);       //��ʼ͸��
				sprintf((char*)p,"����Զ,����");//��������
				//Show_Str(30+54,100,200,12,p,12,0);
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
			//LCD_Fill(30+54,115,239,130,WHITE);
			//POINT_COLOR=BRED;
			//Show_Str(30+54,115,156,12,p,12,0); 			//��ʾ���յ������ݳ���
			POINT_COLOR=BLUE;
			LCD_Fill(0,130,45,220,WHITE);
			Show_Str(0,130,180,190,USART3_RX_BUF,12,0);//��ʾ���յ�������  
			USART3_RX_STA=0;
			if(constate!='+')t=1000;		//״̬Ϊ��δ����,������������״̬
			else t=0;                   //״̬Ϊ�Ѿ�������,10����ټ��
			if(USART3_RX_BUF[0] == '1')
			{
				Show_Str(0,150,180,190,"wifi����",12,0);//��ʾ���յ������� 
				TIM3_PWM_Init(9999,71);  //50Hz 10000��ֵ
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
		if(t==1000)//����10����û���յ��κ�����,��������ǲ��ǻ�����.
		{
			constate=atk_8266_consta_check();//�õ�����״̬
			if(constate=='+')Show_Str(30+30,80,200,12,"���ӳɹ�",12,0);  //����״̬
			else Show_Str(30+30,80,200,12,"����ʧ��",12,0); 	  	 
			t=0;
		}
		atk_8266_at_response(1);
		/////////////////////////////////////////////////////////////////////////////////////
                  //atk_8266_wifiap_test();

			if(USART_RX_BUF[0] == '1'&&flagg == 0)
			{
					TIM3_PWM_Init(9999,71);  //50Hz 10000��ֵ
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
			
			
		
		      
	       	

			TIM6_Int_Init(10000,7199);          //10Khz����Ƶ��,1�����ж�                                     
			EXTI8_Init();  
		      OV7725_camera_refresh();//������ʾ

               
	 	
			status = PcdRequest(PICC_REQALL,CT);//����  //�˴�������

			
			if(status==MI_OK)//�����ɹ�
			{
				
				LCD_ShowString(0,0,200,16,16,"PcdRequest_MI_OK");
				status=MI_ERR;
				status = PcdAnticoll(SN);//����ײ		
			}
			
			f_open(&fil,"0:/TXT/num.txt",FA_READ);
			f_read(&fil, buf ,1,&bww);
			f_close(&fil);
				
			num[0] = (buf[0]);

			if((TPAD_Scan(0))&&(i==0))
			{

						name[2]=0X30+num[0]-'0'; //ת��ASCII�ַ�
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

			if (status==MI_OK )//���nײ�ɹ�
			{

				LCD_ShowString(150,0,200,16,16,"PcdAnticoll_MI_OK");
				/****************************���������BUG��********************************/
				      TIM3_PWM_Init(9999,71);  //50Hz 10000��ֵ
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
				ShowID(0,20,SN,BLUE,WHITE); //��Һ��������ʾ����ID��
				printf("The Card ID is:");

				LEDB=0;  //B��
				delay_ms(300);
				
				LCD_ShowString(0,56,140,16,16,"The Card ID is:");
				for(j=0;j<4;j++)
				{
					 LCD_ShowNum(150+32*j,56,SN[j],3,16);
					 
                        		
				}
				
				SN[4] = '\0';
				
			      printf("ID:%02x %02x %02x %02x\r\n",SN[0],SN[1],SN[2],SN[3]);//���Ϳ���
				printf("ID:%s\r\n",SN);
				
				f_open(&fil,"0:/TXT/user.txt", FA_WRITE|FA_OPEN_ALWAYS);
				f_lseek(&fil, f_size(&fil)); 
				f_write (&fil, "user :",6, &bww);
				f_lseek(&fil, f_size(&fil)); 
				f_write (&fil, SN,4, &bww);
				f_close(&fil);
				printf("your message already in SD_card!!!\r\n");
				
				
				camera_new_pathname(pname);//�õ��ļ���
				bmp_encode(pname,(lcddev.width-240)/2,(lcddev.height-320)/2,240,320,0);

				
				BEEP = 1;

                        //////////////////////////////////////////////////////
				atk_8266_quit_trans();
				atk_8266_send_cmd("AT+CIPSEND","OK",20);       //��ʼ͸��
				sprintf((char*)p,"%02x%02x%02x%02x",SN[0],SN[1],SN[2],SN[3]);//��������
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
				myfree(SRAMIN,picfileinfo.lfname);	//�ͷ��ڴ�			    
				myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
				myfree(SRAMIN,picindextbl);			//�ͷ��ڴ�
				

                       			
			}
			else
			{

				LEDB=1;
				BEEP = 0;
			}
			
		      if(status==MI_OK)//�x���ɹ�
			{
				LCD_ShowString(0,48,200,16,16,"PcdSelect_MI_OK  ");
				status=MI_ERR;
				status =PcdAuthState(0x60,0x09,KEY,SN);
			}
			if(status==MI_OK)//��C�ɹ�
			{
				LCD_ShowString(0,64,200,16,16,"PcdAuthState_MI_OK  ");
				status=MI_ERR;
				status=PcdRead(s,RFID);

			}

			if(status==MI_OK)//�x���ɹ�
			{
				LCD_ShowString(0,80,200,16,16,"READ_MI_OK");
				status=MI_ERR;
				delay_ms(100);
			}

	
	}
		
}


/*************************************
*�������ܣ���ʾ���Ŀ��ţ���ʮ��������ʾ
*������x��y ����
*		p ���ŵĵ�ַ
*		charcolor �ַ�����ɫ
*		bkcolor   ��������ɫ
***************************************/

void ShowID(u16 x,u16 y, u8 *p, u16 charColor, u16 bkColor)	 //��ʾ���Ŀ��ţ���ʮ��������ʾ
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
*�������ܣ���p��n����
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
 
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD     
	W25QXX_Init();				//��ʼ��W25Q128
 	VS_Init();	  				//��ʼ��VS1053 
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH.
	POINT_COLOR=RED;       
 	/*while(font_init()) 				//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
	}
 	Show_Str(30,50,200,16,"ս�� STM32������",16,0);				    	 
	Show_Str(30,70,200,16,"���ֲ�����ʵ��",16,0);				    	 
	Show_Str(30,90,200,16,"����ԭ��@ALIENTEK",16,0);				    	 
	Show_Str(30,110,200,16,"2015��1��20��",16,0);
	Show_Str(30,130,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
	Show_Str(30,150,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);*/
	/*while(1)
	{
  		/*LED1=0; 	  
		Show_Str(30,170,200,16,"�洢������...",16,0);
		printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//��ӡRAM���Խ��	    
		Show_Str(30,170,200,16,"���Ҳ�����...",16,0); 	 	 
 		VS_Sine_Test();	   
		Show_Str(30,170,200,16,"<<���ֲ�����>>",16,0); 		 
		LED1=1;*/
		/*mp3_play();
	} 	   										    
}
*/






































