/**************************************************************************************************************/
/* RF Power Meter 900MHz - 10GHz                                                                              */
/* Designed by Joost Breed - October 2015                                                                     */
/* Based on AD8317 Log Detector of Analog Devices                                                             */
/* Datasheet: http://www.analog.com/media/en/technical-documentation/evaluation-documentation/AD8317.pdf      */
/* V1.1   2-11-2015                                                                                           */
/* - Added profiles of two commonly used attenuators for FPV equipment. Mini-Circuits VAT-20W2+ and VAT-30W2+ */
/* - Attenuation can now be set with 1 decimal accuracy.                                                      */
/**************************************************************************************************************/
#include <EEPROM.h>
#include <Bounce2.h>
#include <LiquidCrystal.h>

// Constant declarations
// LCD Pins (4 bit mode with RS and E)
const byte LCD_RS_PIN = 12; 
const byte LCD_RW_PIN = 11;
const byte LCD_E_PIN = 10;
const byte LCD_DB4_PIN = 9;
const byte LCD_DB5_PIN = 8;
const byte LCD_DB6_PIN = 7;
const byte LCD_DB7_PIN = 6;

// Button pins
const byte BTN_UP = A1;
const byte BTN_ENTER = A2;
const byte BTN_DOWN = A3;
const byte BUTTON_DEBOUNCE_MS = 5;    // Button debounce time in ms

const byte AD_POWER_PIN = A0;         // Pin used as A/D converter input, measuring power

const int SAMPLE_COUNT = 500;         // Amount of samples to take for averaging
const int SAMPLE_INTERVAL_US = 750;   // Amount of time between two samples in us. 750us so one loop is 500ms

const float V_REF = 2.048;            // External reference voltage
const float AD_RESOLUTION  = 1023.0;  // AD resolution - 1

// Menus
const byte MENU_MEASUREMENTS = 0;
const byte MENU_MIN_POWER = 1;
const byte MENU_MAX_POWER = 2;
const byte MENU_MODULATIONINDEX = 3;
const byte MENU_FREQUENCY_SETTINGS = 4;
const byte MENU_ATTENUATION_SETTINGS = 5;

// EEPROM Addresses
const short INIT_SCHEMA = 0x0101;         // Schema to check if eeprom has been initialized fot the first time.
const byte INIT_EEPROM_ADDRESS = 0;       // Address to write schema to
const byte FREQ_EEPROM_ADDRESS = 2;       // Selected frequency address
const byte ATTENUATION_PROFILE_EEPROM_ADDRESS = 3;  // Selected attenuation profile address
const byte ATTENUATION_VALUE_EEPROM_ADDRESS = 4;  // Selected attenuation address (float = 4 byte)
//Next address will be 8


// EEPROM Defaults
const byte FREQ_DEFAULT_INDEX = 4;          // Default selected frequency 5.8GHz. Is index of FREQUENCIES array.
const float ATTENUATION_DEFAULT = 30.0;     // Default attenuation 30dBm
const byte ATTENUATION_PROFILE_DEFAULT = 0; // Default profile (0=custom, 1=VAT-20W2+, 2=VAT-30W2+)

const byte MAX_ATTENUATION_DBM = 50;        // Maximum possible attenuation in dBm
const byte MAX_ATTENUATION_PROFILE = 2;     // Current max profile setting

// A frequency can be selected by the user. 
// The selected frequency determines the typical performance characteristics taken from the datasheet.
// All values below are taken from page 3 and 4 of the datasheet.
const int FREQUENCIES[6]    = { 900, 1800, 2200, 3600, 5800, 8000 };   // List of frequencies we know settings data of.
const int MIN_INPUT_DBM[6]  = { -53, -54, -54, -48, -54, -46 };      // Minimum input power supported according to the datasheet for frequencies in list
const int MAX_INPUT_DBM[6]  = { -3, -4, -5, -6, -4, -2 };            // Maximum input power supported according to the datasheet for frequencies in list
const float DBM_AT_0V[6]    = { 16.36, 14.55, 14.09, 11.36, 16.82, 21.82 };  // dBm value at 0V according to the datasheet for frequencies in list.
const float VAT20W2_ATT_PROFILE[6] = { 20.0, 19.7, 19.6, 18.9, 18.1, 18.0 };  // Attenuator profile of Minicircuits VAT-20W2+ attenuator. A value for each frequency of the list.
const float VAT30W2_ATT_PROFILE[6] = { 29.8, 29.3, 29.2, 28.6, 27.7, 27.5 };  // Attenuator profile of Minicircuits VAT-30W2+ attenuator. A value for each frequency of the list.

