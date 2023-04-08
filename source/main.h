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

ATS_HANDLES g_output; // 出力