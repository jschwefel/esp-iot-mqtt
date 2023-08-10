# NOTE

This the my first ever pure C programming effort. I am still learning C and best practices of same. I appreciate any feedback, pull requests or otherwise that will help my learning and/or improving the code.

# ESP-IOT-MQTT

### esp-iot-mqtt is currently targetting ESP32-C6 as it is the only one have at the moment.

The esp-iot-mqtt project purpose is to create a configurable, via web interface, iot sensor/enpoint and actuator. This is acoomplished by abstracting as much of the ESP32/FreeRTOS in to configuration entities as possible. 

The control messages are send/received with MQTT. The MQTT messages will be configurable through the web interface to suit the needs of the end uses.

Interfacing with connected "devices" with be acomplished through "drivers". I am still working out the specifics of this.

## Usage

Clone the repo, build/flash using the esp-idf framwork tool or using VS Code.

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

## License

[MIT](https://choosealicense.com/licenses/mit/)