const float MV_DB_SLOPE     = 0.022;       // Slope of AD8317 characteristics in -mV/dBm. Is the same for all frequencies. 
// NOTE: The DBM_AT_0V[] values are based on the -10dBm value from the datasheet. 
// i.e. when f=5.8GHz the datasheet notes 700mV at -10dBm. With a slope of -22mV/dBm ==> (700/22m) - 10 = 21.82dBm at 0V.
// Unfortunatly these figures are can vary quite a lot up to 1.9GHz. In that case the typical value is taken.
// I did not have a calibrated power source. But people who have can maybe add a calibration function to the software and store the values in EEPROM.

// Variable declarations
LiquidCrystal lcd(LCD_RS_PIN, LCD_RW_PIN, LCD_E_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN); // LCD driver
Bounce button_UP = Bounce();      // Button driver for up button
Bounce button_ENTER = Bounce();   // Button driver for enter button
Bounce button_DOWN = Bounce();    // Button driver for down button

// Measured and calculated values
int samples_dbm[SAMPLE_COUNT];  // Array to store measured values
float avg_dbm;                  // Current average power in dBm
float pep_dbm;                  // Current peak envelope power in dBm
const byte max_history_count = 20;    // Maximum amount of history items
float min_dbm_history[max_history_count];   // Minimum power history ring buffer
float max_dbm_history[max_history_count];   // Maximum power history ring buffer
float min_dbm = 1000;           // Minimum power based on history
float max_dbm = -1000;          // Maximum power based on history
int modulation_index;           // Modulation index based on history
byte att_profile = 0;           // Current selected attenuation profile (0=custom, 1=VAT-20W2+, 2=VAT-30W2+).
float custom_attenuation = 0;   // Custom attenuation in dBm. Used when no attenuator profile is selected.
byte selected_freq = 4;         // Selected index from array of frequencies.
bool power_overload;            // If true, too much power is put into the device.
bool power_notconnected;        // If true no or too less power is put into the device.

// Menu and display
byte display_line, display_col; // Cursor position on lcd
byte current_menu;              // Current selected menu displayed


/* 
* Setup routine
*/
void setup() 
{   
  analogReference(EXTERNAL);    // Set to external reference
  initButtons();
  getSettingsFromEEPROM();
  displaySplashScreen();
  resetHistoryRingBuffers();
}


/*
 * The main loop 
*/
void loop() 
{  
  takeSamples();        // Take all samples (and read buttons during sampling)
  calculate();          // Calculate values
  readButtons();        // Read buttons
    
  // Show measurements or warning
  if (checkInputBounds())   // Check if the measured values are within the device's range.
  {
    if (current_menu == MENU_MEASUREMENTS)    
      displayMeasurements();       
    else if (current_menu == MENU_MIN_POWER)
      displayMinPower();
    else if (current_menu == MENU_MAX_POWER)
      displayMaxPower();
    else if (current_menu == MENU_MODULATIONINDEX)
      displayPEPModulationIndex();   
  }
  else  
    displayOverloadWarning();     // Display overload warning.
  
  readButtons();
}

/*
 * Take SAMPLE_COUNT of samples
 * sampling 500 values with no delay takes 76ms
 * with a delay of 750us it takes 495ms
*/
void takeSamples() 
{
  for (int i=0; i<SAMPLE_COUNT; i++)
  {     
    samples_dbm[i] = analogRead(AD_POWER_PIN); // Take a sample - 116us            
    readButtons();
    delayMicroseconds(SAMPLE_INTERVAL_US);        
  }    
}


