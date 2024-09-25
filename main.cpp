/**
 * @file main.cpp
 * @author Barnabas Mulu Boka,
 * Amanuel Hayele Tsegay,
 * Habteab Teame,
 * Teklit Amanuel Ghebremichael,
 *Ali Shahab Rezaii
 */

#include "include/openweathermap.h"
#include "include/weatherapi_R3.h"
#include "include/wifi.h"
#include "include/worldtimeapi.h"
#include "mbed.h"
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <valarray>

#define JSON_NOEXCEPTION
#include "DFRobot_RGBLCD.h"      // Library for LCD screen
#include "HTS221/HTS221Sensor.h" // Library for lcdScreen
#include "include/global_variables.h"
#include "json.hpp"

#include "BufferedSerial.h"
#include "TLSSocket.h"
#include "rtos.h"

BufferedSerial uart(USBTX, USBRX, 115200);
using json = nlohmann::json;

// Define UART serial object
std::string location;

Thread network_thread;

#define WAIT_TIME_MS 2000ms
#define WAIT_SNOOZE 100ms
#define WAIT_BOOT 2000ms
#define WAIT_error 400ms
#define WAIT_TIME_MS_small 200ms
#define STACK_SIZE 8192 // Increase stack size to 8KB
using json = nlohmann::json;

static char buffer[2000];
bool tempTrue = true;

Ticker snoozeTimer;
// THREAD
static Thread buzzer_Thread(osPriorityRealtime7, STACK_SIZE);

// Mutex
#include "rtos.h"

Mutex network_mutex;
Mutex weather_mutex;

Mutex datetime_mutex;
Mutex data_mutex;
// GLOBAL VARIABLES
std::string formatted_time;
std::string alarm_status;

nsapi_error_t status;
nsapi_error_t result;
SocketAddress ip_address;
SocketAddress address1;
NetworkInterface *network = NetworkInterface::get_default_instance();

// Variables for the API and Parse to struct.
static std::string condition, date, wind_Dir, city;
static int epoch, temperature, hour, minute, second, year, month, day;
static double latitude, longitude, wind_Kph, wind_Degree, pressure_In, humiditY,
    Cloud, vis_Km, gust_Kph;
std::string localtime_str;
static unsigned int setHour, setMinute;

// Initialize the digital pin LED1 as an output
DigitalOut led1(LED1);
DFRobot_RGBLCD lcdScreen(16, 2, PB_9, PB_8); // Instantiate the LCD screen
PwmOut buzzer(PB_4);                         // Speaker

// Button declarations
InterruptIn button_increase_2(PC_5, PullDown); // Increase
InterruptIn button_decrease_3(PC_4, PullDown); // Decrease
InterruptIn button_select_4(PC_3, PullDown);   // Select and back function.

// State declarations
enum State {
  ACTIVE,
  SNOOZE,
  MUTED,
  DEFINED,
  UNDEFINED,
  DISABLED,
  ENABLED,
  WEATHER,
  CALENDAR,
  ADVANCED,
};

State currentState = UNDEFINED;
State hour_State = UNDEFINED;
State minute_State = UNDEFINED;
State alarmState = UNDEFINED;
State weatherState = UNDEFINED;
State MAIN_MENU_STATE = UNDEFINED;
State recurringState = ENABLED;
State WeatherScreenState = UNDEFINED;

// Function prototypes
void increaseButtonHandler();
void decreaseButtonHandler();
void selectButtonHandler();
void navigateButtonHandler();
void setupInterrupts();
bool isButtonPressed(InterruptIn &button);

void Bootingprocess_gp35();
void DateTimeScreen();
void AlarmScreen();
void TempHumScreen();
void WeatherScreen();
void TopNewsScreen();

void BuzzerAlarm();
void snoozeAlarm();
void muteAlarm();
void disableAlarm();
void enableAlarm();
void MenuPicker2();
void soundAlarm();

bool Buzzer_Running = false;
bool Is_Page_Selected = false;
int Selected_Page = 0;
std::string menuItem[] = {
    "Date and Time", "Alarm",        "Temp and Humidity", "Weather Forcast", "IP weather Forcast",
    "Disable alarm", "Enable alarm", "Top recent news"};
std::string menuItem2[] = {"Snooze alarm", "Mute alarm"};

