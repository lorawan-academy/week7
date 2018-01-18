const ttn = require("ttn");
const app = require("express")();
const http = require("http").Server(app);
const bodyParser = require("body-parser");

const googleMapsClient = require("@google/maps").createClient({
  key: "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
});

var appID = "xxxxxxxxxxxxxxxxx";
var accessKey = "ttn-account-v2.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

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
      //iterate over each field discovered
      for (var field in payload.payload_fields) {
        if (payload.payload_fields.hasOwnProperty(field)) {
          var address = String(payload.payload_fields[field]);
          //appends the wifiAccessPointsAddresses tab
          wifiAccessPointsAddresses.push({
            macAddress: address
          });
        }
      }
    });
  })
  .catch(function(error) {
    console.error("Error", error);
    process.exit(1);
  });

http.listen(8080);