/*
 * Perform all calculations we need.
*/
void calculate()
{    
  static byte history_idx = 0;           // Current index of history ringbuffers
  // Calculate average, min and max values  
  int min_value = 1000, max_value = -1000;
  unsigned long sum = 0;

  // Determine the min, max and sum
  for (int i=0; i < SAMPLE_COUNT; i++)
  {    
    if (min_value > samples_dbm[i]) min_value = samples_dbm[i];
    if (max_value < samples_dbm[i]) max_value = samples_dbm[i];    
    sum += samples_dbm[i];
  }
    
  avg_dbm = measuredValueTodBm(sum / SAMPLE_COUNT);          // Calculate average of current samples
  pep_dbm = measuredValueTodBm(max_value);                   // Peak envelope power is the max measured power
  
  // Store current min/max value in history
  min_dbm_history[history_idx] = measuredValueTodBm(min_value);
  max_dbm_history[history_idx++] = measuredValueTodBm(max_value);
  if (history_idx >= max_history_count)
    history_idx = 0;                                          //Reset ringbuffer index

  // Get absolute min/max value from history
  min_dbm = 1000, max_dbm = -1000;
  for (int h=0; h < max_history_count; h++)
  {
    if (min_dbm_history[h] < min_dbm)
      min_dbm = min_dbm_history[h];                           // Current minimum value
    if (max_dbm_history[h] > max_dbm)
      max_dbm = max_dbm_history[h];                           // Current maximum value
  }
    
  //Calculate modulation index based voltage of min and max dbm values
  float min_voltage, max_voltage;
  min_voltage = dBmToVolt(min_dbm);
  max_voltage = dBmToVolt(max_dbm);
  modulation_index = (max_voltage - min_voltage) / (max_voltage + min_voltage);
}

/*
 * Convert the AD converter value to voltage and then dbm
*/
float measuredValueTodBm(int value) 
{
  float voltage = value * (V_REF / AD_RESOLUTION);    
  float dbm = convertVoltageToDbm(voltage);
  return dbm;
}

/*
 * Reset the history
*/
void resetHistoryRingBuffers() 
{
  for (int h=0; h < max_history_count; h++)
  {
    min_dbm_history[h] = 1000;      
    max_dbm_history[h] = -1000;      
  }  
}

/*
 * Check if the power measured is within the range.
 * Overpower can damage the device.
*/
bool checkInputBounds() 
{
  power_overload = (avg_dbm >= MAX_INPUT_DBM[selected_freq]);
  power_notconnected = (avg_dbm < MIN_INPUT_DBM[selected_freq]);  
  return !power_overload;
}


