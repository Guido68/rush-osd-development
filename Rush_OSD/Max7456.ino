
#define DATAOUT 11		//MOSI
#define DATAIN  12		//MISO
#define SPICLOCK  13		//sck

//#define MAX7456SELECT 6	//ss 
#define MAX7456SELECT 10	//ss 

#define MAX7456RESET 9		//RESET
#define VSYNC 2			// INT0

//MAX7456 opcodes
#define DMM_reg   0x04
#define DMAH_reg  0x05
#define DMAL_reg  0x06
#define DMDI_reg  0x07
#define VM0_reg   0x00
#define VM1_reg   0x01

// video mode register 0 bits
#define VIDEO_BUFFER_DISABLE 0x01
#define MAX7456_RESET 0x02
#define VERTICAL_SYNC_NEXT_VSYNC 0x04
#define OSD_ENABLE 0x08
#define SYNC_MODE_AUTO 0x00
#define SYNC_MODE_INTERNAL 0x30
#define SYNC_MODE_EXTERNAL 0x20
#define VIDEO_MODE_PAL 0x40
#define VIDEO_MODE_NTSC 0x00

// video mode register 1 bits


// duty cycle is on_off
#define BLINK_DUTY_CYCLE_50_50 0x00
#define BLINK_DUTY_CYCLE_33_66 0x01
#define BLINK_DUTY_CYCLE_25_75 0x02
#define BLINK_DUTY_CYCLE_75_25 0x03

// blinking time
#define BLINK_TIME_0 0x00
#define BLINK_TIME_1 0x04
#define BLINK_TIME_2 0x08
#define BLINK_TIME_3 0x0C

// background mode brightness (percent)
#define BACKGROUND_BRIGHTNESS_0 0x00
#define BACKGROUND_BRIGHTNESS_7 0x01
#define BACKGROUND_BRIGHTNESS_14 0x02
#define BACKGROUND_BRIGHTNESS_21 0x03
#define BACKGROUND_BRIGHTNESS_28 0x04
#define BACKGROUND_BRIGHTNESS_35 0x05
#define BACKGROUND_BRIGHTNESS_42 0x06
#define BACKGROUND_BRIGHTNESS_49 0x07

#define BACKGROUND_MODE_GRAY 0x40

//MAX7456 commands
#define CLEAR_display 0x04
#define CLEAR_display_vert 0x06
#define END_string 0xff

#define WHITE_level_80 0x03
#define WHITE_level_90 0x02
#define WHITE_level_100 0x01
#define WHITE_level_120 0x00


#if defined(VideoSignalType_PAL) 
#define ENABLE_display 0x48
#define ENABLE_display_vert 0x4c
#define MAX7456_reset 0x42
#define DISABLE_display 0x40
#define MAX_screen_size 480
#define MAX_screen_rows 16
#endif

#if defined(VideoSignalType_NTSC)
#define ENABLE_display 0x08
#define ENABLE_display_vert 0x0c
#define MAX7456_reset 0x02
#define DISABLE_display 0x00
#define MAX_screen_size 390
#define MAX_screen_rows 13
#endif



#define MAX7456ADD_VM0          0x00  //0b0011100// 00 // 00             ,0011100
#define MAX7456ADD_VM1          0x01   
#define MAX7456ADD_HOS          0x02
#define MAX7456ADD_VOS          0x03
#define MAX7456ADD_DMM          0x04
#define MAX7456ADD_DMAH         0x05
#define MAX7456ADD_DMAL         0x06
#define MAX7456ADD_DMDI         0x07
#define MAX7456ADD_CMM          0x08
#define MAX7456ADD_CMAH         0x09
#define MAX7456ADD_CMAL         0x0a
#define MAX7456ADD_CMDI         0x0b
#define MAX7456ADD_OSDM         0x0c
#define MAX7456ADD_RB0          0x10
#define MAX7456ADD_RB1          0x11
#define MAX7456ADD_RB2          0x12
#define MAX7456ADD_RB3          0x13
#define MAX7456ADD_RB4          0x14
#define MAX7456ADD_RB5          0x15
#define MAX7456ADD_RB6          0x16
#define MAX7456ADD_RB7          0x17
#define MAX7456ADD_RB8          0x18
#define MAX7456ADD_RB9          0x19
#define MAX7456ADD_RB10         0x1a
#define MAX7456ADD_RB11         0x1b
#define MAX7456ADD_RB12         0x1c
#define MAX7456ADD_RB13         0x1d
#define MAX7456ADD_RB14         0x1e
#define MAX7456ADD_RB15         0x1f
#define MAX7456ADD_OSDBL        0x6c

