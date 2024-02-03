# Sban_Siv3D_Physics1  
## これ何？  
全てあなたの所為です。氏の「..」の映像にあるような物理演算をOpenSiv3Dで再現するやつです。  

## 必要環境  
OS	Windows 10 (64-bit) / Windows 11  
CPU	Intel または AMD 製の CPU  
映像出力	モニタなど、何らかの映像出力装置があること  
開発環境 Build Tools for Visual Studio 2022  
        (インストーラ内で「C++ によるデスクトップ開発」を追加インストールしてください)
        使い方分かる方はVisual Studioでslnを開いてください  

私の環境(Windows 11,Core i5-6500,内蔵グラフィック)では問題ありませんでした。ご参考までに。  


## 使い方  
### Build Tools for Visual Studio 2022のインストール  
まず以下のリンクからBuild Tools for Visual Studio 2022をダウンロードします。  
https://visualstudio.microsoft.com/ja/downloads/#build-tools-for-visual-studio-2022  
ダウンロードされたファイルを実行します。  
続行→C++によるデスクトップ開発にチェックを入れる→インストール  
分かりづらい場合は[こちら](https://www.kkaneko.jp/tools/win/buildtool2022.html)などが参考になると思います。  
  
### OpenSiv3Dのインストール  
次に、[こちら](https://siv3d.github.io/ja-jp/)からOpenSiv3D(2024年2月1日現在v0.6.13)をダウンロード、実行します。  
詳細はリンク先に記載されていますので、そちらに従ってください。  
  
ここで必ず再起動を行います。 
  
### ソースコードのダウンロード、配置  
次に、[こちら](https://github.com/0x-sinsu/Sban_Siv3D_Physics1)のページの「Code」をクリック、「Download ZIP」をクリックしてダウンロードします。  
(gitでも問題なし)  
任意のフォルダに配置します。  
SbanPhysics1.slnがあるフォルダのパスを控えておきます。  
次に、Main.cppをVSCodeなどで開きます。
編集する箇所をコメントアウトで示していますので、それを参考に編集してください。  
編集する部分の大まかな行は、
121行目
161行目
170行目
189行目
227行目
231行目
334行目
辺りです。  
分からない所があればTwitterのDMやDiscordまでどうぞ。  
Twitter:[@0x_sinsu](https://twitter.com/0x_sinsu)  
Discord:subete_light  
  
### ビルド
build.batを実行します。
しばらく待つと入力待機状態になりますので、そうなったらビルド完了です。
ビルドが終了すると、Intermediate\Physics1\Release内にSbanPhysics1.exeファイルが出来ています。 

---以下は以前のビルド方法です---
次にスタートメニューで「x64 Native Tools Command Prompt for VS 2022」と検索し、出てきたものを実行します。  
そこに以下のコマンドを貼り付け、実行します。  
```Batchfile
msbuild "先ほど控えたフォルダパス\SbanPhysics1.sln" /p:Configuration=Release /p:Platform=x64
```
大量の文が流れてきますが、そのまま入力待機状態になるまで待ちます。  
---以前のビルド方法も使用することが可能です---
  
### このプログラムはフルスクリーンで実行されます。  
### 「Esc」キーを押すことでプログラムは終了します。  
実行される画面のサイズ変更方法などはいずれ追記します。変更するのであれば意図する動作になるようにご自由にコードを書き換えてください。  