/*
 * Read the buttons and display the correct menu if button is clicked.
*/
void readButtons() 
{
  // Get button states
  bool up_pressed = (button_UP.update() && button_UP.rose());
  bool enter_pressed = (button_ENTER.update() && button_ENTER.rose());
  bool down_pressed = (button_DOWN.update() && button_DOWN.rose());
      
  if (up_pressed || enter_pressed || down_pressed)
  {
    switch(current_menu)
    {
      case MENU_MEASUREMENTS:
        if (up_pressed) 
          current_menu = MENU_MIN_POWER;
        else if (down_pressed) 
          current_menu = MENU_MODULATIONINDEX;
        else if (enter_pressed) 
        {
          current_menu = MENU_FREQUENCY_SETTINGS;
          display_line = 0; // Set cursor on line 0
          display_col = 10; // Set cursor on row 10
          displayFrequencySettingsMenu();
        }
        break;
        
      case MENU_MIN_POWER:
        if (up_pressed) 
          current_menu = MENU_MAX_POWER;
        else if (down_pressed) 
          current_menu = MENU_MEASUREMENTS;
        else if (enter_pressed) 
          resetHistoryRingBuffers();        
        break;

      case MENU_MAX_POWER:
        if (up_pressed) 
          current_menu = MENU_MODULATIONINDEX;
        else if (down_pressed) 
          current_menu = MENU_MIN_POWER;
        else if (enter_pressed) 
          resetHistoryRingBuffers();        
        break;

     case MENU_MODULATIONINDEX:
        if (up_pressed) 
          current_menu = MENU_MEASUREMENTS;
        else if (down_pressed) 
          current_menu = MENU_MAX_POWER;
        else if (enter_pressed) 
          resetHistoryRingBuffers();        
        break;
        
      case MENU_FREQUENCY_SETTINGS:
        if (up_pressed) 
        {          
          // If on frequency line            
          setSelectedFrequency(true); // Set to next frequency in the list           
          displayFrequencySettingsMenu();        
        }
        else if (down_pressed) 
        {          
          // If on frequency line            
          setSelectedFrequency(false); // Set to previous frequency in the list         
          displayFrequencySettingsMenu();     
        }
        else if (enter_pressed) 
        {          
          current_menu = MENU_ATTENUATION_SETTINGS;    
          display_line = 1; // Set cursor on line 1
          display_col = 0; // Set cursor on row 0      
          displayAttenuationSettingsMenu();
        }
        break;

      case MENU_ATTENUATION_SETTINGS:
        if (up_pressed) 
        {
          if (display_col == 0) 
          {
            // If on profile row          
            setAttenuationProfile(true); // Set to next frequency in the list
          }
          else 
          { 
            // On attenuation line                        
            if (display_col == 7)
              custom_attenuation+=10;  // Increment attenuation with 10
            else if (display_col == 8)
              custom_attenuation++;    // Increment attenuation with 1
            else if (display_col == 10)
              custom_attenuation+=0.1; // Increment attanuation with 0.1
              
            if (custom_attenuation > MAX_ATTENUATION_DBM) 
              custom_attenuation = MAX_ATTENUATION_DBM; // Cannot be higher than MAX_ATTENUATION_DBM;
          }  
          displayAttenuationSettingsMenu();        
        }
        else if (down_pressed) 
        {
          if (display_col == 0) 
          {
            // If on profile row          
            setAttenuationProfile(false); // Set to next frequency in the list
          }
          else 
          { 
            // On attenuation line                        
            if (display_col == 7)
              custom_attenuation-=10;  // Decrement attenuation with 10
            else if (display_col == 8)
              custom_attenuation--;   // Decrement attenuation with 1
            else if (display_col == 10)
              custom_attenuation-=0.1;  // Decrement attanuation with 0.1
              
            if (custom_attenuation < 0) 
              custom_attenuation = 0; // Cannot be lower than 0;
          }  
          displayAttenuationSettingsMenu();      
        }
        else if (enter_pressed) 
        {
          if (att_profile == 0) // Custom profile
          {
            if (display_col == 0) // If on profile selection column
            {
              display_col = 7;    // Goto x10 row         
            }
            else if (display_col == 7) // If on attenuation x10 column
            {            
              display_col = 8;    // Goto x1 row  
            }
            else if (display_col == 8) // If on attenuation x1 column
            {            
              display_col = 10;   // Goto x0.1 row  
            }
            else if (display_col == 10) // If on attenuation x0.1 column
            {
              writeSettingsToEEPROM();  // Write settings to EEPROM so we can use the same settings next time.
              // Turn off cursor            
              lcd.noCursor();
              // Go back to measure menu;
              current_menu = MENU_MEASUREMENTS;
            }
          }
          else // If on other profile then 'custom'
          {
            writeSettingsToEEPROM();  // Write settings to EEPROM so we can use the same settings next time.
            // Turn off cursor            
            lcd.noCursor();
            // Go back to measure menu;
            current_menu = MENU_MEASUREMENTS;
          }
          displayAttenuationSettingsMenu();
        }
        break;
    }                
  }  
}

/*
 * Display warning message
*/
void displayOverloadWarning() 
{
  lcd.home();  
  lcd.print("OVERLOAD WARNING");         
  lcd.setCursor(0, 1); // goto next line
  lcd.print("DISCONNECT NOW! ");        
}


/*
* Display the settings and primary measurements
* Frequency, attenuation and power in dBm and mW
*/
void displayMeasurements () 
{
  static byte cnt = 0;
  byte chars = 0;
  lcd.home();    

  // Display frequency and attenuation in first line alternatly
  if (cnt <= 5)
  {    
    chars += lcd.print("Frequency:");
    if (FREQUENCIES[selected_freq] >= 1000)
    {
      chars += printFormattedNumber(FREQUENCIES[selected_freq] / 1000.0, 1, 1, false, true);    
      chars += lcd.print("GHz");
    }      
    else
    {
      chars += lcd.print(FREQUENCIES[selected_freq]);    
      chars += lcd.print("MHz");
    }
  }
  else 
  {
    chars += lcd.print("ATTN: ");   
    chars += printFormattedNumber(currentAttenuation(), 1, 1, false, false);    
    chars += lcd.print("dBm");         
  }
  filloutLine(chars);
  if (cnt++ > 10)
    cnt = 0;

  // Line 2
  lcd.setCursor(0, 1); // goto next line     
  if (power_notconnected)
  {    
    lcd.print("INPUT TOO LOW/NC");   
  }
  else
  {        
    float actualPowerdBm = avg_dbm + currentAttenuation();
    float actualPowerMw = convertDbmToMilliWatt(actualPowerdBm);
    
    chars = printFormattedNumber(actualPowerdBm, 1, 1, true, true); 
    chars += lcd.print("dBm ");  
    chars += printPowerWatts(actualPowerMw);
    filloutLine(chars);
  }
}

