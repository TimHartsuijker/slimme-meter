#include <Arduino.h>
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#define BUFSIZE 75

char input; // inkomende seriele data (byte)
bool readnextLine = false;
char buffer[BUFSIZE]; //Buffer voor seriele data om \n te vinden.
int bufpos = 0;
long mEVLT = 0; //Meterstand Elektra - verbruik laag tarief
long mEVHT = 0; //Meterstand Elektra - verbruik hoog tarief
long mETLT = 0; //Meterstand Elektra - teruglevering laag tarief
long mETHT = 0; //Meterstand Elektra - teruglevering hoog tarief
long mEAV = 0;  //Meterstand Elektra - actueel verbruik
long mEAT = 0;  //Meterstand Elektra - actueel teruglevering
float mG = 0;   //Meterstand Gas

long DatamEVLT;
long DatamEVHT;
long DatamETLT;
long DatamETHT;
long DatamEAV;
long DatamEAT;
float DatamG;

//database information
IPAddress server_addr(128,199,52,194);  
char user[] = "tim.hartsuijker";              
char password[] = "Tim+Hartsuijker";  
char db[] = "tim.hartsuijker";

//query information
char INSERT_DATA[] = "INSERT INTO test (mEVLT, mEVHT, mETLT, mETHT, mEAV, mEAT, mG) VALUES (%d, %d, %d, %d, %d, %d, %d)";
char query[128];

// wifi information
const char* ssid = "OmniEnergy"; 
const char* pass = "Kilowattuur";

WiFiClient client;          
MySQL_Connection conn((Client *)&client);
MySQL_Cursor cur = MySQL_Cursor(&conn);

// Connect to Wi-Fi network with SSID and password
void WiFiConnect()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) 
    delay(500);
  Serial.println("WiFi connected.");
}


// executes a query to insert values into the database. 
// after this, it disconnects from the database.
void ExecuteInsertQuery()
{
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  sprintf(query, INSERT_DATA, DatamEVLT, DatamEVHT, DatamETLT, DatamETHT, DatamEAV, DatamEAT, DatamG);
  cur_mem->execute(query);
  delete cur_mem;
}

void setup()
{
  Serial.begin(115200);
  WiFiConnect();
}

void loop()
{

  long tl = 0;
  long tld =0;

  if (Serial.available() > 0) 
  {
    input = Serial.read();

    if(!conn.connected())
    {
      Serial.println("database not conneted");
      conn.connect(server_addr, 3306, user, password, db);
        delay(500);
    }
    
    
    // --- 7 bits instelling ---
    input &= ~(1 << 7);
    char inChar = (char)input;
    // --- 7 bits instelling ---
 
    // Serial.print(input);
 
    // Vul buffer tot en met een nieuwe lijn (\n)
   buffer[bufpos] = input&127;
   bufpos++;
 
    if (input == '\n') 
    {
 
      if (sscanf(buffer,"1-0:1.8.1(%ld.%ld%*s" , &tl, &tld)) 
      {
        mEVLT = tl * 1000 + tld;
        if (mEVLT > 0) 
        {
          DatamEVLT = mEVLT;
          Serial.print("Elektra - meterstand verbruik LAAG tarief (Wh): ");
          Serial.println(mEVLT);
          mEVLT = 0;
        }
      }
 
      if (sscanf(buffer,"1-0:1.8.2(%ld.%ld%*s" , &tl, &tld)) 
      {
        mEVHT = tl * 1000 + tld;
        if (mEVHT > 0) 
        {
          DatamEVHT = mEVHT;
          Serial.print("Elektra - meterstand verbruik HOOG tarief (Wh): ");
          Serial.println(mEVHT);
          mEVHT = 0;
        }
      }
 
      if (sscanf(buffer,"1-0:1.7.0(%ld.%ld%*s" , &tl , &tld)) 
      {
        mEAV = tl * 1000 + tld * 10;
        if (mEAV > 0) 
        {
          DatamEAV = mEAV;
          Serial.print("Elektra - actueel verbruik (W): ");
          Serial.println(mEAV);
          mEAV = 0;
        }
      }
 
      if (sscanf(buffer,"1-0:2.8.1(%ld.%ld%*s" , &tl, &tld)) 
      {

        mETLT = tl * 1000 + tld;
        if (mETLT > 0) 
        {
          DatamETLT =mETLT;
          Serial.print("Elektra - meterstand teruglevering LAAG tarief (Wh): ");
          Serial.println(mETLT);
          mETLT = 0;
        }
      }
 
      if (sscanf(buffer,"1-0:2.8.2(%ld.%ld%*s" , &tl, &tld)) 
      {
        mETHT = tl * 1000 + tld;
        if (mETHT > 0) 
        {
          DatamETHT = mETHT;
          Serial.print("Elektra - meterstand teruglevering HOOG tarief (Wh): ");
          Serial.println(mETHT);
          mETHT = 0;
        }
      }

      if(sscanf(buffer,"1-0:2.7.0(%ld.%ld%*s" , &tl , &tld)) 
      {
        mEAT = tl * 1000 + tld * 10;
        if (mEAT > 0) 
        {
          DatamEAT = mEAT;
          Serial.print("Elektra - actueel teruglevering (W): ");
          Serial.println(mEAT);
          mEAT = 0;
        }
      }
 
      if(sscanf(buffer,"0-1:24.2.1(%ld.%c" , &tl, &tld)) 
      {
        mG = float (tl * 1000 + tld) / 1000;
        DatamG = mG;
        Serial.print("Gas - meterstand (m3): ");
        Serial.println(mG);
        mG = 0;
      }

      if(sscanf(buffer,"!%s")) 
      {
        ExecuteInsertQuery();
        conn.close();      
      }

      // Maak de buffer weer leeg (hele array)
      for (int i=0; i<75; i++)
        buffer[i] = 0;
      bufpos = 0;
    }
  }
}