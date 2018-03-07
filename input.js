var num = 1 + 2;
var str = "hoge";
var buf = new Uint8Array(5);

print(num);
print(str);
print(buf);

var h = new Hoge();
print(h);

h.num = 11;
print(h.num);
h.str = "hoge";
var buf = h.buf;
for (var i = 0; i < buf.length; i++) {
  buf[i] = i;
}
print(h.buf);
h.dump();
h = null;

var h2 = new Hoge();
h2.dump();
h2 = null;

var arr = new Array();
var count = 0;
while (true) {
  if (count % 100000 == 0) {
    arr.push(new Uint8Array(1920 * 1080 * 4));
  }
  count++;
}

