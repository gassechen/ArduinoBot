/* This is an IRC chat bot, originaly writen to connect to Twitch and be a chat control bot.  I have taken code from several places online including 
http://www.hcidata.info/host2ip.htm
The Telnet Client from the example in the Arduino Library created 14 Sep 2010, modified 9 Apr 2012, by Tom Igoe
The Arduino Web Client by David A. Mellis created 18 Jan 2011 and modified by Keiran "Affix" Smith <affix@affix.me>
Modified and re-writen by
Les "Bruteclaw" Holdeman March 2015
les.holdeman@gmail.com
Updated to add lcd screen support in October 2015
December 2015 - Bug fix on lcd screen locking up.
January 2016 - Larger LCD screen update.
November 2016 - Adding a real time clock  From the DS3231 example file included with the RTC by Makuna library
December 2016 - Added a Serial MP3 player
Febuary 2017 - Updated the RTC info after library was updated.  Also added a channel change system and removed IP address from Display
March 2017 - Cleaned up some of the inculded libraries that were not being used.  Cleaned up MP3 Player comands to free memory and reset volume to new setup
Aug 2017 - Crude Multichannel support added, fixed the server connection settings
Oct 2017 - Recovered source files after they were deleted from my local system.  Cleaned up the command section to take less memory.  Moved config variable off to a second file.
Feb 2018 - Uploaded to GitHub and updated source command
Backend commands not listed to the public:
%chng - Change the current channel that the bot is talking on
%clsc - Clear the LCD screen
%load - Load info onto the LCD screen
%blon - Turn on the LCD backlight
%blof - Turn off the LCD backlight
Connections settings for server, user name and password configured in config.h 
*/

//Includes
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#include "config.h"
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>

//Pin Setup



const char* ssid     = "SkayNet";
const char* password = "";

const char* host = "192.168.10.104";
const int buttonPin = 0;
int buttonState;             // the current reading from the input pin

WiFiClient client;

void setup()
{
  //Set Strings
  channel = channel1;
  pinMode(buttonPin, INPUT);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
  //Set Pin modes
  
  

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  /* this check is only needed on the Leonardo.  Commented out for use on Uno
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only.
  }*/
  Serial.println("Serial Port Opened");

  //display current time in RTC
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);
  
   
}

void loop()
{
  //Connecting to the IRC server and channel
  if (!client.connected())
  {
    Serial.print("Connecting to ");
    Serial.println(server);
    
    if (client.connect(serverC, port)) //Could not get this command to use a string var, so used a char var
    {
      Serial.println("Connected to Server");
      delay(1000);
      client.print("PASS ");
      client.println(pass);
      Serial.println("Password has been Sent");
      delay(500);
      client.print("NICK ");
      client.println(nick);
      Serial.println("Nickname has been set");
      delay(500);
      client.print("USER ");
      client.println(user);
      Serial.println("User Name has been Sent");
      delay(500);
      Serial.print("Joining Channels ");
      Serial.println(channel1);
      client.print("JOIN ");
      client.println(channel1);
      delay(500);
      Serial.println(channel2);
      client.print("JOIN ");
      client.println(channel2);
      delay(500);
      Serial.println("Channels Joined");
      Serial.println("====Connection to server Completed====");
      
      handle_irc_connection();
    }
    else
    {
      //If you didn't get a connection to the server, wait 2 minutes and try again.
      Serial.println("Connection Failed");
      Serial.println("Waiting for 2 minutes to try again");
      
      delay(20000);
    }
  }
}

//commands function
void command(String response, String title)
{
          client.print("PRIVMSG ");
          client.print(channel);
          client.print(" :");
          client.println(response);
          Serial.println(title);
          
}