void soundAlarm() {
  alarmState = ACTIVE;
  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Alarm (A) %.2d:%.2d", setHour, setMinute);
  buzzer_Thread.start(callback(BuzzerAlarm));
  Buzzer_Running = true;

  // Mute alarm after 10 minutes 600000ms
  ThisThread::sleep_for(500ms);
  if (alarmState == ACTIVE) {
    muteAlarm();
  }
}
//---------------------------------FUNCTION START:
//NetworkAccess--------------------------------------
// Function to initialize the network
void initializeNetwork() {
  printf("Initializing network...\n");

  //    nsapi_error_t status;
  do {
    status = network->connect();
    if (status != NSAPI_ERROR_OK) {
      printf("Failed to connect to network: %d\n", status);
      ThisThread::sleep_for(2000ms);
    }
  } while (status != NSAPI_ERROR_OK);

  printf("Connected to network.\n");

  //    SocketAddress ip_address;
  status = network->get_ip_address(&ip_address);
  if (status != NSAPI_ERROR_OK) {
    printf("Failed to get IP address: %d\n", status);
  } else {
    printf("IP address: %s\n",
           ip_address.get_ip_address() ? ip_address.get_ip_address() : "None");
  }
}
//---------------------------------FUNCTION DONE:
//NetworkAccess--------------------------------------

void DATESCREEN() {
  bool whileToggle = true;
  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Date/Time");

  while (whileToggle) {
    led1 =
        !led1; // Toggle "running" LED to indicate the function is being called

    data_mutex.lock();
    std::string display_time = formatted_time;
    std::string display_alarm = alarm_status;
    data_mutex.unlock();

    // Debug output to console
    printf(" %s\n", display_time.c_str());
    printf("Alarm: %s\n", display_alarm.c_str());

    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("%s", display_time.empty() ? "No Time Data"
                                                : display_time.c_str());
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("%s", display_alarm.empty() ? "No Alarm"
                                                 : display_alarm.c_str());

    ThisThread::sleep_for(200ms); // Refresh every 200ms to check data

    if (isButtonPressed(button_decrease_3)) {
      lcdScreen.clear();
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("Exiting DATESCREEN");
      ThisThread::sleep_for(350ms);
      whileToggle = false;
    }
  }
}
//---------------------------------FUNCTION DONE:
//API_time------------------------------------------

void increaseButtonHandler() {
  // Implement increase button handler logic here
}

void decreaseButtonHandler() {
  // Implement decrease button handler logic here
}

void selectButtonHandler() {
  // Implement select button handler logic here
}

void navigateButtonHandler() {
  // Implement navigate button handler logic here
}

void setupInterrupts() {
  button_increase_2.rise(&increaseButtonHandler);
  button_decrease_3.rise(&decreaseButtonHandler);
  button_select_4.rise(&selectButtonHandler);
}

bool isButtonPressed(InterruptIn &button) {
  const auto debounceDelay = 50ms;
  if (button.read()) {
    ThisThread::sleep_for(debounceDelay);
    if (button.read()) {
      return true;
    }
  }
  return false;
}
void newWeather();
void newweather(){
bool whileToggle = true;
lcdScreen.clear();
lcdScreen.setCursor(0, 0);
lcdScreen.printf("Loading..");
  while (whileToggle) {
        
        led1 = !led1; // Toggle "running" LED
        TLSSocket socket;
        socket.set_timeout(1000);

        result = socket.open(network);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(result));
            continue;
        }
        const char host[] = "api.weatherapi.com";
        result = network->gethostbyname(host, &address1);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to get IP address of host %s: %s\n", host, get_nsapi_error_string(result));
            continue;
        }
        printf("IP address of server %s is %s\n", host, address1.get_ip_address());

        address1.set_port(443);
        result = socket.set_root_ca_cert(weatherapi_R3);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to set root certificate of the web site: %s\n", get_nsapi_error_string(result));
            continue;
        }
        socket.set_hostname(host);

        result = socket.connect(address1);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to connect to server at %s: %s\n", host, get_nsapi_error_string(result));
            continue;
        }

        printf("Successfully connected to server %s\n", host);
        const char request[] = "GET /v1/current.json?key=5b4602368fab46b9a8f35313231105&q=158.37.240.80&localtime_epoch HTTP/1.1\r\n"
                               "Host: api.weatherapi.com\r\n"
                               "Accept: application/json\r\n"
                               "Connection: close\r\n"
                               "\r\n";

        // Send request
        weather_mutex.lock();
        result = send_request(&socket, request);
        weather_mutex.unlock();

        if (result < 0) {
            printf("Failed to send request: %d\n", result);
            continue;
        }
        // Read response
        result = read_response(&socket, buffer, sizeof(buffer));

        if (result < 0) {
            printf("Failed to read response: %d\n", result);
            continue;
        }

        char *json_begin = strchr(buffer, '{');
        char *json_end = strrchr(buffer, '}');

        if (json_begin == nullptr || json_end == nullptr) {
            printf("Failed to find JSON in response\n");
            continue;
        }
        json_end[1] = 0;

        json api_json = json::parse(json_begin);

        if (api_json.is_discarded()) {
            printf("The input is invalid JSON\n");
            continue;
        }
        std::string localtime_str;
        api_json["location"]["localtime"].get_to(localtime_str);
        api_json["location"]["localtime_epoch"].get_to(epoch);
        api_json["location"]["lat"].get_to(latitude);
        api_json["location"]["lon"].get_to(longitude);
        api_json["location"]["name"].get_to(city);
        api_json["current"]["temp_c"].get_to(temperature);
        api_json["current"]["condition"]["text"].get_to(condition);

        api_json["current"]["wind_kph"].get_to(wind_Kph);
        api_json["current"]["wind_degree"].get_to(wind_Degree);
        api_json["current"]["wind_dir"].get_to(wind_Dir);
        api_json["current"]["pressure_in"].get_to(pressure_In);
        api_json["current"]["humidity"].get_to(humiditY);
        api_json["current"]["cloud"].get_to(Cloud);
        api_json["current"]["vis_km"].get_to(vis_Km);
        api_json["current"]["gust_kph"].get_to(gust_Kph);
        lcdScreen.clear();
        lcdScreen.setCursor(0, 0);
        lcdScreen.printf("weather");
        ThisThread::sleep_for(500ms);

        // Clear RGB display
        lcdScreen.clear();
        lcdScreen.setCursor(0, 0);
        // Display the weather on the RGB LCD
        lcdScreen.printf("%s", condition.c_str());

        lcdScreen.setCursor(0, 1);
        lcdScreen.printf("%d C", temperature);
        ThisThread::sleep_for(1000ms);
        if (isButtonPressed(button_decrease_3)) {
            lcdScreen.clear();
            lcdScreen.setCursor(0, 1);
            lcdScreen.printf("Back to menu");
            ThisThread::sleep_for(350ms);
            Is_Page_Selected = false;
            currentState = UNDEFINED;
            weatherState = UNDEFINED;
            return;
        }

        }
}

