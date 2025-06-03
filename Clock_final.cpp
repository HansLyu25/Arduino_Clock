#include"Arduino.h"
#include <DS1302.h>
#include <Wire.h>
#include <TM1650.h>

int tonePin=7;//蜂鸣器的pin
int brightness=7;//亮度
TM1650 d;//屏幕类
DS1302 rtc(4,5,6);//对应DS1302的RST,DAT,CLK
Time now(2025,6,3,21,33,30,2); // 时间类，使用合适的初始值
int hr;//闹钟的小时
int fakehr;//临时储存闹钟的小时
int minl;//闹钟的分钟
int fakeminl;//临时储存闹钟的分钟
bool sing=true;//闹铃标志
char s[5];
int state=0;//状态,共10个
unsigned long lastsetTime = 0;// 上次设置时间
unsigned long lastBlinkTime = 0;  // 闪烁计时器
bool blinkState = true;           // 闪烁状态

char zhuan(int k)//将一位数int类型的数字转化成char类型
{
  switch(k)
  {
    case 1:return '1';
    case 0:return '0';
    case 2:return '2';
    case 3:return '3';
    case 4:return '4';
    case 5:return '5';
    case 6:return '6';
    case 7:return '7';
    case 8:return '8';
    case 9:return '9';
  }
}
void initRTCTime()//初始化时间
{
    rtc.writeProtect(false);  // 关闭写保护
    rtc.halt(false);          // 启动时钟
  
    // 检查DS1302是否有有效时间
    Time t = rtc.time();
    if(t.yr < 2020 || t.yr > 2050) 
    {
        // 如果时间无效，设置为当前时间
        rtc.time(now);
    }
    rtc.writeProtect(true);  // 恢复写保护
}
void printTime()//在串口打印时间
{
  Serial.print(now.yr);
  Serial.print("-");
  Serial.print(now.mon);
  Serial.print("-");
  Serial.print(now.date);
  Serial.print(" ");
  Serial.print(now.hr);
  Serial.print(":");
  Serial.print(now.min);
  Serial.print(":");
  Serial.println(now.sec);
}
void writetime()//将修改的时间写入芯片
{
    rtc.writeProtect(false);  // 关闭写保护
    rtc.time(now);            // 写入时间
    rtc.writeProtect(true);   // 恢复写保护
  
    // 保存闹钟时间到临时变量
    fakehr = hr;
    fakeminl = minl;
}
void setstatechange()//修改相应状态以及sing 按键1
{
  unsigned long last=millis();
  if(last-lastsetTime>=200)
  {
    sing=false;
    state=(state+1)%11;
    lastsetTime=last;
    if(state == 0) 
    {
      writetime();  // 写入修改后的时间
      sing = true;  // 恢复闹钟开关
    }
  }
  
}
void upstatechange()//增加值 按键2
{
    unsigned long last=millis();
  if(last-lastsetTime>=200)
  {
    switch(state)
    {
    case 3:
        now.yr++;
        break;
    case 4:
        now.mon = (now.mon % 12) + 1;
        break;
    case 5:
        {// 增加日期
        now.date += 1;
        int maxDays = 31;// 获取当前月份的最大天数
        if (now.mon == 4 || now.mon == 6 || now.mon == 9 || now.mon == 11) {maxDays = 30;} 
        else if (now.mon == 2) 
        {
        bool isLeapYear = (now.yr % 4 == 0 && now.yr % 100 != 0) || (now.yr % 400 == 0);
        maxDays = isLeapYear ? 29 : 28;
        }
        if (now.date > maxDays) {now.date = 1;}// 如果日期超过当前月份最大天数，则重置为1
        break;
        }
    case 6:
        now.hr = (now.hr + 1) % 24;
        break;
    case 7:
        now.min = (now.min + 1) % 60;
        break;
    case 8:
        hr = (hr + 1) % 24;
        break;
    case 9:
        minl = (minl + 1) % 60;
        break;
    case 10:
        brightness=(brightness %7 ) + 1;
        break;
    }
    lastsetTime=last;
  }
}
void showtime(int mode)
{
  switch(mode)
  {
    case 0:  // 显示年份
      s[0] = zhuan(now.yr / 1000);
      s[1] = zhuan((now.yr / 100) % 10);
      s[2] = zhuan((now.yr / 10) % 10);
      s[3] = zhuan(now.yr % 10);
      break;
      
    case 1:  // 显示月日
      s[0] = zhuan(now.mon / 10);
      s[1] = zhuan(now.mon % 10);
      s[2] = zhuan(now.date / 10);
      s[3] = zhuan(now.date % 10);
      break;
      
    case 2:  // 显示时分
      s[0] = zhuan(now.hr / 10);
      s[1] = zhuan(now.hr % 10) | 128;  // 中间显示冒号
      s[2] = zhuan(now.min / 10);
      s[3] = zhuan(now.min % 10);
      
      break;
      
    case 3:  // 设置年份
      s[0] = zhuan(now.yr / 1000);
      s[1] = zhuan((now.yr / 100) % 10);
      s[2] = zhuan((now.yr / 10) % 10);
      s[3] = zhuan(now.yr % 10);
      break;
      
    case 4:  // 设置月份
      s[0] = zhuan(now.mon / 10);
      s[1] = zhuan(now.mon % 10);
      s[2] = zhuan(now.date / 10);
      s[3] = zhuan(now.date % 10);
      break;
      
    case 5:  // 设置日期
      s[0] = zhuan(now.mon / 10);
      s[1] = zhuan(now.mon % 10);
      s[2] = zhuan(now.date / 10);
      s[3] = zhuan(now.date % 10);
      break;
      
    case 6:  // 设置小时
      s[0] = zhuan(now.hr / 10);
      s[1] = zhuan(now.hr % 10) | 128;
      s[2] = zhuan(now.min / 10);
      s[3] = zhuan(now.min % 10);
      break;
      
    case 7:  // 设置分钟
      s[0] = zhuan(now.hr / 10);
      s[1] = zhuan(now.hr % 10) | 128;
      s[2] = zhuan(now.min / 10);
      s[3] = zhuan(now.min % 10);
      break;
      
    case 8:  // 设置闹钟小时
      s[0] = zhuan(hr / 10);
      s[1] = zhuan(hr % 10) | 128;
      s[2] = zhuan(minl / 10);
      s[3] = zhuan(minl % 10)| 128;
      break;
      
    case 9:  // 设置闹钟分钟
      s[0] = zhuan(hr / 10);
      s[1] = zhuan(hr % 10) | 128;
      s[2] = zhuan(minl / 10);
      s[3] = zhuan(minl % 10)| 128;
      break;
      
    case 10:  // 设置亮度
      s[0] = 'b';  // 显示"b"表示亮度
      s[1] = zhuan(brightness);
      s[2] = ' ';
      s[3] = ' ';
      break;
  }
}
void setup()
{
    Wire.begin();//I2C接口初始化
    Serial.begin(9600);//串口接口初始化
    d.init();//数码管类初始化
    d.setBrightness(brightness);
    initRTCTime();//实时时钟模块初始化

    attachInterrupt(0,setstatechange,RISING);//两个按键设置为中断方式
    attachInterrupt(1,upstatechange,RISING);
    pinMode(2,INPUT);//两个按键设置为输入模式
    pinMode(3,INPUT);

    now=rtc.time();//用芯片的时间给now时间类赋初值
    pinMode(tonePin, OUTPUT);//设置蜂鸣器的pin为输出模式
    hr=10;//设置闹钟的小时
    fakehr=10;//设置临时储存的闹钟小时
    minl=20;//设置闹钟的分钟
    fakeminl=10;//设置临时储存的闹钟分钟

}