/*
* Display the max power measurement
*/
void displayMinPower () 
{
    lcd.home();    
    if (power_notconnected) 
    {       
      lcd.print("Minumum power:  ");
      lcd.setCursor(0, 1); // goto next line     
      lcd.print("INPUT TOO LOW/NC");
    }
    else 
    {      
      byte chars = lcd.print("Minimum power:");    
      filloutLine(chars);

      lcd.setCursor(0, 1); // goto next line     
      chars = printFormattedNumber(min_dbm + currentAttenuation(), 1, 1, true, true);    
      chars += lcd.print("dBm ");    
      chars += printPowerWatts(convertDbmToMilliWatt(min_dbm + currentAttenuation()));
      filloutLine(chars);          
    }
}

/*
 * Display the maximum power
*/
void displayMaxPower () 
{
    lcd.home();    
    if (power_notconnected) 
    {       
      lcd.print("Maximum power:  ");
      lcd.setCursor(0, 1); // goto next line     
      lcd.print("INPUT TOO LOW/NC");
    }
    else 
    {
      lcd.print("Maximum power:  ");           
      lcd.setCursor(0, 1); // goto next line     
      byte chars = printFormattedNumber(max_dbm + currentAttenuation(), 1, 1, true, true);    
      chars += lcd.print("dBm ");    
      chars += printPowerWatts(convertDbmToMilliWatt(max_dbm + currentAttenuation()));
      filloutLine(chars);          
    }
}

/*
* Display the min/max measurements
*/
void displayPEPModulationIndex () 
{
    lcd.home();    
    if (power_notconnected) 
    {       
      lcd.print("PEP:            ");
      lcd.setCursor(0, 1); // goto next line     
      lcd.print("INPUT TOO LOW/NC");
    }
    else 
    {      
      byte chars = lcd.print("PEP:");    
      chars += printFormattedNumber(pep_dbm + currentAttenuation(), 1, 1, true, true);    
      chars += lcd.print("dBm");
      filloutLine(chars);    
      
      lcd.setCursor(0, 1); // goto next line     
      chars = lcd.print("M: ");    
      chars += lcd.print(modulation_index);
      chars += lcd.print("%");    
      filloutLine(chars);
    }
}


/*
 * Displays the settings menu
 * Frequency can be changed.
*/
void displayFrequencySettingsMenu() 
{
  lcd.home();
  
  byte chars = lcd.print("frequency:");    
  if (FREQUENCIES[selected_freq] >= 1000)
  {
    chars += printFormattedNumber(FREQUENCIES[selected_freq] / 1000.0, 1, 1, false, true);    
    chars += lcd.print("GHz");
  }      
  else
  {
    chars += lcd.print(FREQUENCIES[selected_freq]);    
    chars += lcd.print("MHz");
  }  
  filloutLine(chars);

  lcd.setCursor(0, 1);
  filloutLine(0); // Empty line
    
  lcd.setCursor(display_col, display_line); // Set cursur on desired position.
  lcd.cursor();   // Turn on cursor
  lcd.blink();    // Turn on blinking of the cursor
}


