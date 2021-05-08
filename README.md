# Tascher -Task Switcher-

マウスの移動とホイールの回転や、Migemoインクリメンタルサーチで絞り込んでウインドウを切り替えるソフトウェアです。  
Alt+Tabの連打や、タスクバーのクリックよりも、素早く、簡単にウインドウの切り替え操作が出来る点が特徴です。  
ウインドウやプロセスを操作する各種コマンドも実装しています。  
マウス派の方、キーボード派の方、どちらにもおすすめです。  
ウインドウのサムネイルを表示させることができます。  
UWPアプリに対応しています。  
ウェブブラウザのタブも切り替えることができます。  
マルチモニタにも少しだけ対応しています。
      
## Screenshot
![Tascherメイン](../..//wiki/Images/Tascher_ss.gif)  
      
## Download
**[Download Binary Package / Installer](http://frostmoon.sakura.ne.jp/Tascher/)**
      
## System requirements
Windows XP/Vista/7/8/8.1/10  
Win32API直打ち、かつCRT非依存ですので、VC++ランタイムも不要です。
      
## Installation
「**Tascher\*.zip**」を適当なフォルダに解凍して下さい。

### Migemo検索を使用する
1. 「**bregonig.dll**」を[K.Takata's Web Pageさま](http://k-takata.o.oo7.jp/)からダウンロードし、  
「Tascher.exe」と同じディレクトリに配置します。
1. 「C/Migemo」を[香り屋さま](http://www.kaoriya.net/)からダウンロードし、  
「**migemo.dll**」「**dict**」を「Tascher.exe」と同じディレクトリに配置します。
1. 「Tascher.exe」を起動し、タスクトレイアイコンを右クリック->設定からMigemoを有効にして下さい。  
検索方法「**部分一致**」、Migemo「**小文字入力で有効**」、「**候補のウインドウが1つなら確定**」がおすすめ設定です。
      
## Usage
### マウス派の方向け
1. マウスカーソルを画面右下へ移動させます。
1. ウインドウリストが表示されます。
1. マウスホイールを回転させて、リスト内にある目的のウインドウを選択します。
1. マウスカーソルをウインドウリストの外へ移動させます。
1. ウインドウが切り替わります。
    
### キーボード派の方向け
1. 「`Shift + Ctrl + Z`」キーを押下します。
1. 目的のウインドウのファイル名やタイトルに含まれる文字を入力します。(AND検索可)
1. ウインドウリストの絞込みが行われ、候補が一つになると自動的に確定します。
1. 絞り込み途中で「`↑` / `↓` / `Ctrl + P` / `Ctrl + N`」などで選択し、  
   「`Enter` / `Ctrl + M`」のいずれかで確定することもできます。
1. ウインドウが切り替わります。

右クリックや「`_`」キーを入力すると、コマンドメニューが表示されます。  
ウインドウの最大化/最小化や、  
![ウインドウ操作メニュー](../../wiki/Images/Tascher_ss_Menu.png)  
  
プロセス(子も含む)の優先度変更/強制終了等々を行うことができます。  
![子プロセス操作メニュー](../../wiki/Images/Tascher_ss_ChildProcessMenu.png)  
  
\* 上記ショートカットキーや機能は設定で変更することができます。  
\* その他情報は「**[Readme.txt](/Readme.txt)**」をご覧ください。  
      
### デフォルトのショートカットキー割り当て
|コマンド|文字列|ショートカットキー|
|:--|:--|:--|
|切り替え|switch|Enter<br>Ctrl+M<br>左クリック|
|切り替えてカーソルをウインドウ中央へ移動|centercursor|Space(インクリメンタルサーチ中はAND検索)|
|キャンセル|cancel|Esc<br>Ctrl+右クリック|
|操作リセット|reset|Ctrl+G|
|前のアイテムを選択|prev|Ctrl+P<br>↑<br>マウスホイール上|
|次のアイテムを選択|next|Ctrl+N<br>↓<br>マウスホイール下|
|先頭のアイテムを選択|first|Ctrl+A|
|末尾のアイテムを選択|last|Ctrl+E|
|設定|settings|Ctrl+S|
|終了|quit|Ctrl+Q|
|最小化|minimize|Ctrl+I|
|最大化|maximize|Ctrl+X|
|閉じる|close|Ctrl+W|
|大きさを元に戻す|restore|Ctrl+R|
|最全面表示する/しない|topmost|Ctrl+T|
|インクリメンタルサーチの文字列を1つ削る|bsstring|Backspace<br>Ctrl+H|
|カラム入れ替え|swap|←<br>→|
|先頭一致検索|forwardmatch|Ctrl+K|
|部分一致検索|partialmatch|Ctrl+L|
|あいまい検索|flexmatch|Ctrl+F|
|検索範囲切り替え|searchscope|Ctrl+O|
|候補が1つなら切り替えする/しない|uniquewindow|Ctrl+U|
|デスクトップを表示|desktop|Ctrl+D|
|コマンドメニュー|menu|_<br>右クリック|
|windowメニュー|windowmenu|Shift+右クリック|
|子プロセスを表示|childprocess|Ctrl+_|
      
## Special Thanks
* KoRoN氏「[C/Migemo](http://www.kaoriya.net/)」
* K.Takata氏「[bregonig.dll](http://k-takata.o.oo7.jp/)」
* cogma氏「[cltc](http://cogma.sakura.ne.jp/)」
* Takla氏「[窓替え](http://taklasoft.web.fc2.com/)」
* kvakulo氏「[Switcheroo](https://github.com/kvakulo/Switcheroo)」
* NTWind Software「[VistaSwitcher](https://www.ntwind.com/)」
* Microsoft「[PowerToys Run](https://github.com/microsoft/PowerToys)」
        
## License
NYSL Version 0.9982に準拠して配布されています。  
ライセンスの条文については同梱の「**[NYSL_withfaq.TXT](/NYSL_withfaq.TXT)**」をご覧下さい。
      
## Support
サポート(ご意見、ご感想、不具合のご報告、ご要望等)は  
<Y.R.Takanashi@gmail.com>  
若しくは  
BBS(以下URLよりアクセス可能)からお願いします。(Twitterも可)  
最新版バイナリは[Frost Moon Project](http://frostmoon.sakura.ne.jp/)から入手することができます。