void WeatherScreen() {
    // Clear the screen and prompt for location entry
    //currentState = WEATHER;
    bool whileToggle = true;
    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Weather");
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("Enter location? Y/N");

    char location[100];
    size_t index = 0;
    bool locationEntered = false;

    while (whileToggle) {
        if (isButtonPressed(button_increase_2)) { // Assuming 'Yes' to enter location
            lcdScreen.clear();
            lcdScreen.setCursor(0, 0);
            lcdScreen.printf("Enter Location:");
            lcdScreen.setCursor(0, 1);
            while (true) {
                if (uart.readable()) {
                    char c;
                    uart.read(&c, 1);
                    if (c == '\r' || c == '\n') {
                        location[index] = '\0'; // Null-terminate the string
                        locationEntered = true;
                        break;
                    }
                    if (c == '\b' && index > 0) { // Handle backspace
                        index--;
                        lcdScreen.printf("\b \b"); // Remove char from screen
                    } else if (index < sizeof(location) - 1) {
                        location[index++] = c;
                        lcdScreen.printf("%c", c); // Display char on screen
                    }
                }
            }
            if (locationEntered) {
                break; // Break the loop if location entered
            }
        } else if (isButtonPressed(button_decrease_3)) { // Assuming 'No' to skip
            return; // Exit the function if 'No'
        }
        ThisThread::sleep_for(100ms); // Debounce delay
    }

    // Proceed with network request after location entered
    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Loading...");
    lcdScreen.setCursor(0, 1);

    TLSSocket socket;
    socket.set_timeout(1000);
    if (socket.open(network) != NSAPI_ERROR_OK) {
        lcdScreen.printf("Socket open failed");
        ThisThread::sleep_for(2000ms);
        return;
    }

    const char host[] = "api.openweathermap.org";
    SocketAddress ip_address;
    if (network->gethostbyname(host, &ip_address) != NSAPI_ERROR_OK) {
        lcdScreen.printf("DNS fail");
        ThisThread::sleep_for(2000ms);
        socket.close();
        return;
    }
    ip_address.set_port(443);
    if (socket.set_root_ca_cert(openweathermap_Certificate_group35) != NSAPI_ERROR_OK) {
        lcdScreen.printf("Cert fail");
        ThisThread::sleep_for(2000ms);
        socket.close();
        return;
    }
    if (socket.connect(ip_address) != NSAPI_ERROR_OK) {
        lcdScreen.printf("Connect fail");
        ThisThread::sleep_for(2000ms);
        socket.close();
        return;
    }

    char request[256];
    snprintf(
        request, sizeof(request),
        "GET /data/2.5/weather?q=%s&appid=1258f4e535eb59f79521d47d29b86083 HTTP/1.1\r\n"
        "Host: api.openweathermap.org\r\n"
        "Connection: close\r\n\r\n",
        location);
    if (socket.send(request, strlen(request)) < 0) {
        lcdScreen.printf("Request fail");
        ThisThread::sleep_for(2000ms);
        socket.close();
        return;
    }

    char buffer[2000];
    int len = socket.recv(buffer, sizeof(buffer) - 1);
    if (len <= 0) {
        lcdScreen.printf("Response fail");
        ThisThread::sleep_for(2000ms);
        socket.close();
        return;
    }
    buffer[len] = '\0';

    socket.close(); // Always ensure the socket is closed after completion

    // Locate the start and end of the JSON content
    char *json_begin = strchr(buffer, '{');
    char *json_end = strrchr(buffer, '}');

    if (json_begin == nullptr || json_end == nullptr) {
        lcdScreen.printf("JSON not found");
        ThisThread::sleep_for(2000ms);
        return;
    }
    json_end[1] = '\0';

    json response = json::parse(json_begin, nullptr, false);
    if (response.is_discarded()) {
        lcdScreen.printf("Invalid JSON");
        ThisThread::sleep_for(2000ms);
    } else {
        float temp = response["main"]["temp"].get<float>() - 273.15;
        std::string weather =
        response["weather"][0]["description"].get<std::string>();
        lcdScreen.clear();
        lcdScreen.setCursor(0, 0);
        lcdScreen.printf("%s", weather.c_str());
        lcdScreen.setCursor(0, 1);
        lcdScreen.printf("%.2f C", temp);

        while (true) {
            if (isButtonPressed(button_decrease_3)) {
                lcdScreen.clear();
                lcdScreen.setCursor(0, 1);
                lcdScreen.printf("Back to menu");
                ThisThread::sleep_for(350ms);
                Is_Page_Selected = false;
                weatherState = UNDEFINED;
                return;
            }
            ThisThread::sleep_for(100ms); // Small delay to prevent busy-waiting
        }
    }
}