/*
 * Displays the attenuation settings menu
 * Attenuation profile can be changed.
*/
void displayAttenuationSettingsMenu() 
{
  lcd.home();
  
  byte chars = lcd.print("atten. profile  ");     
  lcd.setCursor(0, 1);
  if (att_profile == 0) // Custom profile
  {        
    chars = lcd.print("Custom:"); 
    chars+= printFormattedNumber(custom_attenuation, 2, 1, false, false);    
    chars+= lcd.print("dBm"); 
    filloutLine(chars);
  }
  else if (att_profile == 1)  
    lcd.print("VAT-20W2+       ");   
  else if (att_profile == 2)  
    lcd.print("VAT-30W2+       "); 
    
  lcd.setCursor(display_col, display_line); // Set cursur on desired position.
  lcd.cursor();   // Turn on cursor
  lcd.blink();    // Turn on blinking of the cursor
}

/*
 * Display the splash screen
*/
void displaySplashScreen() 
{
  lcd.begin(16,2);
  lcd.print(" RF Power Meter");
  lcd.setCursor(0, 1);
  lcd.print(" 1MHZ - 10GHz  ");
  delay(2000);
  lcd.home();
  lcd.print("Maximum input   ");
  lcd.setCursor(0, 1);
  lcd.print("power 0dBm / 1mW");  
  delay(2000);
  lcd.clear();
}

/*
* Print power in uW, mW or W.
*/
byte printPowerWatts(float mw) 
{
  byte chars = 0;
  if (mw < 1.0) // If less then 1mW diaplay in uW
  {
    chars = printFormattedNumber(mw * 1000.0, 1, 0, false, false);
    chars += lcd.print("uW");        
  }
  else if (mw >= 1000.0) // If more than 1000mW display in W
  {
    chars = printFormattedNumber(mw / 1000.0, 1, 1, false, false);
    chars += lcd.print("W");        
  }
  else
  {
    chars = printFormattedNumber(mw, 1, 1, false, false);
    chars += lcd.print("mW");        
  }
  return chars;
}


/*
 * Selected next or previous frequency from the list
*/
void setSelectedFrequency(bool up) 
{
  if (up && selected_freq < 5)  
    selected_freq++;  
  else if (!up && selected_freq > 0)
    selected_freq--;      
}

/*
 * Select the next or previous attenuation profile
*/
void setAttenuationProfile(bool next)
{
  if (next && att_profile < MAX_ATTENUATION_PROFILE)
    att_profile++;  
  else if (next && att_profile >= MAX_ATTENUATION_PROFILE)
    att_profile = 0;
  else if (!next && att_profile > 0)
    att_profile--;
  else if (!next && att_profile <= 0)
    att_profile = MAX_ATTENUATION_PROFILE;
}

/*
 * Fillout a complete display line with spaces.
*/
void filloutLine(byte charsWritten) 
{
  for (byte i=0; i < 16 - charsWritten;i++)
      lcd.print(" ");    
}

/*
 * Print a number with a specific amount of digits and decimals on the display.
 * It is also possible to print a space when there is no negative sign.
 * number: Number to display
 * minAmountDigits: The minimum amount of digits to display. If number is shorter then '0's will be printed before the number.
 * amountDecimals: The amount of decimals to display.
 * fillout: If the amount of decimals is shorter then the desired amount, spaces will be used to get the same string length.
*/
byte printFormattedNumber(float number, byte minAmountDigits, byte amountDecimals, bool reserveMinus, bool fillout)
{
  byte chars = 0; // Number of chars written.
  
  // If no minus sign, write a space
  if (reserveMinus && number >= 0)
    chars += lcd.print(" ");
  else if (number < 0)
    chars += lcd.print("-");
    
  //Print integer part. When current value has less digits then wanted add extra zero's
  int digitPart = (int)number;
  byte digitCount = getAmountDigits(digitPart);
  if (digitCount < minAmountDigits)
  {
    for (byte i=0; i < minAmountDigits - digitCount; i++)
      chars += lcd.print("0");
  }
  // Display digit part
  chars += lcd.print(abs((int)number));
  // Display decimal part
  if (amountDecimals > 0) 
  {
     chars += lcd.print(".");
     chars += lcd.print(getDecimalPart(number, amountDecimals));
  }  
  
  if (fillout) 
  {
    // Fill missing chars according to desired length
    byte totalCharsWanted = minAmountDigits + amountDecimals;
    if (reserveMinus)
      totalCharsWanted++;    
    for (byte i=0; i < totalCharsWanted - chars; i++)
      chars += lcd.print(" ");   
  }  
  return chars; // return the amount of chars written to the display
}