volatile byte writeOK;
volatile byte valid_string;
volatile byte save_screen;
volatile int  incomingByte;
volatile int  count;

//////////////////////////////////////////////////////////////
byte spi_transfer(volatile byte data)
{
  SPDR = data;			  // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;			  // return the received byte
}



// ============================================================   WRITE TO SCREEN

void MAX7456Setup(void)
{
  byte spi_junk, eeprom_junk;
  int x;
  pinMode(MAX7456RESET,OUTPUT);
  digitalWrite(MAX7456RESET,HIGH); //hard enable

  delay(250);

  pinMode(MAX7456SELECT,OUTPUT);
  digitalWrite(MAX7456SELECT,HIGH); //disable device
  
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(VSYNC, INPUT);

  // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (4 meg)
 
  SPCR = (1<<SPE)|(1<<MSTR);
  spi_junk=SPSR;
  spi_junk=SPDR;
  delay(250);

  // force soft reset on Max7456
  digitalWrite(MAX7456SELECT,LOW);
  spi_transfer(VM0_reg);
  spi_transfer(MAX7456_reset);
  digitalWrite(MAX7456SELECT,HIGH);
  delay(500);

  // set all rows to same charactor white level, 90%
  digitalWrite(MAX7456SELECT,LOW);
  for (x = 0; x < MAX_screen_rows; x++)
  {
    spi_transfer(x + 0x10);
    spi_transfer(WHITE_level_120);
  }

  // make sure the Max7456 is enabled
  spi_transfer(VM0_reg);


#if defined VideoSignalType_NTSC
  spi_transfer(OSD_ENABLE|VIDEO_MODE_NTSC);  
#endif

#if defined VideoSignalType_PAL
  spi_transfer(OSD_ENABLE|VIDEO_MODE_PAL);  
#endif   
  digitalWrite(MAX7456SELECT,HIGH);
  delay(100);


}

// Copy string from ram into screen buffer
void MAX7456_WriteString(const char *string, int Adresse)
{
  int xx;

  for(xx=0;string[xx]!=0;)
  {
    screen[Adresse++] = string[xx++];
  }
}

// Copy string from progmem into the screen buffer
void MAX7456_WriteString_P(const char *string, int Adresse)
{
  int xx = 0;
  char c;
  while((c = (char)pgm_read_byte(&string[xx++])) != 0)
  {
    screen[Adresse++] = c;
  }
}

void MAX7456_DrawScreen()
{
  int xx;
#if defined(DISPLAY_DEBUG_MODE)
  for(xx=0;xx<MAX_screen_size;xx++)
  {
    Screen[xx]=0xff;
  }
  DisplayDebugScreen(); 
#endif

  digitalWrite(MAX7456SELECT,LOW);
  for(xx=0;xx<MAX_screen_size;++xx)
  {
    MAX7456_Send(MAX7456ADD_DMAH, xx>>8);
    MAX7456_Send(MAX7456ADD_DMAL, xx);
    MAX7456_Send(MAX7456ADD_DMDI, screen[xx]);
    screen[xx] = ' ';
  }
  digitalWrite(MAX7456SELECT,HIGH);
}

void MAX7456_Send(int add, char data)
{
  spi_transfer(add);
  spi_transfer(data);
}
