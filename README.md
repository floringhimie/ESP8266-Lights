# ESP8266 lights

ESP8266 HTTP lights (lights_esp.ino)

 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/light/0 will set the GPIO2 low,
 *    http://server_ip/light/1 will set the GPIO2 high
 *    http://server_ip/status will show GPIO2 state (usefull for Homekit)
 *  if you want DHCP IP => comment // WiFi.config(ip, gateway, subnet);
 *  server_ip is the IP address of the ESP8266 module, will be printed to Serial when the module is connected.
 *  Automaticaly save the last relay state in config.json to use it if ESP8266 lose power and set the last state to relay.
 *  You can use with Homebrdidge(tested), any other app that can make HTTP request GET or in browser.
 *  Successfuly tested with Kodi xbmc.callbacks2 plugin to turn light OFF when Play and turn ON when Pause or Stop.
