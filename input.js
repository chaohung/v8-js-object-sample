var num = 1 + 2;
var str = "hoge";
var buf = new Uint8Array(5);

print(num);
print(str);
print(buf);

var node = "global_object"
global_access();

callback((num) => { print("callback: ", num); });

print(native_object);

var uint8array = new Uint8Array(5);
uint8array.set([1, 2, 3, 4]);

var h = new Hoge(11, "hoge", uint8array);
h.dump();

h.num = 22;
print(h.num);
h.str = "hogehoge";
print(h.str);
var buf = h.buf;
for (var i = 0; i < buf.length; i++) {
  buf[i] = i+5;
}
print(h.buf);
h.dump();

var h2 = new Hoge(33, "hoge2", h.buf);
h2.dump();

h = null;
h2 = null;

var arr = new Array();
var count = 0;
while (true) {
  if (count % 100000 == 0) {
    arr.push(new Uint8Array(1920 * 1080 * 4));
  }
  count++;
}