void Bootingprocess_gp35() {
  bool whileToggle = true;
  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Booting");

  while (whileToggle) {
    TLSSocket socket;
    socket.set_timeout(1000);
    nsapi_error_t status = socket.open(network);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(status));
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    const char host[] = "api.weatherapi.com";
    status = network->gethostbyname(host, &ip_address);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to get IP address of host %s: %s\n", host,
             get_nsapi_error_string(status));
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    ip_address.set_port(443);
    status = socket.set_root_ca_cert(weatherapi_R3);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to set root certificate: %s\n",
             get_nsapi_error_string(status));
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    socket.set_hostname(host);

    status = socket.connect(ip_address);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to connect to server: %s\n",
             get_nsapi_error_string(status));
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    const char request[] = "GET "
                           "/v1/"
                           "current.json?key=5b4602368fab46b9a8f35313231105&q="
                           "158.37.240.80&localtime_epoch HTTP/1.1\r\n"
                           "Host: api.weatherapi.com\r\n"
                           "Accept: application/json\r\n"
                           "Connection: close\r\n"
                           "\r\n";
    status = socket.send(request, sizeof(request));
    if (status < 0) {
      printf("Failed to send request: %d\n", status);
      socket.close();
      return;
    }

    char buffer[2000];
    int len = socket.recv(buffer, sizeof(buffer) - 1);
    if (len <= 0) {
      printf("Failed to read response: %d\n", len);
      socket.close();
      return;
    }
    buffer[len] = '\0'; // Ensure null-termination

    socket.close(); // Close the socket after receiving data

    // Locate the JSON part of the response
    char *json_start = strchr(buffer, '{');
    if (!json_start) {
      printf("Failed to find JSON in response\n");
      continue;
    }

    json api_json = json::parse(json_start, nullptr, false);
    if (api_json.is_discarded()) {
      printf("The input is invalid JSON\n");
      continue;
    } else {
      // Extracting the relevant data
      int epoch;
      double latitude, longitude;
      std::string city;

      api_json["location"]["localtime_epoch"].get_to(epoch);
      api_json["location"]["lat"].get_to(latitude);
      api_json["location"]["lon"].get_to(longitude);
      api_json["location"]["name"].get_to(city);

      // Update display or notify other tasks
      lcdScreen.clear();
      lcdScreen.setCursor(0, 0);
      lcdScreen.printf("Unix epoch time: ");
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("%d", epoch);
      ThisThread::sleep_for(WAIT_BOOT);
      lcdScreen.clear();
      lcdScreen.setCursor(0, 0);
      lcdScreen.printf("Lat: %.4f", latitude);
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("Lon: %.4f", longitude);
      ThisThread::sleep_for(WAIT_BOOT);
      lcdScreen.clear();
      lcdScreen.setCursor(0, 0);
      lcdScreen.printf("City: ");
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("%s", city.c_str());
      ThisThread::sleep_for(WAIT_BOOT);
      return;
    }

    ThisThread::sleep_for(350ms);
    if (isButtonPressed(button_select_4)) {
      lcdScreen.clear();
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("Back to menu");
      ThisThread::sleep_for(350ms);
      Is_Page_Selected = false;
      whileToggle = false;
      currentState = UNDEFINED;
      return;
    }
  }
}

