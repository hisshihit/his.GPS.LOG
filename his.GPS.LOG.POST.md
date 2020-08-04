# his.gps.log
a sketch to save logs for M5Stack GPS Unit
and a sketch to upload logs w/ http POST using JSON

## 主な機能
    M5Stack COCOA Counter + ENV.2 Sensor + GPS Unit その４で出力した
    M5StackのマイクロSDカード内のログを、M5StackからWiFi経由でhttp POSTするもの。
    POST先は、node.js+mongoDBで動いているものなどを想定している。


## 付加機能
    ボタンA(左)を押すと、M5Stack-SD-Updaterに遷移する。
    読み出すログファイル名、接続するWiFiのSSID/pass、http POSTするURLを
    マイクロSD内の"/GPSlog.conf"という名の初期設定ファイルから読み出すようにした。
    （使用環境に合わせるためのスケッチの修正やリコンパイルを不要にした。）

## かっこ悪いところ
    ログファイル内のJSONのフォーマットチェックを一切おこなっていない。
    http POSTが失敗した時のエラー処理を一切おこなっていない。
    何レコードをPOSTしたのかもカウントしていない。


## 初期設定ファイル例
サンプルは<a href="./GPSlog.cnf">GPSlog.cnf</a>
<pre>
    {
        logfile: "/GPSlog.jsn",
        buf: 2048,
        ssid: "secret-SSID-home2.4",
        pass: "secret-password-home",
        url: "http://192.168.0.112:3000/log"
    }
</pre>

## 初期設定ファイルの説明
    logfile: ログファイル名
    buf: １レコードのJSON文字列用のバッファ長
    ssid: 接続するWiFiのSSID
    pass: WiFiのパスワード
    url: httpでPOSTするURL　　　