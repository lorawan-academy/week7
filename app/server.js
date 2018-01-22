const ttn = require("ttn");
const app = require("express")();
const http = require("http").Server(app);
const bodyParser = require("body-parser");

const googleMapsClient = require("@google/maps").createClient({
  key: "xxxxxxxxxxxxxxxxxxx"
});

var appID = "xxxxxxxxxxxx";
var accessKey = "ttn-account-v2.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

var wifiAccessPointsAddresses = [];

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

//this is the default home page of our application
app.get("/", function(req, res) {
  res.sendFile(__dirname + "/map.html");
});

app.get("/generator", function(req, res) {
  googleMapsClient.geolocate(
    {
      considerIp: false,
      wifiAccessPoints: wifiAccessPointsAddresses
    },
    function(err, response) {
      if (err) {
        console.log(err);
      } else {
        console.log(response.json.location);
        return res.json(response.json.location);
      }
    }
  );
});

//opening mqtt connection with ttn
ttn
  .data(appID, accessKey)
  .then(function(client) {
    client.on("uplink", function(devID, payload) {
      wifiAccessPointsAddresses = [];
      console.log(payload.payload_fields);
      //iterate over the tab which contains the addresses
      var arrayLength = payload.payload_fields.access_points.length;
      for (var i = 0; i < arrayLength; i++) {
        var address = payload.payload_fields.access_points[i].bssid;
        var rssi = payload.payload_fields.access_points[i].rssi;
        wifiAccessPointsAddresses.push({
          macAddress: address,
          signalStrength: rssi
        });
      }
    });
  })
  .catch(function(error) {
    console.error("Error", error);
    process.exit(1);
  });

http.listen(8080);