void AlarmScreen() {
  bool whileToggle = true;
  Is_Page_Selected = false;

  // Menu for active or snoozed alarm
  if (alarmState == ACTIVE || alarmState == SNOOZE ||
      recurringState == DISABLED) {
    while (whileToggle) {
      if (!Is_Page_Selected) {
        MenuPicker2();
      } else {
        switch (Selected_Page) {
        case 1:
          snoozeAlarm();
          whileToggle = false;
          break;
        case 2:
          muteAlarm();
          whileToggle = false;
          break;
        case 3:
          Is_Page_Selected = true;
          whileToggle = false;
          break;
        }
      }
    }
  }

  ThisThread::sleep_for(100ms);

  // Menu for defining or editing an alarm
  while (alarmState == DEFINED || alarmState == MUTED ||
         recurringState == DISABLED) {
    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Edit? Yes");
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("Alarm: %.2d:%.2d", setHour, setMinute);
    ThisThread::sleep_for(100ms);

    if (isButtonPressed(button_select_4)) {
      alarmState = UNDEFINED;
      hour_State = UNDEFINED;
      minute_State = UNDEFINED;
      recurringState = ENABLED;
      break;
    }
    if (isButtonPressed(button_decrease_3)) {
      Is_Page_Selected = false;
      if (buzzer_Thread.get_state() != Thread::Running) {
        buzzer_Thread.start(BuzzerAlarm);
        printf("========================Start Buzzer "
               "thread=============================\n");
      }
      return;
    }
  }

  // Set or edit the alarm
  while (alarmState == UNDEFINED && recurringState == ENABLED) {
    while (hour_State == UNDEFINED) {
      lcdScreen.clear();
      lcdScreen.setCursor(0, 0);
      lcdScreen.printf("Set Alarm");
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("Hour: %.2d:%.2d", setHour, setMinute);
      ThisThread::sleep_for(100ms); // Button debounce
      if (isButtonPressed(button_increase_2)) {
        if (setHour < 23) {
          setHour++;
        }
      }

      if (isButtonPressed(button_decrease_3)) {
        if (setHour > 0) {
          setHour--;
        }
      }

      if (isButtonPressed(button_select_4)) {
        lcdScreen.clear();
        lcdScreen.setCursor(0, 0);
        lcdScreen.printf("Hour is set");
        hour_State = DEFINED;
        ThisThread::sleep_for(100ms); // Button debounce
      }
    }

    while (minute_State == UNDEFINED) {
      lcdScreen.clear();
      lcdScreen.setCursor(0, 0);
      lcdScreen.printf("Set Alarm");
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("Minute: %.2d:%.2d", setHour, setMinute);
      ThisThread::sleep_for(100ms); // Button debounce
      if (isButtonPressed(button_increase_2)) {
        if (setMinute < 60) {
          setMinute++;
        }
      }

      if (isButtonPressed(button_decrease_3)) {
        if (setMinute > 0) {
          setMinute--;
        }
      }

      if (isButtonPressed(button_select_4)) {
        printf("Minute is set\n");
        lcdScreen.clear();
        lcdScreen.setCursor(0, 1);
        lcdScreen.printf("Minute is set");
        ThisThread::sleep_for(100ms); // Button debounce
        minute_State = DEFINED;
      }
    }

    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Time set to:");
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("%.2d:%.2d", setHour, setMinute);
    ThisThread::sleep_for(300ms);

    if (buzzer_Thread.get_state() != Thread::Running) {
      buzzer_Thread.start(BuzzerAlarm);
      printf("========================Start Buzzer "
             "thread=============================\n");
    }

    Is_Page_Selected = false;
    alarmState = DEFINED;
  }
}

void TempHumScreen() {
  // Implement the DateTime screen logic here
  bool whileToggle = true;
  float Hum, Temp;

  DevI2C i2c(PB_11, PB_10);
  HTS221Sensor sensor(&i2c); // Humidity and temp

  lcdScreen.clear();
  while (whileToggle) {
    // Init HTS221 with default params (NULL)
    if (sensor.init(NULL) != 0) {
      printf("Failed to initialize HTS221 device!\n");
    }
    // Power up the HTS221 device
    if (sensor.enable() != 0) {
      printf("Failed to power up HTS221 device!\n");
    }
    if (isButtonPressed(button_decrease_3)) {
      lcdScreen.clear();
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("Back to menu");
      ThisThread::sleep_for(400ms);
      Is_Page_Selected = false;
      whileToggle = false;
    }
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Temp: %.fC", Temp);
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("Humidity: %.1f %%", Hum);
    ThisThread::sleep_for(100ms);
  }
}

void TopNewsScreen() {
  // Implement the DateTime screen logic herehere
  bool whileToggle = true;
  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Top News");
  if (isButtonPressed(button_decrease_3)) {
    lcdScreen.clear();
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("Back to menu");
    ThisThread::sleep_for(400ms);
    Is_Page_Selected = false;
    whileToggle = false;
  }
}

