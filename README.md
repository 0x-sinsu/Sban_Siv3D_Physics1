# Sban_Siv3D_Physics1  
## これ何？  
全てあなたの所為です。氏の「..」の映像にあるような物理演算をOpenSiv3Dで再現するやつです。  
趣味で作った上プログラミング知識がほぼない状態でAIなどに書いてもらったのでおかしなところがあるかもしれません。多分あります。  

ざっくり使い方解説動画(一部手順省略)  
https://youtu.be/yFjHf1IeN2Y
## 必要環境  
OS	Windows 11(10でも動くと思いますがテストしていないので保証できません)  
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
  
### OpenSiv3Dのインストール  
次に、[こちら](https://siv3d.github.io/ja-jp/)からOpenSiv3D(2024年2月6日現在v0.6.14)をダウンロード、実行します。  
詳細はリンク先に記載されていますので、そちらに従ってください。  
  
ここで必ず再起動を行います。 
  
### ソースコードのダウンロード、配置  
次に、[こちら](https://github.com/0x-sinsu/Sban_Siv3D_Physics1)のページの「Code」をクリック、「Download ZIP」をクリックしてダウンロードします。  
(gitでも問題なし)  
任意のフォルダに配置します。  
次に、lyrics.txtに上から降らせる歌詞を入力し、保存します。(文字エンコーディングはUTF-8)  
次に、right.txtに右から出てくる文字を、left.txtに左から出てくる文字を入力、保存します。(こちらもUTF-8)  
次にSbanPhysics1/edit.exeを実行する  
↓誤判定された場合のみで良いと思います  
(のですが、現在ウイルスと誤判定されてしまうようですので[こちら](https://support.microsoft.com/ja-jp/windows/windows-%E3%82%BB%E3%82%AD%E3%83%A5%E3%83%AA%E3%83%86%E3%82%A3%E3%81%AB%E3%82%88%E3%82%8B%E4%BF%9D%E8%AD%B7%E3%82%92%E5%88%A9%E7%94%A8%E3%81%97%E3%81%BE%E3%81%99-2ae0363d-0ada-c064-8b56-6a39afb6a963)のサイトの下の方に記載されている  
"リアルタイム保護を一時的に無効にする"  
の手順に従い無効化します。  
作業が終了したら必ず元に戻します。なお、他のセキュリティソフトに関しては私は分かりませんので各自お調べください。  
ウイルス入ってるんじゃ...と思った場合はedit.cppの中身をチェックしてください。  
それでも不安な場合はedit.cppを各自コンパイルしてください。  
コンパイル方法は割愛します。)  
edit.exeを開いたら、指示に従い数値を入力します。  
(標準値というのはあくまでも私がテストした時点でいい感じだった値ですので参考程度にしてください)  
この後のbuild.batを実行する際もリアルタイム保護の無効化が必要なため、まだ無効化のままにしておきます。(無効化した場合のみ)  
  
---以下は以前の編集方法です---  
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
しばらく待つとコマンドプロンプトが閉じますので、そうなったらビルド完了です。  
ビルドが終了すると、Intermediate\SbanPhysics1\Release内にSbanPhysics1.exeファイルが出来ています。  
(ここでリアルタイム保護を有効に戻します。)  

---以下は以前のビルド方法です---  
次にスタートメニューで「x64 Native Tools Command Prompt for VS 2022」と検索し、出てきたものを実行します。  
そこに以下のコマンドを貼り付け、実行します。  
```Batchfile
msbuild "先ほど控えたフォルダパス\SbanPhysics1.sln" /p:Configuration=Release /p:Platform=x64
```
大量の文が流れてきますが、そのまま入力待機状態になるまで待ちます。  
  
### このプログラムはフルスクリーンで実行されます。  
### 「Esc」キーを押すことでプログラムは終了します。  
追記:実行される画面のサイズ変更方法  
149行目付近の
```cpp:149.cpp
Window::SetFullscreen(true);
```
の部分を、1280*720にする時は
```cpp:720p.cpp
Window::Resize(1280, 720);
```
のように記述すると動くはずです。多分ね...  
  
vswhereを使用しています:https://github.com/microsoft/vswhere
