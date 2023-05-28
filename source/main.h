//main.h
//このファイルでは汎用機能を定義します。

int g_emgBrake; //非常ブレーキ
int g_svcBrake; //常用最大ブレーキ
int g_brakeNotch; //ブレーキノッチ
int g_powerNotch; //力行ノッチ
int g_reverser; //レバーサー
int g_pilotLamp; //運転士知らせ灯
int g_time; //現在時刻[ms]
int g_speed; //速度計の速度[km/h]
int g_deltaT; //前フレームからのフレーム時間[ms/f]

//互換モード関連
int snp2Beacon;

//INIファイル関連
int PLampindex; //パイロットランプインデックス
int DispType; //運転情報画面のE/M
int EMeter; //メーター表示の有効化
int ETIMS; //TIMS表示の有効化
int EVmeter; //電圧計の有効化
int EUD; //ユニット表示他有効化
int D01ABnum; //表示する駅数
int Row6; //6段目の扱い
int LVindex; //架線電圧計
int HVindex; //架線電圧計
int BCMRType; //BC/MR圧計仕様
int LineUpdate; //1行更新の時間
int EbCut; //ブレーキ減圧時間
int Lbinit; //初回起動時力行遅延

TIMS_SierraIni ini;
ATS_HANDLES g_output; // 出力