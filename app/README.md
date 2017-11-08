# Wifi Localization Application

Here is a small tutorial on how to build your ow web application in Node.js using wifi localization through LoraWan.

## Building our Server

* Make sure you properly installed [Node.js](https://nodejs.org/en/) (you probably did this already while following the MQTT tutorial).

```
$ mkdir wifi_localization_app
$ cd wifi_localization_app
```

We just created a new directory to which we will save all the files for our small project.

Now open your favorite text editor (like Atom, Gedit, SublimeText etc.). Create a new blank file and save it as `server.js` in the directory `http_integration_app`. Add the line **console.log("Starting server...")** to the `server.js` file.


Going back to the command line and create an empty `package.json` by typing:

```
$ npm init -y
```

Now, let's donwload 3 packages we need for building our app:

```
$ npm install ttn --save
$ npm install http --save
$ npm install express --save
$ npm install body-parser --save
$ npm install @google/maps --save
```

Let's go back to the `server.js` file in your text editor. We will start with importing the libraries into our code:

```js
const ttn = require("ttn");
const app = require("express")();
const http = require("http").Server(app);
const bodyParser = require("body-parser");

const googleMapsClient = require('@google/maps').createClient({
  key: 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx' //replace with the API key you got from Google
});

var appID = "xxxxxxxxxxxx"
var accessKey = "ttn-account-v2.XXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

var macAddress1 = ""
var macAddress2 = ""
var macAddress3 = ""
```
You will need to ask for a Google Maps API key to use Google Maps api: https://developers.google.com/maps/documentation/geocoding/get-api-key?hl=fr#key

To start the server, we can add the lines:


```js
app.get("/", function(req, res){
	res.sendFile(__dirname + "/map.html")
})
http.listen(8000);
```

Our web server is now running on **port 8000**, you can access it by going to [http://localhost:8000](http://localhost:8000). Before there is something to show on this page, we need to create an `map.html` file. Open another blank file save it as `map.html` in the same directory as `server.js` and write down someting like this:
```html
<!DOCTYPE html>
<html>
  <head>
    <style>
      #map {
        width: 100%;
        height: 400px;
        background-color: grey;
      }
    </style>
  </head>
  <body>
    <h3>Geolocation integration</h3>
    <div id="map"></div>
    <script async defer
    src="https://maps.googleapis.com/maps/api/js?key=AIzaSyDvG9inIBgoXyFWZAhZqyzyJub_7VXsl8c&callback=initMap">
    </script>
    <script src="http://code.jquery.com/jquery-2.0.3.min.js"></script>
  </body>
</html>
```
You can see a division called map which will contain our embeded Google map. And then you can find the scripts we need to include to create later on, our map.

Let's launch the server from the command line and see what happens:

```
$ node server.js
```

Now go to localhost and you should see **Geolocation integration** printed in your web page.

Now let's start listening to our device and store the MAC Addresses we receive. We will need to add the following changes to our `server.js` file:
```js
const ttn = require("ttn");
const app = require("express")();
const http = require("http").Server(app);
const bodyParser = require("body-parser");

const googleMapsClient = require('@google/maps').createClient({
  key: 'XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'
});

var appID = "XXXXXXXXXXXXXX"
var accessKey = "ttn-account-v2.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

var macAddress1 = ""
var macAddress2 = ""
var macAddress3 = ""

app.use(bodyParser.json())
app.use(bodyParser.urlencoded({ extended: false }))

//this is the default home page of our application
app.get("/", function(req, res){
	res.sendFile(__dirname + "/map.html")
})
//opening mqtt connection with ttn
ttn.data(appID, accessKey)
.then(function(client) {
	//on uplink messages
	client.on("uplink", function(devID, payload) {
		//print the message in the console
		console.log("Received uplink from ", devID)
		console.log(payload.payload_fields)
    macAddress1 = payload.payload_fields.Bssid1
    macAddress2 = payload.payload_fields.Bssid2
    macAddress3 = payload.payload_fields.Bssid3
	})
})
//Error handling
.catch(function (error) {
	console.error("Error", error)
	process.exit(1)
})

http.listen(8080)
```
Stop your server with `CTRL` + `C` if it was still running, save the changes to the server file and start the server again.
As we wait for a bit, we should see the incomming messages from our board arrive with the 3 BSSID we are going to give to Google maps API so that I gives us back a location.
Let's use the API in the `server.js` file and add this before the ttn client part of our code:
```js
app.get("/generator", function(req, res){
  googleMapsClient.geolocate({
    "considerIp": false,
    "wifiAccessPoints": [
      {
        "macAddress": macAddress1
      },
      {
        "macAddress": macAddress2
      },
      {
        "macAddress": macAddress3
      }
    ]
  }, function(err, response) {
    console.log(response)
    console.log(response.json.location)
    return res.json(response.json.location)
  })
})
```
Then we will need to add a script in our html page to fetch the location that's given back by the Google API and generate the map.
```html
<!DOCTYPE html>
<html>
  <head>
    <style>
      #map {
        width: 100%;
        height: 100%;
        background-color: grey;
      }
    </style>
  </head>
  <body>
    <h3>Geolocation integration</h3>
    <div id="map"></div>
    <script>
      function initMap() {
        $.get("/generator", function(data){
          var uluru = {lat: parseFloat(JSON.stringify(data.lat)), lng: parseFloat(JSON.stringify(data.lng))};
          var map = new google.maps.Map(document.getElementById('map'), {
            zoom: 4,
            center: uluru
          });
          var marker = new google.maps.Marker({
            position: uluru,
            map: map
          });
        })
      }
    </script>
    <script async defer
    src="https://maps.googleapis.com/maps/api/js?key=AIzaSyDvG9inIBgoXyFWZAhZqyzyJub_7VXsl8c&callback=initMap">
    </script>
    <script src="http://code.jquery.com/jquery-2.0.3.min.js"></script>
  </body>
</html>
```
Stop the server and start it again. Let's go to http://localhost:8000. You should see a fancy map with the marker on your current location.

<div align="center">
<img src="../img/geolocation.png" width="350"/>
</div>
The messages that you receive in the terminal should also look like this:

<div align="center">
<img src="../img/msg_wifi_localization.png" width="350"/>
</div>
