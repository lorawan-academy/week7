const ttn = require("ttn");
const app = require("express")();
const http = require("http").Server(app);
const bodyParser = require("body-parser");

const googleMapsClient = require("@google/maps").createClient({
  key: "xxxxxxxxxxxxxxxxxxxxxxxx"
});

var appID = "xxxxxxxxxx";
var accessKey = "ttn-account-v2.xxxxxxxxxxxxxxxxxxxxxxxx";

var macAddress1 = "";
var macAddress2 = "";
var macAddress3 = "";

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
      wifiAccessPoints: [
        {
          macAddress: macAddress1
        },
        {
          macAddress: macAddress2
        },
        {
          macAddress: macAddress3
        }
      ]
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
      if (payload.payload_fields.bssid3 && setAddresses == false) {
        console.log("Received uplink from ", devID);
        console.log(payload.payload_fields);
        macAddress1 = payload.payload_fields.bssid1;
        macAddress2 = payload.payload_fields.bssid2;
        macAddress3 = payload.payload_fields.bssid3;
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
