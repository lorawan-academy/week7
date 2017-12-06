function pad(b) {
  var h = b.toString(16);
  return (h + "").length < 2 ? "0" + h : h;
}

function Decoder(bytes, port) {
  var result = {};
  for (var i = 0; i < bytes.length; i += 6) {
    result["bssid" + (i / 6 + 1)] = bytes.slice(i, i + 6).map(pad).join(":");
  }
  return result;
}