void snoozeAlarm() {
  alarmState = SNOOZE;
  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Alarm Snoozed");

  lcdScreen.setCursor(0, 1);
  lcdScreen.printf("Alarm (S) %.2d:%.2d", setHour, setMinute);
  ThisThread::sleep_for(WAIT_SNOOZE);

  // Snooze for 5 minutes 300000ms
  ThisThread::sleep_for(500ms);
  if (alarmState == SNOOZE) {
    soundAlarm();
  }
}
void muteAlarm() {
  alarmState = MUTED;
  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Alarm Muted");
  ThisThread::sleep_for(300ms);
  if (Buzzer_Running) {
    Buzzer_Running = false;
    buzzer_Thread.terminate();
  }

  lcdScreen.setCursor(0, 1);
  lcdScreen.printf("Alarm %.2d:%.2d", setHour, setMinute);
  ThisThread::sleep_for(1000ms);
}

void enableAlarm() {
  if (alarmState == DISABLED) {
    recurringState = ENABLED;
    alarmState = DEFINED; // Corrected assignment
    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Alarm Enabled");

    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("Alarm %.2d:%.2d", setHour, setMinute);
    ThisThread::sleep_for(1000ms);
  }
}

void disableAlarm() {
  if (alarmState == DEFINED || alarmState == MUTED || alarmState == SNOOZE ||
      alarmState == ENABLED) {
    alarmState = DISABLED;
    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Alarm Disabled");

    if (Buzzer_Running) {
      Buzzer_Running = false;
      buzzer_Thread.terminate();
    }

    ThisThread::sleep_for(1000ms);
  }
}

void MenuPicker2() {
  int countItem = 1;
  bool whileToggle = true;

  while (whileToggle) {
    lcdScreen.clear();
    lcdScreen.setCursor(0, 0);
    lcdScreen.printf("Menu:");
    lcdScreen.setCursor(0, 1);
    lcdScreen.printf("%s", menuItem2[countItem - 1].c_str());
    ThisThread::sleep_for(100ms);

    if (isButtonPressed(button_increase_2)) {
      countItem = (countItem < 2) ? countItem + 1 : 1;
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("%s              ", menuItem2[countItem - 1].c_str());
      ThisThread::sleep_for(100ms);
    } else if (isButtonPressed(button_decrease_3)) {
      countItem = (countItem > 1) ? countItem - 1 : 2;
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("%s              ", menuItem2[countItem - 1].c_str());
      ThisThread::sleep_for(100ms);
    } else if (isButtonPressed(button_select_4)) {
      Selected_Page = countItem;
      Is_Page_Selected = true;
      whileToggle = false;
    }
  }
}

void MenuPicker() {
  int countItem = 1;
  bool whileToggle = true;

  lcdScreen.clear();
  lcdScreen.setCursor(0, 0);
  lcdScreen.printf("Menu:");
  lcdScreen.setCursor(0, 1);
  lcdScreen.printf("%s", menuItem[countItem - 1].c_str());
  ThisThread::sleep_for(100ms);
  while (whileToggle) {
    // If the Alarm is NOT ACTIVE
    // if (alarmState != ACTIVE) {
    if (isButtonPressed(button_increase_2)) {
      countItem = (countItem < 8) ? countItem + 1 : 1;
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("%s              ", menuItem[countItem - 1].c_str());
      ThisThread::sleep_for(100ms);
    } else if (isButtonPressed(button_decrease_3)) {
      countItem = (countItem > 1) ? countItem - 1 : 8;
      lcdScreen.setCursor(0, 1);
      lcdScreen.printf("%s              ", menuItem[countItem - 1].c_str());
      ThisThread::sleep_for(100ms);
    } else if (isButtonPressed(button_select_4)) {
      Selected_Page = countItem;
      Is_Page_Selected = true;
      whileToggle = false;
    }
  }
}
void handleMenuSelection() {
  while (true) {
    MenuPicker();
    switch (Selected_Page) {
    case 1:
      DATESCREEN();
      break;
    case 2:
      AlarmScreen();
      break;
    case 3:
      TempHumScreen();
      break;
    case 4:
      weatherState = ACTIVE;
      newweather();
      break;
    case 5:
      weatherState = ACTIVE;
      WeatherScreen();
      break;
    case 6:
      disableAlarm();
      break;
    case 7:
      enableAlarm();
      break;
    default:
      TopNewsScreen();
      break;
    }
  }
}

