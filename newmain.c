/*
 * File:   newmain.c
 * Author: Mohammad
 *
 * Created on May 13, 2020, 10:33 AM
 */
#define _XTAL_FREQ   4000000UL 

#define LENA  PORTEbits.RE1
#define LDAT  PORTEbits.RE2
#define LPORT PORTD


#define L_ON	0x0F
#define L_OFF	0x08
#define L_CLR	0x01
#define L_L1	0x80
#define L_L2	0xC0
#define L_CR	0x0F		
#define L_NCR	0x0C	

#define L_CFG   0x38
#include <xc.h>
unsigned char r;
unsigned char rec[10];

void delay_ms(unsigned int n)
{
    int i;
    for (i=0; i < n; i++){
         __delaywdt_ms(1) ; 
    }
}
void adc_init(void)
{
#if defined (_18F452) || defined(_16F877A)
  ADCON1=0x02;
  ADCON0=0x41; 
#else
  ADCON0=0x01;
  ADCON1=0x0B;
  ADCON2=0x01;
#endif


}
unsigned int adc_amostra(unsigned char canal)
{


#if defined(_18F4620) || defined(_18F4550)
    switch(canal)
    {
      case 0: 
        ADCON0=0x01;
        break;
      case 1: 
        ADCON0=0x05;
        break;
      case 2: 
        ADCON0=0x09;
        break;
    }
#else   
     switch(canal)
    {
      case 0:
        ADCON0=0x01;
        break;
      case 1:
        ADCON0=0x09;
        break;
      case 2:
        ADCON0=0x11;
        break;
    }   
#endif
   

    ADCON0bits.GO=1;
    while(ADCON0bits.GO == 1);

   return ((((unsigned int)ADRESH)<<2)|(ADRESL>>6));
}
void serial_init(void)
{
    SPBRG=103; //baud rate de 19200 - FOSC=32MHz 
               //baud rate de 4800  - FOSC=8MHz   brgh=1 baud=FOSC/16(X+1)
               //                                 brgh=0 baud=FOSC/64(X+1)
        
	//Configuracao da serial
    TXSTAbits.TX9=0;    //transmissao em 8 bits
    TXSTAbits.TXEN=1;  //habilita transmissao
    TXSTAbits.SYNC=0;  //modo assincrono
    TXSTAbits.BRGH=1;  //high baud rate
    RCSTAbits.SPEN=1;  //habilita porta serial - rx
    RCSTAbits.RX9=0;   //recepcao em 8 bits
    RCSTAbits.CREN=1;  //recepcao continua


}
unsigned char serial_rx(unsigned int timeout)
{
  unsigned int to=0;

  if(RCSTAbits.FERR || RCSTAbits.OERR)//trata erro
  {
      RCSTAbits.CREN=0;
      RCSTAbits.CREN=1;
  }

  while(((to < timeout)||(!timeout))&&(!PIR1bits.RCIF))
  {
   delay_ms(20);
    to+=20; 
  }
  if(PIR1bits.RCIF)
    return RCREG;
  else
    return 0xA5;
}
void serial_tx(unsigned char val)
{
  TXREG=val;
  while(!TXSTAbits.TRMT);
}
void PWM1_Init(unsigned int f)
  {
   unsigned int temp;
 //PWM Period = [(PR2) + 1] * 4 * TOSC *(TMR2 Prescale Value)
 //PWM Duty Cycle = (CCPRXL:CCPXCON<5:4>) *TOSC * (TMR2 Prescale Value)


   //desliga PWM
     CCP1CON=0x00;//CCP disabled
     TRISCbits.TRISC2=1; //desliga saídas PWM
     TRISDbits.TRISD5=1;

     PORTCbits.RC2=0; //deliga saídas PWM
     PORTDbits.RD5=0;

      CCPR1L=0;//ou 255?


     //calculo TMR2

      T2CONbits.TMR2ON=0;
             
     temp=_XTAL_FREQ/(f*4l);
    
     if (temp < 256)
     {
       T2CONbits.T2CKPS=0;  //1
       PR2=temp;
     }
     else if(temp/4 < 256 )
     {
       T2CONbits.T2CKPS=1;  //4
       PR2=(temp+2)/4;
     }
     else
     {
       PR2=(temp+8)/16;
       T2CONbits.T2CKPS=2;  //16
     }

     T2CONbits.TOUTPS=0;  //1-16
     
  }


  void PWM1_Start(void)
  {

      TRISCbits.TRISC2=0; //liga saídas PWM
      TRISDbits.TRISD5=0;


      CCP1CON=0x0F; //CCP -> PWM mode 0x0F

      
      T2CONbits.TMR2ON=1;
 
      //espera PWM normalizar

      PIR1bits.TMR2IF=0;
      while(PIR1bits.TMR2IF == 0);
      PIR1bits.TMR2IF=0;
      while(PIR1bits.TMR2IF == 0);

     


  }
  void PWM1_Set_Duty(unsigned char d)
  {
      unsigned int temp;
      
      temp=(((unsigned long)(d))*((PR2<<2)|0x03))/255;

      CCPR1L= (0x03FC&temp)>>2;
      CCP1CON=((0x0003&temp)<<4)|0x0F;
  }
  void lcd_wr(unsigned char val)
{
  LPORT=val;
}
 void set_pwm1_raw(unsigned int raw_value)//raw value 0 -- 1023 corresponds to  0--100%
{
    CCPR1L = (raw_value >> 2) & 0x00FF; // Load the upper 8 bit in CCPL1
    CCP1CONbits.DC1B = raw_value & 0x0003; //first two bits in bits 4, 5 of CCP1COn
    
   // Another way for Writing to 2 LSBs of pwm duty cycle in CCPCON register
    //CCP1CON = ((unsigned char)(CCPICON1 & 0xCF) | ((raw_value & 0x0003)<<4));
}
void set_pwm1_percent(float value)// value 0--100%, corresponds to 0--1023
{
    float tmp = value*1023.0/100.0;//scale 0--100 to 0--1023
    int raw_val = (int)(tmp +0.5); // for rounding
    if ( raw_val> 1023) raw_val = 1023;// Do not exceed max value
    set_pwm1_raw(raw_val);
}
  void lcd_cmd(unsigned char val)
{
	LENA=1;
        lcd_wr(val);
        LDAT=0;
        delay_ms(3);
        LENA=0;
        delay_ms(3);
	LENA=1;
}
void lcd_init(void)
{
	LENA=0;
	LDAT=0;
	delay_ms(20);
	LENA=1;
	
	lcd_cmd(L_CFG);
	delay_ms(5);
	lcd_cmd(L_CFG);
        delay_ms(1);
	lcd_cmd(L_CFG); //configura
	lcd_cmd(L_OFF);
	lcd_cmd(L_ON); //liga
	lcd_cmd(L_CLR); //limpa
	lcd_cmd(L_CFG); //configura
        lcd_cmd(L_L1);
}
void error(){
    serial_tx('E');
}
void read(){
   
    for(int i=0;i<10;i++){
        rec[i]=serial_rx(10000);
        lcd_wr(rec[i]);
        if(rec[i]=='*'){
            return;
        }
        
    }
        
}

