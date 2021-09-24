#include <Arduino.h>
#include <WiFi.h>                  // Use this for WiFi instead of Ethernet.h
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

byte mac_addr[] = { 0xE0, 0xE2, 0xE6, 0xF4, 0x21, 0x90 };

//database information
IPAddress server_addr(128,199,52,194);  
char user[] = "tim.hartsuijker";              
char password[] = "Tim+Hartsuijker";  
char db[] = "tim.hartsuijker";

// wifi information
const char* ssid = "OmniEnergy"; 
const char* pass = "Kilowattuur";

char INSERT_DATA[] = "INSERT INTO test (value) VALUES ('%d')";
char REQUEST_DATA[] = "SELECT `value` FROM `test`";
char query[128];

WiFiClient client;          
MySQL_Connection conn((Client *)&client);
MySQL_Cursor cur = MySQL_Cursor(&conn);

void WiFiConnect()
{
  // Connect to Wi-Fi network with SSID and password
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
void executeInsertQuery()
{
  Serial.println("Connecting to database");

  if(conn.connect(server_addr, 3306, user, password, db)) 
  {
    int Baka = random(1, 10);
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    sprintf(query, INSERT_DATA, Baka);
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
void executeSelectQuery()
{
  if(conn.connect(server_addr, 3306, user, password, db))
  {
    row_values *row = NULL;
    long value;
    int val = 0;

    delay(1000);

    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    sprintf(query, REQUEST_DATA);
    // Execute the query
    cur_mem->execute(query);
    // Fetch the columns (required) but we don't use them.k
    column_names *columns = cur_mem->get_columns();

   // Read the row (we are only expecting the one)
   do 
    {
      row = cur_mem->get_next_row();
      if (row != NULL) 
      {
        value = atol(row->values[0]);
        Serial.println(atol(row->values[0]));
        val++;
      }
    } while (row != NULL);
    // Deleting the cursor also frees up memory used
    delete cur_mem;

    delay(500);
    conn.close();
  }else
  Serial.println("Connection failed.");
}

void setup() 
{
  Serial.begin(115200);
  WiFiConnect();
  executeInsertQuery();
  executeSelectQuery();
}

void loop() 
{

}