void BuzzerAlarm() {
  buzzer.period(4.0f); // Set buzzer period to 4 seconds (0.25 Hz)
  buzzer.write(0.5f);  // 50% duty cycle to make sound
  bool whileToggle = true;

  while (whileToggle) { // Continuous loop to check the condition
    datetime_mutex.lock();
    int current_hour = hour;
    int current_minute = minute;
    datetime_mutex.unlock();

    printf("----------------ALARM--------------------\n");
    printf("localtime: %02d:%02d \n", current_hour, current_minute);
    printf("Set time: %02d:%02d \n", setHour, setMinute);

    ThisThread::sleep_for(200ms);
    if ((setMinute == current_minute && setHour == current_hour) &&
        (alarmState == DEFINED) && (alarmState != MUTED)) {
      printf("Alarm time reached! Turning on the buzzer.\n");
      alarmState = ACTIVE;

      // Activate the buzzer
      // Buzzer will sound for 1 minute or until muted
      int buzzer_duration = 60; // 60 seconds
      while (buzzer_duration > 0 && alarmState == ACTIVE) {
        ThisThread::sleep_for(1000ms); // Sleep for 1 second
        buzzer_duration--;
      }

      // Turn off the buzzer after duration
      buzzer.write(0.0f);

      // If the alarm state is still active, we reset it to defined
      if (alarmState == ACTIVE) {
        alarmState = DEFINED;
      }

      // whileToggle = false;
    }

    // ThisThread::sleep_for(200ms);  // Check every 200 milliseconds
  }
}

