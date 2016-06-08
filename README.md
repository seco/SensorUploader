# SensorUploader

esp8266 sketch to read sensor data (for now that's only BMP085 - pressure, temperature and DHT22 - temperature and humidity) and to push that over mqtt to a server.

My test server is mosquitto + mongodb + nginx + php (to display aggregated data) running on PI 2.

MQTT message can be single json containing all the sensors data or individual messages for each value  (configurable).

Serial port is used to configure parameters - wifi connection credentials, mqtt server name and credentials.

Display - sample script to display data

NodeJSMongoMQTT - sample nodejs script forwarding messages to mongodb collection, and able to forward messages to external web server.
