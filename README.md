This uses FastLED and ArtnetWifi to use an ESP8266 NodeMCU to connect to a WiFi near you (edit credentials) and be able to receive DMX over ArtNet. 
This was build of some example online, but I forgot where it was. I will add some credits, whenever I find it back!

There is some logic to bounce between SSIDs, so you can collaborate with people and also have it connect to either your closed off IoT WiFi network (please, just do not think about putting this into the internet. Please.) or your debugging network.

You need to connect the D6 pin of the ESP8266 to the data line of your LED Strip (WS2812 or whatever, you can also change it) and also connect the ground to all the other grounds. How you provide power to the ESP8266 is up to you :) I like step-downs and decent PSUs, but I also use this to have ArtNet Nodes for raves and light installations.

I sort of maintain this? It works and is fast and surprisingly reliable. You need a decent WiFi AP, when using a bunch of nodes. I have noticed that on cheap TP-Link stuff around 20 nodes is a breaking point. Some fixtures lag or flicker weird etc. Not bad, you are running almost 20 nodes. 
