# Gas-Temperature-Humidity-and-Pressure-Data-Logger
This is a datalogger that I made using the ATSAML21J18B(ARM Cortex M0+ microcontroller from ATMEL/MICROCHIP) that outputs a temperature in °C, gas resistance in ohms, pressure in hPa and humidity in % relative humidity. I used the BME680 sensor from BOSCH to implement this functionality.  
  
In this project, I am using the EADOGM163W-A screen and Tera Term (computer terminal program) as the output for data for the user.  
I created my own driver for the screen's controller (the ST7036) using the datasheet with SPI as the main communication protocol.  
From this project, I was also able to learn how to use APIs given by manufacturers (in this case, BOSCH) to use the sensor and get the proper information I needed.
