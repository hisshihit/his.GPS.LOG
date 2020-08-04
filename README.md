# his.gps.log
a sketch to save logs for M5Stack GPS Unit
and a sketch to upload logs w/ http POST using JSON

## 今回のハード構成
<p align="center">
	<img src="./20200718_233842.jpg" width="75%" />
</p>
<ul>
  <li><a href="https://www.switch-science.com/catalog/3647/">M5Stack Basic</a></li>
  <li><a href="https://www.switch-science.com/catalog/5694/">M5Stack用GPSユニット</a></li>
  <li><a href="https://www.switch-science.com/catalog/6344/">M5Stack用環境センサユニット ver.2（ENV II）</a></li>
  <li><a href="https://www.switch-science.com/catalog/3653/">M5Stack用電池モジュール</a></li>
</ul>

## 各ユニットの接続
<p align="center">
  <img src="./M5Stack-cabling.jpg" width="75%" />  
</p>

## 主な機能
<ul>
  <li><a href="./his.GPS.LOG.SAVE.ino">his.GPS.LOG.SAVE.ino</a>
    M5Stackの内蔵BLEを使って、厚労省の接触確認アプリCocoaを検出するもの。
    BLE全体で検出した台数と内数としてのCoCoa台数を表示する。
    検出した情報は、microSDにログファイルとして記録する。
    M5Stack Basicに温度湿度気圧センサであるENV.2 UnitをGROVE経由で接続し、
    それらの情報もついでに表示・記録する。
    GPS Unitを接続し、検出した時の位置情報と日時を記録する。
  </li>
  <li><a href="./his.GPS.LOG.POST.ino">his.GPS.LOG.POST.ino</a>
    his.GPS.LOG.SAVE.inoが出力したmicroSD内のログファイルをJSON形式で
    http POSTする。
    WiFiを使用する。
  </li>
</ul>

## 詳細情報
<ul>
  <li><a href="./his.GPS.LOG.SAVE.md">his.GPS.LOG.SAVE.md</a></li>
  <li><a href="./his.GPS.LOG.POST.md">his.GPS.LOG.POST.md</a></li>
</ul>