//IRC handeling function
void handle_irc_connection()
{
  char c;
  String msg = "";
  client.print("PRIVMSG ");
  client.print(channel);
  client.print(" :");
  client.println(".me has arrived!");
  
  while (true)
  {
    //Allowing Serial console to send messages back to the server
    if (Serial.available() > 0)
    {
      while (Serial.available() > 0)
      {
        msg += char(Serial.read());
        delay(25);
      }
      client.print("PRIVMSG ");
      client.print(channel);
      client.print(" :");
      client.println(msg);
      Serial.println(msg); //Allows for feedback to see what was sent
      msg = ""; //Clears the variable after it has been sent
    }

    //If server connection fails, stop this function and return to attempting to connect again
    if (!client.connected())
    {
      return;
    }

    //Handeling the IRC traffic and commands
    if (client.available())
    {
      //Print IRC channel traffic to serial console
      char c = client.read();
      Serial.print(c);

      //Ping Pong IRC keep alive
      if (c == 'P')
      {
        char buf[5];
        memset(buf, 0, sizeof(buf));
        buf[0] = c;
        for (int i = 1; i < 4; i++)
        {
          c = client.read();
          buf[i] = c;
        }
        if (strcmp(buf, "PING") == 0)
        {
          client.println("PONG\r");
          Serial.println(buf);
          Serial.println("Ping Pong Played");
        }
      }

      //Command Preface Character
      if (c == '%')
      {
        char buf[6];
        memset(buf, 0, sizeof(buf));
        buf[0] = c;
        for (int i = 1; i < 5; i++)
        {
          c = client.read();
          buf[i] = c;
        }

        //Commands Section
/*
        if (strcmp(buf, "<command>") == 0)
        {
          command("<response>", "<title>");
        }
 */

        //Help
        if (strcmp(buf, "%help") == 0)
        {
          command("Please use %com1 to see the list of my commands.", "Help");
        }
        
        //Testing
        if (strcmp(buf, "%test") == 0)
        {
          command("Test Failed Successfully", "Testing");
        }

        //Information
        if (strcmp(buf, "%info") == 0)
        {
          command("I am Version 3.5 of a bot built on the Arduino Mega.  Programmed by BruteClaw.  Currently only usefull as a command bot.  More features may be added at a later time.", "Information");
        }

        //Source Code
        if (strcmp(buf, "%srce") == 0)
        {
          command("My source code can be found at: https://github.com/BruteClaw/ArduinoBot", "Source Code");
        }

        //Commands 1
        if (strcmp(buf, "%com1") == 0)
        {
          command("My Current commands are: %test - Test Message, %info - Information About me, %srce - My Source Code, %time - Time where I currently am, %ston - Get the streamers attention, %stof - Turn off the strobe, %snd1 & snd0 - Start and stop music", "Commands 1");
        }
        

/*        if (buttonState == LOW) 
        {
        // turn LED on:
          Serial.println("ON");
          command("ESTOT EN EMERGENCIA", "S.O.S");
             
        }

        
        //Strobe Light On 
        if (strcmp(buf, "%ston") == 0)
        {
          command("Strobe Light On", "Strobe On");
          digitalWrite(strbPin, HIGH);
        }

        //Strobe Light Off
        if (strcmp(buf, "%stof") == 0)
        {
          command("Strobe Light Off", "Strobe Off");
          digitalWrite(strbPin, LOW);
         
        }

        //Backlight On
        if (strcmp(buf, "%blon") == 0)
        {
          command("Backlight On", "Backlight On");
          digitalWrite(backLight1, HIGH);
        }

        //Backlight Off
        if (strcmp(buf, "%blof") == 0)
        {
          command("Backlight Off", "Backlight Off");
          digitalWrite(backLight1, LOW);
        }

        //Reloading the LCD
        if (strcmp(buf, "%load") == 0)
        {
          command("Screen Reloaded","Screen Reloaded");
        }

     */  

        //Clear Screen
        if (strcmp(buf, "%clsc") == 0)
        {
          client.print("PRIVMSG ");
          client.print(channel);
          client.println(" :Screen Cleared");
          Serial.println("Clear Screen");
          
        }
        
        //Channel changing
        if (strcmp(buf, "%chng") == 0)
        {
          client.print("PRIVMSG ");
          client.print(channel);
          client.println(" :I am changing to another channel");
          if (channel == channel1)
          {
            channel = channel2;
          }
          else if (channel == channel2)
          {
            channel = channel1;
          }
          client.print("PRIVMSG ");
          client.print(channel);
          client.println(" :I have changed to this channel");
          Serial.println("Channel Change");
          
        }


        //End of commands
        delay(300);
      }
    }
  }
}
