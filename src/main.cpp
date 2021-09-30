#include <Arduino.h>
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>


HardwareSerial bt(2);

#define RX 3

char input; // inkomende seriele data (byte)
bool readnextLine = false;
#define BUFSIZE 75
char buffer[BUFSIZE]; //Buffer voor seriele data om \n te vinden.
int bufpos = 0;
long mEVLT = 0; //Meterstand Elektra - verbruik laag tarief
long mEVHT = 0; //Meterstand Elektra - verbruik hoog tarief
long mETLT = 0; //Meterstand Elektra - teruglevering laag tarief
long mETHT = 0; //Meterstand Elektra - teruglevering hoog tarief
long mEAV = 0;  //Meterstand Elektra - actueel verbruik
long mEAT = 0;  //Meterstand Elektra - actueel teruglevering
float mG = 0;   //Meterstand Gas
int usage;  
String kolom;

//database information
IPAddress server_addr(128,199,52,194);  
char user[] = "tim.hartsuijker";              
char password[] = "Tim+Hartsuijker";  
char db[] = "tim.hartsuijker";

// wifi information
const char* ssid = "OmniEnergy"; 
const char* pass = "Kilowattuur";

//query information
char INSERT_DATA[] = "INSERT INTO test (%s) VALUES (%d)";
char REQUEST_DATA[] = "SELECT `value` FROM `test`";
char query[128];

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

// this function creates a database connection, 
// executes a query to insert values into the database. 
// after this, it disconnects from the database.
void ExecuteInsertQuery()
{
  Serial.println("Connecting to database");

  if(conn.connect(server_addr, 3306, user, password, db)) 
  {
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    sprintf(query, INSERT_DATA, kolom, usage);
    cur_mem->execute(query);
    delete cur_mem;
    conn.close();
  }
  else
    Serial.println("Connection failed.");
}

// this function creates a database connection, 
// executes a query to select all information from the database. 
// after this, it disconnects from the database.
void ExecuteSelectQuery()
{
  if(conn.connect(server_addr, 3306, user, password, db))
  {
    row_values *row = NULL;
    long value;
    int val = 0;
    delay(1000);
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    sprintf(query, REQUEST_DATA);
    cur_mem->execute(query);
    column_names *columns = cur_mem->get_columns();
   do 
    {
      row = cur_mem->get_next_row();
      if (row != NULL) 
      {
        value = atol(row->values[0]);
        Serial.println(atol(row->values[0]));
        val++;
      }
    }while (row != NULL);
    
    delete cur_mem;
    delay(500);
    conn.close();
  }
  else
  Serial.println("Connection failed.");
}

void setup() {
  Serial.begin(115200);
  WiFiConnect();
}

void loop() {
long tl = 0;
long tld =0;

  if (Serial.available() > 0) {
    input = Serial.read(); 
    // --- 7 bits instelling ---
    input &= ~(1 << 7);
    char inChar = (char)input;
    // --- 7 bits instelling ---
 
    Serial.print(input); //Debug
 
    // Vul buffer tot en met een nieuwe lijn (\n)
   buffer[bufpos] = input&127;
   bufpos++;
 
    if (input == '\n') { // we hebben een lijn binnen (gegevens tot \n)
 
      // 1-0:1.8.1 = Elektra verbruik laag tarief (DSMR v4.0)
      if (sscanf(buffer,"1-0:1.8.1(%ld%.%ld%*s" , &tl, &tld) >0 ) {
        mEVLT = tl * 1000 + tld;
        if (mEVLT > 0) {
          kolom = "mEVLT";
          usage = mEVLT;
          ExecuteInsertQuery();
          Serial.print("Elektra - meterstand verbruik LAAG tarief (Wh): ");
          Serial.println(mEVLT);
          mEVLT = 0;
        }
      }
 
      // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
      if (sscanf(buffer,"1-0:1.8.2(%ld%.%ld%*s" , &tl, &tld) >0 ) {
        mEVHT = tl * 1000 + tld;
        if (mEVHT > 0) {
          kolom = "mEVHT";
          usage = mEVHT;
          ExecuteInsertQuery();
          Serial.print("Elektra - meterstand verbruik HOOG tarief (Wh): ");
          Serial.println(mEVHT);
          mEVHT = 0;
        }
      }
 
      // 1-0:1.7.0 = Elektra actueel verbruik (DSMR v4.0)
      if (sscanf(buffer,"1-0:1.7.0(%ld.%ld%*s" , &tl , &tld) >0 ) {
        mEAV = tl * 1000 + tld * 10;
        if (mEAV > 0) {
          kolom = "mEAV";
          usage = mEAV;
          ExecuteInsertQuery();
          Serial.print("Elektra - actueel verbruik (W): ");
          Serial.println(mEAV);
          mEAV = 0;
        }
      }
 
      // 1-0:2.8.1 = Elektra teruglevering hoog tarief (DSMR v4.0)
      if (sscanf(buffer,"1-0:2.8.1(%ld%.%ld%*s" , &tl, &tld) >0 ) {
        mETLT = tl * 1000 + tld;
        if (mETLT > 0) {
          kolom = "mETLT";
          usage = mETLT;
          ExecuteInsertQuery();
          Serial.print("Elektra - meterstand teruglevering LAAG tarief (Wh): ");
          Serial.println(mETLT);
          mETLT = 0;
        }
      }
 
      // 1-0:2.8.2 = Elektra teruglevering hoog tarief (DSMR v4.0)
      if (sscanf(buffer,"1-0:2.8.2(%ld%.%ld%*s" , &tl, &tld) >0 ) {
        mETHT = tl * 1000 + tld;
        if (mETHT > 0) {
          kolom = "mETHT";
          usage = mETHT;
          ExecuteInsertQuery();
          Serial.print("Elektra - meterstand teruglevering HOOG tarief (Wh): ");
          Serial.println(mETHT);
          mETHT = 0;
        }
      }
 
      // 1-0:2.7.0 = Elektra actueel teruglevering (DSMR v4.0)
      if (sscanf(buffer,"1-0:2.7.0(%ld.%ld%*s" , &tl , &tld) >0  ) {
        mEAT = tl * 1000 + tld * 10;
        if (mEAT > 0) {
          kolom = "mEAT";
          usage = mEAT;
          ExecuteInsertQuery();
          Serial.print("Elektra - actueel teruglevering (W): ");
          Serial.println(mEAT);
          mEAT = 0;
        }
      }
 
      // 0-1:24.3.0 = Gas (DSMR v4.0)
      if (sscanf(buffer,"0-1:24.2.0(%6ld%4ld%*s" , &tl, &tld) > 0  ) {
        readnextLine = true; // we moeten de volgende lijn hebben
      }
      if (readnextLine){
        if (sscanf(buffer,"(%ld.%ld%*s" , &tl, &tld) >0  ) {
          mG = float ( tl * 1000 + tld ) / 1000;
          kolom = "mG";
          usage = mG;
          ExecuteInsertQuery();
          Serial.print("Gas - meterstand (m3): ");
          Serial.println(mG);
          Serial.println("");
          readnextLine = false;
        }
      }
 
      // Maak de buffer weer leeg (hele array)
      for (int i=0; i<75; i++)
      { buffer[i] = 0;}
      bufpos = 0;
    }
  }
}