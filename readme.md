# Display Your Location's Weather Data

This weather data displaying system is designed to provide real-time weather information based on the user's location. It utilizes the OpenWeather API to fetch weather data and display it in a user-friendly format.

![Processor Animation](https://github.com/Bhanuka-Anjana/Configurable-Weather-Station/blob/main/images/processor%20animation.gif) 
##

<img align="left" height="300px" width="400px" alt="Github" src="https://github.com/Bhanuka-Anjana/Configurable-Weather-Station/blob/main/images/setup.jpg" />

## Features

- **Customizable User Location:** Users have the flexibility to configure the longitude and latitude of their location for accurate weather data retrieval.
- **Configurability:** The code is highly customizable, allowing users to adapt it to different WiFi networks and display specifications. For instance, adjustments can be made for displays with dimensions other than the default 128x32 pixels.
- **Real-time Data:** The system continuously fetches weather data from the [OpenWeather API](https://openweathermap.org/) at intervals of 5 seconds, ensuring that the displayed information remains up-to-date.
- **User-friendly Interface:** Weather information is presented in a visually appealing and easy-to-understand format, enhancing user experience and accessibility.
##

## Usage

To utilize this weather data displaying system, proceed as follows:

1. **Setup Hardware:** Begin by setting up your ESP-32 and OLED Display. If you're using a display with different specifications, ensure to customize the code accordingly by modifying parameters such as `SCREEN_WIDTH` and `SCREEN_HEIGHT`.
   
   ```cpp
   #define SCREEN_WIDTH 128 // OLED display width, in pixels
   #define SCREEN_HEIGHT 32 // OLED display height, in pixels
2. **Clone Repository and Upload Code**: Clone the repository and upload the provided code to your ESP board.
3. **Configure Credentials**: Connect to the "AccesspointAP" network to configure necessary API and WiFi credentials. This initial setup is facilitated by the WiFi Manager Library.
4. **Automatic Data Fetching**: Once configured, the system will automatically retrieve weather data based on the user's location and display it on the OLED screen.
5. **Configuration Persistence**: Configuration settings are automatically saved after initial setup, eliminating the need for repeated configuration in subsequent uses.

## Dependencies

This weather data displaying system relies on the following dependencies:

-  <a href="https://openweathermap.org/">Open-weather API </a>: Provides access to real-time weather data.
-  <a href="https://github.com/tzapu/WiFiManager">Wifi Manager Library</a>: Facilitates initial configuration by managing WiFi credentials.

## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT), offering users the freedom to use, modify, and distribute the software as per the terms of the license agreement.