/*
 * Get the decimal part of a number.
 * 123.56 returns 56
*/
unsigned int getDecimalPart(float number, int amountDecimals) 
{      
  if (number < 0)
    number = -number;
        
  float fract = number - floor(number);     
  return (unsigned int)(fract * pow(10, amountDecimals));
 
}

/*
 * Get amount of digits of an integer.
 * Simple but fast
*/
byte getAmountDigits(int n)
{
    n = abs(n);
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;    
}

/*
 * Get the current selected attenuation
*/
float currentAttenuation()
{
  float att_value;
  switch (att_profile)
  {
    case 0: // Custom value
      att_value = custom_attenuation;
      break;
    case 1: // VAT-20W2+, select the attenuation of the current selected frequency
      att_value = VAT20W2_ATT_PROFILE[selected_freq];
      break;
    case 2: // VAT-30W2+, select the attenuation of the current selected frequency
      att_value = VAT30W2_ATT_PROFILE[selected_freq];
      break;
  }
  return att_value;
}

/*
 * Initialize the buttons
*/
void initButtons() 
{
  // Set button pins as input.  
  pinMode(BTN_UP ,INPUT);
  pinMode(BTN_ENTER ,INPUT);
  pinMode(BTN_DOWN ,INPUT);
  // Attach pins to Bounce
  button_UP.attach(BTN_UP);
  button_ENTER.attach(BTN_ENTER);
  button_DOWN.attach(BTN_DOWN);
  // Set bounce interval
  button_UP.interval(BUTTON_DEBOUNCE_MS);
  button_ENTER.interval(BUTTON_DEBOUNCE_MS);
  button_DOWN.interval(BUTTON_DEBOUNCE_MS);
}



/*
 * Read settings from EEPROM
*/
void getSettingsFromEEPROM() 
{ 
  // Check if EEPROM has been initialized.
  short schema = 0;
  EEPROM_readAnything(INIT_EEPROM_ADDRESS, schema);  
  if (schema != INIT_SCHEMA)   // Compare with expected schema   
    initEEPROM();   // Initialize EEPROM with default value

  // Read values from EEPROM
  selected_freq = EEPROM.read(FREQ_EEPROM_ADDRESS);
  att_profile = EEPROM.read(ATTENUATION_PROFILE_EEPROM_ADDRESS);
  EEPROM_readAnything(ATTENUATION_VALUE_EEPROM_ADDRESS, custom_attenuation);  
}


/*
 * Initialize the EEPROM with defaults
*/
void initEEPROM()
{
  EEPROM.write(FREQ_EEPROM_ADDRESS, selected_freq);
  EEPROM.write(ATTENUATION_PROFILE_EEPROM_ADDRESS, att_profile);
  EEPROM_writeAnything(ATTENUATION_VALUE_EEPROM_ADDRESS, custom_attenuation);
  EEPROM_writeAnything(INIT_EEPROM_ADDRESS, INIT_SCHEMA); // Mark EEPROM as initialized
}


/*
 * Write settings to EEPROM
*/
void writeSettingsToEEPROM() 
{
  EEPROM.write(FREQ_EEPROM_ADDRESS, selected_freq);       //Write selected frequency to EEPROM  
  EEPROM.write(ATTENUATION_PROFILE_EEPROM_ADDRESS, att_profile);  //Write selected attenuation profile to EEPROM
  EEPROM_writeAnything(ATTENUATION_VALUE_EEPROM_ADDRESS, custom_attenuation);  //Write selected attenuation to EEPROM  
}


/*
 * Convert voltage to dBm.
 * According to Figure 8 of the datasheet
 * dBm value at 0V - (measured voltage / slope)
 * The supported range is -50dBm to 0dBm because of high error beyond. 
*/
float convertVoltageToDbm(float voltage) 
{
  return DBM_AT_0V[selected_freq] - (voltage / MV_DB_SLOPE);
}

/* Convert dBm to mW */
float convertDbmToMilliWatt(float dBm) 
{
  return pow(10, dBm / 10.0);
}

/* Convert mW to dBm */
float convertMilliWattToDbm(float mw) 
{
  return 10 * log10(mw);
}

/* Convert dBm to voltage */
float dBmToVolt(float dBm) 
{
  return pow(10, dBm/20.0);
}


template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}