void loop()
{
    printTime();//串口打印时间
    
    if(state<3){now=rtc.time();}//如果在第一类状态,就将芯片的时间赋给now时间类
    if(hr == now.hr&&abs(minl-now.min) <= 0.5&&sing)//判断闹铃时间是否到,并且变量sing为true,闹铃就响
    {
        for(int i = 0; i < 10; i++) 
        {
        tone(tonePin, 1000);  // 1kHz频率
        delay(100);
        noTone(tonePin);
        delay(100);
      }
    }
    if(!(hr == now.hr&&abs(minl-now.min) <= 2)){sing=true;}//没到闹铃时间,将sing改回true
    d.setBrightness(brightness);//设置亮度

    showtime(state);
    if(state<3)//如果在第一类大状态,不闪烁
    {
        d.displayString(s);
    }
    else//如果在第二类大状态,闪烁
    {
        static unsigned long lastBlinkTime = 0;
        if(millis() - lastBlinkTime > 700) 
        {
          blinkState = !blinkState;
          lastBlinkTime = millis();
        }
    
        if(blinkState) 
        {
          d.displayString(s);
        } 
        else 
        {
          switch(state)
          {
          case 4:
            {
              char temp[5];
              temp[0] = ' ';  // 月份第一位闪烁
              temp[1] = ' ';  // 月份第二位闪烁
              temp[2] = s[2]; // 保持日期第一位显示
              temp[3] = s[3]; // 保持日期第二位显示
            
              d.displayString(temp);
              break;
            } 
          case 5:
            {
              char temp[5];
              temp[0] = s[0];  
              temp[1] = s[1];  
              temp[2] = ' '; 
              temp[3] = ' '; 
            
              d.displayString(temp);
              break;
            }
          case 6:
            {
              char temp[5];
              temp[0] = ' ';  
              temp[1] = ' ';  
              temp[2] = s[2]; 
              temp[3] = s[3]; 
            
              d.displayString(temp);
              break;
            }
          case 7:
            {
              char temp[5];
              temp[0] = s[0];  
              temp[1] = s[1];  
              temp[2] = ' '; 
              temp[3] = ' '; 
            
              d.displayString(temp);
              break;
            }
          case 8:
            {
              char temp[5];
              temp[0] = ' ';  
              temp[1] = ' ';  
              temp[2] = s[2]; 
              temp[3] = s[3]| 128; 
            
              d.displayString(temp);
              break;
            }
          case 9:
            {
              char temp[5];
              temp[0] = s[0];  
              temp[1] = s[1];  
              temp[2] = ' '; 
              temp[3] = ' '| 128; 
            
              d.displayString(temp);
              break;
            }
          default: 
            {
              d.displayString("    ");  // 其他状态仍然全闪烁
            }
          }
        }
    }

    if(state>10)//完成一次循环,写入时间和闹钟,修改sing为true
    {
        writetime();
        rtc.time(now);
        state = 0;
        sing = true;
    }
    delay(100);
}