void NetworkRequestThread() {
  bool whileToggle = true;
  while (whileToggle) {
    while (weatherState == ACTIVE) {
    ThisThread::sleep_for(500ms);
    }
    
     network_mutex.lock(); // Lock the mutex before making network requests

    /*  /*if (currentState == WEATHER) {
          // Perform network request for weather
          TLSSocket socket;
          socket.set_timeout(1000);
          nsapi_error_t status = socket.open(network);

          if (status != NSAPI_ERROR_OK) {
            printf("Failed to open TLSSocket: %s\n",
            get_nsapi_error_string(status)); 
            network_mutex.unlock(); // Unlock the mutex if there's an error 
            ThisThread::sleep_for(10000ms); // Retry after 10 seconds 
            continue;
          }

          const char host[] = "api.openweathermap.org";
          status = network->gethostbyname(host, &ip_address);

          if (status != NSAPI_ERROR_OK) {
            printf("Failed to get IP address of host %s: %s\n", host,
            get_nsapi_error_string(status)); 
            network_mutex.unlock(); // Unlock the mutex if there's an error 
            ThisThread::sleep_for(10000ms); // Retry after 
            continue;
          }

          ip_address.set_port(443);
          status = socket.set_root_ca_cert(openweathermap_Certificate_group35);

          if (status != NSAPI_ERROR_OK) {
            printf("Failed to set root certificate: %s\n",
            get_nsapi_error_string(status)); network_mutex.unlock(); // Unlock the mutex if there's an error 
            ThisThread::sleep_for(10000ms); // Retry after 10 seconds 
            continue;
          }

          socket.set_hostname(host);

          status = socket.connect(ip_address);

          if (status != NSAPI_ERROR_OK) {
            printf("Failed to connect to server: %s\n",
            get_nsapi_error_string(status)); network_mutex.unlock(); // Unlock the mutex if there's an error 
            ThisThread::sleep_for(10000ms); // Retry after 10 seconds 
            continue;
          }

          // Format the request for weather data
          char request[256];
          snprintf(request, sizeof(request), "GET /data/2.5/weather?q=%s&appid=1258f4e535eb59f79521d47d29b86083
            HTTP/1.1\r\n" "Host: api.openweathermap.org\r\n" 
            "Accept: application/json\r\n" 
            "Connection: close\r\n"
            "\r\n", "London"); // Replace "London" with actual location if needed

          // Send request
          status = send_request(&socket, request);

          if (status < 0) {
              printf("Failed to send request: %d\n", status);
              network_mutex.unlock(); // Unlock the mutex if there's an error
              ThisThread::sleep_for(10000ms); // Retry after 10 seconds
              continue;
          }

          // Read response
          status = read_response(&socket, buffer, sizeof(buffer));

          if (status < 0) {
              printf("Failed to read response: %d\n", status);
              network_mutex.unlock(); // Unlock the mutex if there's an error
              ThisThread::sleep_for(10000ms); // Retry after 10 seconds
              continue;
          }

          char *json_begin = strchr(buffer, '{');
          char *json_end = strrchr(buffer, '}');

          if (json_begin == nullptr || json_end == nullptr) {
              printf("Failed to find JSON in response\n");
              network_mutex.unlock(); // Unlock the mutex if there's an error
              ThisThread::sleep_for(10000ms); // Retry after 10 seconds
              continue;
          }
          json_end[1] = 0;

          json api_json = json::parse(json_begin);

          if (api_json.is_discarded()) {
              printf("The input is invalid JSON\n");
              network_mutex.unlock(); // Unlock the mutex if there's an error
              ThisThread::sleep_for(10000ms); // Retry after 10 seconds
              continue;
          }

          float temp_kelvin = api_json["main"]["temp"];
          std::string weather_description =
      api_json["weather"][0]["description"];

          float temperature = temp_kelvin - 273.15; // Convert from Kelvin to
      Celsius

          // Display on the LCD screen
          lcdScreen.clear();
          lcdScreen.setCursor(0, 0);
          lcdScreen.printf("Condition: %s", weather_description.c_str());
          lcdScreen.setCursor(0, 1);
          lcdScreen.printf("Temp: %.1f C", temperature);

      } else if (currentState == UNDEFINED) {*/
    // Perform network request for time
    TLSSocket socket;
    socket.set_timeout(1000);
    nsapi_error_t status = socket.open(network);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to open TLSSocket: %s\n", get_nsapi_error_string(status));
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    const char host[] = "worldtimeapi.org";
    status = network->gethostbyname(host, &ip_address);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to get IP address of host %s: %s\n", host,
             get_nsapi_error_string(status));
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    ip_address.set_port(443);
    status = socket.set_root_ca_cert(worldtimeapi_Certificate_group35);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to set root certificate: %s\n",
             get_nsapi_error_string(status));
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    socket.set_hostname(host);

    status = socket.connect(ip_address);

    if (status != NSAPI_ERROR_OK) {
      printf("Failed to connect to server: %s\n",
             get_nsapi_error_string(status));
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    const char request[] = "GET /api/ip HTTP/1.1\r\n"
                           "Host: worldtimeapi.org\r\n"
                           "Accept: application/json\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    // Send request
    network_mutex.lock();
    status = send_request(&socket, request);
    network_mutex.unlock();

    if (status < 0) {
      printf("Failed to send request: %d\n", status);
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    status = read_response(&socket, buffer, sizeof(buffer));

    if (status < 0) {
      printf("Failed to read response: %d\n", status);
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }

    char *json_begin = strchr(buffer, '{');
    char *json_end = strrchr(buffer, '}');

    if (json_begin == nullptr || json_end == nullptr) {
      printf("Failed to find JSON in response\n");
      network_mutex.unlock();         // Unlock the mutex if there's an error
      ThisThread::sleep_for(10000ms); // Retry after 10 seconds
      continue;
    }
    json_end[1] = 0;

    json api_json = json::parse(json_begin);

    if (api_json.is_discarded()) {
      printf("Invalid JSON\n");
      network_mutex.unlock();       // Unlock the mutex if there's an error
      ThisThread::sleep_for(100ms); // Retry after 10 seconds
      continue;
    }

    std::string datetime_str = api_json["datetime"];
    int year, month, day, hour, minute, second;
    sscanf(datetime_str.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day,
           &hour, &minute, &second);

    struct tm time_struct = {};
    time_struct.tm_year = year - 1900;
    time_struct.tm_mon = month - 1;
    time_struct.tm_mday = day;
    time_struct.tm_hour = hour;
    time_struct.tm_min = minute;
    time_struct.tm_sec = second;

    time_t raw_time = mktime(&time_struct);
    struct tm *local_time = localtime(&raw_time);

    char time_buffer[30];
    strftime(time_buffer, sizeof(time_buffer), "%a %d %b %H:%M", local_time);

    data_mutex.lock();
    formatted_time = std::string(time_buffer);

    // Check if the current time matches the set alarm time
    if (hour == setHour && minute == setMinute &&
        (alarmState == DEFINED || alarmState == ACTIVE)) {
      alarmState = ACTIVE;

      buzzer_Thread.start(BuzzerAlarm);
    }

    switch (alarmState) {
    case DEFINED:
      alarm_status =
          "Alarm  " + std::to_string(setHour) + ":" + std::to_string(setMinute);
      break;
    case ACTIVE:
      alarm_status = "Alarm (A) " + std::to_string(setHour) + ":" +
                     std::to_string(setMinute);
      break;
    case SNOOZE:
      alarm_status = "Alarm (S) " + std::to_string(setHour) + ":" +
                     std::to_string(setMinute);
      break;
    case MUTED:
      alarm_status =
          "Alarm  " + std::to_string(setHour) + ":" + std::to_string(setMinute);
      break;
    default:
      alarm_status.clear();
      break;
    }
    data_mutex.unlock();
  }

  network_mutex
      .unlock(); // Unlock the mutex after the network request is complete

  ThisThread::sleep_for(500ms); // Update every minute
    
}

int main() {
  setupInterrupts();

  // Initialize RGB display
  lcdScreen.init();
  // Turn RGB Display on
  lcdScreen.display();
  // Clear RGB display
  lcdScreen.clear();
  ThisThread::sleep_for(100ms);

  initializeNetwork();
  Bootingprocess_gp35();
  ThisThread::sleep_for(WAIT_BOOT); // Retry after 10 seconds
  Thread networkRequestThread(osPriorityNormal, STACK_SIZE);
  networkRequestThread.start(NetworkRequestThread);

    
  handleMenuSelection();

  return 0;
}