void main(void) {
    
    TRISD=0x00;
    serial_init();
    lcd_init();
    lcd_wr('S');
    PWM1_Init(1);
    PWM1_Set_Duty(4);
    PWM1_Start();
    adc_init();
    while(1){
        read();
        if(rec[0]=='$'){
            
            if(rec[1]=='R'){
                
                if(rec[2]=='A'){
                    unsigned int x = adc_amostra(rec[3]);
                    serial_tx(x);
                }
                else if (rec[2]=='D'){
                    unsigned int x= rec[3];
                    unsigned char z= 0x00;
                    
                    switch(x){
                        case 0 : z=0x01 && PORTD;
                        break;
                        case 1 : z=0x02 && PORTD;
                        break;
                        case 2 : z=0x04 && PORTD;
                        break;
                        case 3: z=0x01 && PORTD;
                        break;
                        case 4: z=0x01 && PORTD;
                        break;
                        case 5 : z=0x01 && PORTD;
                        break;
                        case 6 : z=0x01 && PORTD;
                        break;
                        case 7 : z=0x01 && PORTD;
                        break;
                        
                    
                    }
                    serial_tx(z);
                }
                else error();
            }
            else if(rec[1]=='W'){
                if(rec[2]=='D'){
                    unsigned int x= rec[3];
                    unsigned  z= rec[4];
                    
                    switch(x){
                        case 0 : PORTDbits.RD0=z && PORTDbits.RD0;
                        break;
                        case 1 : PORTDbits.RD1=z && PORTDbits.RD1;
                        break;
                        case 2 : PORTDbits.RD2=z && PORTDbits.RD2;
                        break;
                        case 3: PORTDbits.RD3=z && PORTDbits.RD3;
                        break;
                        case 4: PORTDbits.RD4=z && PORTDbits.RD4;
                        break;
                        case 5 : PORTDbits.RD5=z && PORTDbits.RD5;
                        break;
                        case 6 : PORTDbits.RD6=z && PORTDbits.RD6;
                        break;
                        case 7 : PORTDbits.RD7=z && PORTDbits.RD7;
                        break;
                        
                    
                    }
                    serial_tx('S');
                }
                if(rec[2]=='P'){
                    unsigned int x= rec[3];
                    unsigned int y= rec[4];
                    int z=((x*10)+y);
                    set_pwm1_percent(z);
                }
            }
            
            else error();
        }
        else{
           error(); 
        }
    }
    return;
}
