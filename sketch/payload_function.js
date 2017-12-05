function Decoder(bytes, port) {

  var decoded = {};

  bssid = shiftBytes(bytes);
  decoded.Bssid1 = bssid.join('');

  bssid = shiftBytes(bytes);
  decoded.Bssid2 = bssid.join('');

  bssid = shiftBytes(bytes);
  decoded.Bssid3 = bssid.join('');

  return decoded;
}

function pad(n) {
  return (n < 10 || n == 'a' || n == 'b' || n == 'c' || n == 'd' || n == 'e' || n == 'f') ? ("0" + n) : n;
}

function shiftBytes(byteArray) {
  var i, bit;
  var tempArray = [6];
  for (i = 0; i < 6; i++) {
    bit = byteArray.shift();
    bit = bit.toString(16);
    if (i != 5) {
      tempArray[i] = pad(bit) + ":";
    }
    else {
      tempArray[i] = pad(bit);
    }
  }
  return tempArray;
}
