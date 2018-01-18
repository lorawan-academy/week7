const ttn = require("ttn");
const app = require("express")();
const http = require("http").Server(app);
const bodyParser = require("body-parser");

const googleMapsClient = require("@google/maps").createClient({
  key: "xxxxxxxxxxxxxxxxxxxxxx"
});

var appID = "xxxxxxxxxxxxx";
var accessKey = "ttn-account-v2.xxxxxxxxxxxxxxxxxxxxxx";

var address = "";
var wifibssids = [];

var setAddresses = false;

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
      wifiAccessPoints: wiFiBssids
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
    //on uplink messages
    client.on("uplink", function(devID, payload) {
      //print the message in the console
      if (setAddresses == false) {
        console.log("Received uplink from ", devID);
        console.log(payload.payload_fields);
        //iterate over each bssid discovered
        for (var field in payload.payload_fields) {
          address = payload.payload_fields[field];
          //appends the bssid tab
          wiFiBssids.push({
            macAddress: address
          });
        }
        setAddresses = true;
      }
    });
  })
  //Error handling
  .catch(function(error) {
    console.error("Error", error);
    process.exit(1);
  });

http.listen(8080);
