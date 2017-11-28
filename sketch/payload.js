function Decoder(bytes, port) {
  var Bssid1 = [6];
  var Bssid2 = [6];
  var Bssid3 = [6];
  var i, j;
  var bit;

  var decoded = {};
  for (i = 0, j = 0; i < 6 && j < 6; i++ , j++) {
    bit = bytes[j].toString(16);
    if (i != 5) {
      Bssid1[i] = pad(bit) + ":";
    }
    else {
      Bssid1[i] = pad(bit);
    }
  }
  for (i = 0, j = 6; i < 6 && j < 12; i++ , j++) {
    bit = bytes[j].toString(16);
    if (j != 11) {
      Bssid2[i] = pad(bit) + ":";
    }
    else {
      Bssid2[i] = pad(bit);
    }
  }
  for (i = 0, j = 12; i < 6 && j < 18; i++ , j++) {
    bit = bytes[j].toString(16);
    if (j != 17) {
      Bssid3[i] = pad(bit) + ":";
    }
    else {
      Bssid3[i] = pad(bit);
    }
  }

  decoded.Bssid1 = Bssid1.join('');
  decoded.Bssid2 = Bssid2.join('');
  decoded.Bssid3 = Bssid3.join('');

  return decoded;
}


function pad(n) {
  return (n < 10 || n == 'a' || n == 'b' || n == 'c' || n == 'd' || n == 'e' || n == 'f') ? ("0" + n) : n;
}
