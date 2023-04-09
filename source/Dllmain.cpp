// Ats.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
// (c) Line-16 2023
// Umicorn様、kikuike様のソースコードを参考にさせていただいています。

#include "stdafx.h"
#include <stdlib.h>
#include "atsplugin.h"
#include "main.h"
#include "tims.h"
#include "Meter.h"
#include "dead.h"
#include "spp.h"
#include "sub.h"

TIMS g_tims; //TIMS表示器
Meter g_meter; //メーター表示器
SPP g_spp; //誤通過防止装置
Sub g_sub; //その他
DEAD g_dead;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

ATS_API void WINAPI Load(void) 
{
	g_tims.Load();
	g_meter.Load();
	g_sub.load();
	g_dead.load();
}

ATS_API int WINAPI GetPluginVersion(void)
{
	return ATS_VERSION;
}

ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC vehicleSpec)
{
	g_svcBrake = vehicleSpec.BrakeNotches;
	g_emgBrake = g_svcBrake + 1;

	g_spp.ServiceNotch = vehicleSpec.AtsNotch;
}

ATS_API void WINAPI Initialize(int brake)
{
	g_speed = 0;
	g_spp.Initialize();
}

ATS_API ATS_HANDLES WINAPI Elapse(ATS_VEHICLESTATE vehicleState, int *panel, int *sound)
{
	g_deltaT = vehicleState.Time - g_time;
	g_time = vehicleState.Time;
	g_speed = vehicleState.Speed;

	g_tims.Location = vehicleState.Location;
	g_tims.BcPressure = vehicleState.BcPressure;
	g_tims.MrPressure = vehicleState.MrPressure;
	g_tims.Current = vehicleState.Current;
	g_meter.Location = vehicleState.Location;
	g_meter.BcPressure = vehicleState.BcPressure;
	g_meter.MrPressure = vehicleState.MrPressure;
	g_meter.Current = vehicleState.Current;
	g_sub.BcPressure = vehicleState.BcPressure;

	g_tims.Execute(); //TIMS表示器
	g_meter.Execute(); //メーター表示器
	g_dead.execute(); //電圧関連
	g_spp.Execute(); //誤通過防止装置
	g_sub.Execute(); //他機能

	// ハンドル出力
	if(g_sub.BcPressCut == 1)
		g_output.Brake = 1; //EB緩解時にブレーキを減圧する
	else
		g_output.Brake = g_brakeNotch;

	g_output.Reverser = g_reverser;

	if (g_time > g_sub.AccelCutting && g_dead.VCB_ON == 1)
		g_output.Power = g_powerNotch;
	else
		g_output.Power = 0;

	g_output.ConstantSpeed = ATS_CONSTANTSPEED_CONTINUE;

	// パネル出力
	//メーター表示器
	// 電流計
	panel[21] = g_meter.AMMeterD[4]; //電流計[符号]
	panel[22] = g_meter.AMMeterD[0]; //電流計[1000位]
	panel[23] = g_meter.AMMeterD[1]; //電流計[100位]
	panel[24] = g_meter.AMMeterD[2]; //電流計[10位]
	panel[25] = g_meter.AMMeterD[3]; //電流計[1位]
	panel[26] = g_meter.AMMeterA[1]; //電流計[指針]
	panel[27] = g_meter.AMMeter[0]; //電流グラフ（+）
	panel[28] = g_meter.AMMeter[1]; //電流グラフ（-）
	//ブレーキシリンダ・元空気ダメ
	panel[122] = g_meter.BcCaution ? ((g_time % 1000) / 500) : 0; //200kPa警告
	panel[123] = g_meter.BCMeter[0]; //BCグラフ(0～180kPa）
	panel[124] = g_meter.BCMeter[1]; //BCグラフ(200～380kPa）
	panel[125] = g_meter.BCMeter[2]; //BCグラフ(400～580kPa）
	panel[126] = g_meter.BCMeter[3]; //BCグラフ(600～780kPa）
	panel[127] = g_meter.MRMeter[0]; //MRグラフ(750～795kPa）
	panel[128] = g_meter.MRMeter[1]; //MRグラフ(800～845kPa）
	panel[129] = g_meter.MRMeter[2]; //MRグラフ(850～895kPa）
	// ブレーキシリンダ・元空気ダメ（デジタル表示）
	/*
	panel[122] = g_meter.BCPressD[0]; //ブレーキシリンダ[100位]
	panel[123] = g_meter.BCPressD[1]; //ブレーキシリンダ[10位]
	panel[124] = g_meter.BCPressD[2]; //ブレーキシリンダ[1位]
	panel[125] = g_meter.BCPress; //ブレーキシリンダ[指針]
	panel[122] = g_meter.MRPressD[0]; //元空気ダメ[100位]
	panel[123] = g_meter.MRPressD[1]; //元空気ダメ[10位]
	panel[124] = g_meter.MRPressD[2]; //元空気ダメ[1位]
	panel[125] = g_meter.MRPress; //元空気ダメ[指針]
	*/
	// 速度計
	panel[45] = g_meter.SpeedD[0]; //速度計[100位]
	panel[46] = g_meter.SpeedD[1]; //速度計[10位]
	panel[47] = g_meter.SpeedD[2]; //速度計[1位]
	panel[50] = g_meter.Speed; //速度計[指針]
	// ブレーキ指令計
	panel[32] = g_meter.AccelDelay; //力行指令
	panel[30] = g_meter.BrakeDelay; //ブレーキ指令
	panel[31] = g_meter.BrakeDelay == g_emgBrake ? 1 : 0; //非常ブレーキ
	//電圧類
	panel[217] = g_dead.AC; //交流
	panel[218] = g_dead.DC; //直流
	panel[219] = g_dead.CVacc; //制御電圧異常
	panel[220] = g_dead.CVacc10; //制御電圧[10位]
	panel[221] = g_dead.CVacc1; //制御電圧[1位]
	panel[222] = g_dead.ACacc; //交流電圧異常
	panel[223] = g_dead.ACacc10000; //交流電圧[10000位]
	panel[224] = g_dead.ACacc1000; //交流電圧[1000位]
	panel[225] = g_dead.ACacc100; //交流電圧[100位]
	panel[226] = g_dead.DCacc; //直流電圧異常
	panel[227] = g_dead.DCacc1000; //直流電圧[1000位]
	panel[228] = g_dead.DCacc100; //直流電圧[100位]
	panel[229] = g_dead.DCacc10; //直流電圧[10位]
	panel[230] = g_dead.Cvmeter; //制御指針
	panel[231] = g_dead.Acmeter; //交流指針
	panel[232] = g_dead.Dcmeter; //直流指針
	panel[233] = g_dead.Accident; //事故
	panel[234] = g_dead.Tp; //三相
	panel[236] = g_dead.VCB; //VCB
	panel[235] = g_dead.alert_ACDC > 0 ? g_dead.alert_ACDC + ((g_time % 800) / 400) : 0; //交直切換

	//TIMS上部表示
	// 時計
	panel[37] = (g_time / 3600000) % 24; //デジタル時
	panel[38] = g_time / 60000 % 60; //デジタル分
	panel[39] = g_time / 1000 % 60; //デジタル秒
	// TIMS速度計
	panel[100] = g_tims.TimsSpeed[0]; //TIMS速度計[100位]
	panel[101] = g_tims.TimsSpeed[1]; //TIMS速度計[10位]
	panel[102] = g_tims.TimsSpeed[2]; //TIMS速度計[1位]
	// 走行距離
	panel[103] = g_tims.Distance[0]; //走行距離[km]
	panel[104] = g_tims.Distance[1]; //走行距離[100m]
	//panel[105] = g_tims.Distance[2]; //走行距離[10m]
	// 矢印
	panel[109] = g_tims.ArrowDirection; //進行方向矢印
	// ユニット表示灯
	panel[41] = g_tims.UnitState[0]; //ユニット表示灯1
	panel[42] = g_tims.UnitState[1]; //ユニット表示灯2
	panel[43] = g_tims.UnitState[2]; //ユニット表示灯3
	panel[44] = g_tims.UnitState[3]; //ユニット表示灯4
	panel[105] = g_tims.UnitTims[0]; //TIMSユニット表示1
	panel[106] = g_tims.UnitTims[1]; //TIMSユニット表示2
	panel[107] = g_tims.UnitTims[2]; //TIMSユニット表示3
	panel[108] = g_tims.UnitTims[3]; //TIMSユニット表示4

	//TIMS全般表示
	panel[110] = g_tims.TrainArrow; //行路表矢印
	panel[111] = g_tims.Kind; //列車種別
	panel[112] = g_tims.Number[0]; //列車番号[1000位]
	panel[113] = g_tims.Number[1]; //列車番号[100位]
	panel[114] = g_tims.Number[2]; //列車番号[10位]
	panel[115] = g_tims.Number[3]; //列車番号[1位]
	panel[116] = g_tims.Charactor; //列車番号[記号]

	panel[117] = g_tims.Number[3] != 0 ? 1 : 0; //設定完了
	panel[118] = g_tims.PassMode; //通過設定
	panel[119] = g_tims.NextBlinkLamp; //次駅停車表示灯
	panel[120] = g_tims.From; //運行パターン始発
	panel[121] = g_tims.Destination; //運行パターン行先
	panel[208] = g_tims.For; //列車行先

	// スタフテーブル
	//電列共通
	panel[130] = g_tims.HiddenLine[0] ? 0 : g_tims.Station[0]; //駅名表示1
	panel[131] = g_tims.HiddenLine[1] ? 0 : g_tims.Station[1]; //駅名表示2
	panel[132] = g_tims.HiddenLine[2] ? 0 : g_tims.Station[2]; //駅名表示3
	panel[133] = g_tims.HiddenLine[3] ? 0 : g_tims.Station[3]; //駅名表示4
	panel[134] = g_tims.HiddenLine[4] ? 0 : g_tims.Station[4]; //駅名表示5

	panel[135] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][0]; //到着時刻1H
	panel[136] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][1]; //到着時刻1M
	panel[137] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][2]; //到着時刻1S
	panel[138] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][0]; //到着時刻2H
	panel[139] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][1]; //到着時刻2M
	panel[140] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][2]; //到着時刻2S
	panel[141] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][0]; //到着時刻3H
	panel[142] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][1]; //到着時刻3M
	panel[143] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][2]; //到着時刻3S
	panel[144] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][0]; //到着時刻4H
	panel[145] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][1]; //到着時刻4M
	panel[146] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][2]; //到着時刻4S
	panel[147] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][0]; //到着時刻5H
	panel[148] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][1]; //到着時刻5M
	panel[149] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][2]; //到着時刻5S

	panel[150] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][0]; //発車時刻1H
	panel[151] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][1]; //発車時刻1M
	panel[152] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][2]; //発車時刻1S
	panel[153] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][0]; //発車時刻2H
	panel[154] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][1]; //発車時刻2M
	panel[155] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][2]; //発車時刻2S
	panel[156] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][0]; //発車時刻3H
	panel[157] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][1]; //発車時刻3M
	panel[158] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][2]; //発車時刻3S
	panel[159] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][0]; //発車時刻4H
	panel[160] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][1]; //発車時刻4M
	panel[161] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][2]; //発車時刻4S
	panel[162] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][0]; //発車時刻5H
	panel[163] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][1]; //発車時刻5M
	panel[164] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][2]; //発車時刻5S

	panel[165] = g_tims.HiddenLine[0] ? 0 : g_tims.Track[0]; //次駅番線1
	panel[166] = g_tims.HiddenLine[1] ? 0 : g_tims.Track[1]; //次駅番線2
	panel[167] = g_tims.HiddenLine[2] ? 0 : g_tims.Track[2]; //次駅番線3
	panel[168] = g_tims.HiddenLine[3] ? 0 : g_tims.Track[3]; //次駅番線4
	panel[169] = g_tims.HiddenLine[4] ? 0 : g_tims.Track[4]; //次駅番線5
	/*
	//列車スタフ
	panel[170] = g_tims.HiddenLine[5] ? 0 : g_tims.Station[5]; //駅名表示6
	panel[171] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][0]; //到着時刻6H
	panel[172] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][1]; //到着時刻6M
	panel[173] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][2]; //到着時刻6S
	panel[174] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][0]; //発車時刻6H
	panel[175] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][1]; //発車時刻6M
	panel[176] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][2]; //発車時刻6S
	panel[177] = g_tims.HiddenLine[5] ? 0 : g_tims.Track[5]; //次駅番線6

	panel[178] = g_tims.HiddenLine[0] ? 0 : g_tims.Span[0][0]; //駅間走行時間1M
	panel[179] = g_tims.HiddenLine[0] ? 0 : g_tims.Span[0][1]; //駅間走行時間1S
	panel[180] = g_tims.HiddenLine[1] ? 0 : g_tims.Span[1][0]; //駅間走行時間2M
	panel[181] = g_tims.HiddenLine[1] ? 0 : g_tims.Span[1][1]; //駅間走行時間2S
	panel[182] = g_tims.HiddenLine[2] ? 0 : g_tims.Span[2][0]; //駅間走行時間3M
	panel[183] = g_tims.HiddenLine[2] ? 0 : g_tims.Span[2][1]; //駅間走行時間3S
	panel[184] = g_tims.HiddenLine[3] ? 0 : g_tims.Span[3][0]; //駅間走行時間4M
	panel[185] = g_tims.HiddenLine[3] ? 0 : g_tims.Span[3][1]; //駅間走行時間4S
	panel[186] = g_tims.HiddenLine[4] ? 0 : g_tims.Span[4][0]; //駅間走行時間5M
	panel[187] = g_tims.HiddenLine[4] ? 0 : g_tims.Span[4][1]; //駅間走行時間5S

	panel[188] = g_tims.HiddenLine[0] ? 0 : g_tims.LimitA[0]; //制限速度1IN
	panel[189] = g_tims.HiddenLine[0] ? 0 : g_tims.LimitL[0]; //制限速度1OUT
	panel[190] = g_tims.HiddenLine[1] ? 0 : g_tims.LimitA[1]; //制限速度2IN
	panel[191] = g_tims.HiddenLine[1] ? 0 : g_tims.LimitL[1]; //制限速度2OUT
	panel[192] = g_tims.HiddenLine[2] ? 0 : g_tims.LimitA[2]; //制限速度3IN
	panel[193] = g_tims.HiddenLine[2] ? 0 : g_tims.LimitL[2]; //制限速度3OUT
	panel[194] = g_tims.HiddenLine[3] ? 0 : g_tims.LimitA[3]; //制限速度4IN
	panel[195] = g_tims.HiddenLine[3] ? 0 : g_tims.LimitL[3]; //制限速度4OUT
	panel[196] = g_tims.HiddenLine[4] ? 0 : g_tims.LimitA[4]; //制限速度5IN
	panel[197] = g_tims.HiddenLine[4] ? 0 : g_tims.LimitL[4]; //制限速度5OUT
	panel[198] = g_tims.HiddenLine[5] ? 0 : g_tims.LimitA[5]; //制限速度6IN
	panel[199] = g_tims.HiddenLine[5] ? 0 : g_tims.LimitL[5]; //制限速度6OUT
	*/
	//電車スタフ
	panel[170] = g_tims.After; //次採時駅
	panel[171] = g_tims.AfterTimeA[0]; //次採時駅着時刻H
	panel[172] = g_tims.AfterTimeA[1]; //次採時駅着時刻M
	panel[173] = g_tims.AfterTimeA[2]; //次採時駅着時刻S
	panel[174] = g_tims.AfterTimeL[0]; //次採時駅発時刻H
	panel[175] = g_tims.AfterTimeL[1]; //次採時駅発時刻M
	panel[176] = g_tims.AfterTimeL[2]; //次採時駅発時刻S
	panel[177] = g_tims.AfterTrack; //次採時駅番線

	panel[178] = g_tims.Before; //直前採時駅
	panel[179] = g_tims.BeforeTime[0]; //直前採時駅着時刻H
	panel[180] = g_tims.BeforeTime[1]; //直前採時駅着時刻M
	panel[181] = g_tims.BeforeTime[2]; //直前採時駅着時刻S
	panel[182] = g_tims.BeforeTrack; //直前採時駅番線

	panel[183] = g_tims.Last; //降車駅
	panel[184] = g_tims.LastTimeA[0]; //降車駅着時刻H
	panel[185] = g_tims.LastTimeA[1]; //降車駅着時刻M
	panel[186] = g_tims.LastTimeA[2]; //降車駅着時刻S
	panel[187] = g_tims.LastTimeL[0]; //降車駅発時刻H
	panel[188] = g_tims.LastTimeL[1]; //降車駅発時刻M
	panel[189] = g_tims.LastTimeL[2]; //降車駅発時刻S
	panel[190] = g_tims.LastTrack; //降車駅番線

	panel[191] = g_tims.AfterKind; //列車種別
	panel[192] = g_tims.AfterNumber[0]; //列車番号[1000位]
	panel[193] = g_tims.AfterNumber[1]; //列車番号[100位]
	panel[194] = g_tims.AfterNumber[2]; //列車番号[10位]
	panel[195] = g_tims.AfterNumber[3]; //列車番号[1位]
	panel[196] = g_tims.AfterChara; //列車番号[記号]

	panel[197] = g_tims.AfterTime[0][0]; //次行路着時刻H
	panel[198] = g_tims.AfterTime[0][1]; //次行路着時刻M
	panel[199] = g_tims.AfterTime[0][2]; //次行路着時刻S
	panel[200] = g_tims.AfterTime[1][0]; //次行路発時刻H
	panel[201] = g_tims.AfterTime[1][1]; //次行路発時刻M
	panel[202] = g_tims.AfterTime[1][2]; //次行路発時刻S


	//次駅表示
	panel[203] = g_tims.HiddenLine[3] ? 0 : (g_tims.Next * g_tims.NextBlink); //駅名表示（次駅、点滅する）
	panel[204] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[0]; //次駅着時刻H
	panel[205] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[1]; //次駅着時刻M
	panel[206] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[2]; //次駅着時刻S
	panel[207] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTrack; //次駅着番線

	// サウンド出力
	sound[100] = g_sub.AirApply; //ブレーキ昇圧音
	sound[101] = g_sub.AirApplyEmg; //ブレーキ昇圧音（非常）
	sound[103] = g_sub.AirHigh; //非常ブレーキ緩解音
	sound[105] = g_sub.EmgAnnounce; //非常ブレーキ放送
	sound[106] = g_sub.UpdateInfo; //運行情報更新

	sound[14] = g_spp.HaltChime3; //停車チャイムループ（減少しない）
	sound[15] = g_spp.HaltChime; //停車チャイム
	sound[16] = g_spp.PassAlarm; //通過チャイム
	sound[17] = g_spp.HaltChime2; //停車チャイムループ

    return g_output;
}

ATS_API void WINAPI SetPower(int notch)
{
	g_powerNotch = notch;
}

ATS_API void WINAPI SetBrake(int notch)
{
	g_sub.PlaySoundAirHigh(g_brakeNotch, notch);
	g_sub.PlaySoundEmgAnnounce(g_brakeNotch, notch);
	g_sub.PlaySoundAirApplyEmg(g_brakeNotch, notch);
	g_spp.NotchChanged();
	g_brakeNotch = notch;
}

ATS_API void WINAPI SetReverser(int pos)
{
	g_reverser = pos;
	g_sub.SetLbInit(pos, g_sub.event_lbInit);
}

ATS_API void WINAPI KeyDown(int atsKeyCode)
{
}

ATS_API void WINAPI KeyUp(int hornType)
{
}

ATS_API void WINAPI HornBlow(int atsHornBlowIndex)
{
}

ATS_API void WINAPI DoorOpen()
{
	g_pilotLamp = false;
	g_tims.DoorOpen();
	g_spp.StopChime();
}

ATS_API void WINAPI DoorClose()
{
	g_pilotLamp = true;
	g_tims.DoorClose();
}

ATS_API void WINAPI SetSignal(int signal)
{
}

ATS_API void WINAPI SetBeaconData(ATS_BEACONDATA beaconData)
{
	switch (beaconData.Type)
	{
	case 30:
		if (g_speed != 0)//駅ジャンプを除外する
			g_spp.Recieve(beaconData.Optional % 100000);
		break;
	case 100://列番
		g_tims.SetNumber(beaconData.Optional / 100, beaconData.Optional % 100);
		break;
	case 101://種別
		g_tims.SetKind(beaconData.Optional);
		break;
	case 102: //進行方向
		g_tims.SetDirection(beaconData.Optional);
		break;
	case 103: //走行距離
		g_tims.SetDistance(beaconData.Distance, beaconData.Optional);
		break;
	case 104://運行パターン
		g_tims.SetLeg(beaconData.Optional);
		break;
	case 105://次駅接近
		if (g_speed != 0)//駅ジャンプを除外する
			g_spp.Recieve(beaconData.Optional % 10000);
		g_tims.Recieve(beaconData.Optional % 10000000, beaconData.Optional / 10000000); //駅ジャンプを除外しない
		break;
	case 106:
		g_tims.SetNext(beaconData.Optional);
		break;
	case 107:
		g_tims.SetNextTime(beaconData.Optional);
		break;
	case 108:
		g_tims.SetNextTrack(beaconData.Optional);
		break;
	case 109:
		g_tims.SetFor(beaconData.Optional);
		break;
	case 110:
		g_tims.InputLine(1, (beaconData.Optional / 1000) - 1, beaconData.Optional % 1000);
		break;
	case 111:
		g_tims.InputLine(2, (beaconData.Optional / 1000000) - 1, beaconData.Optional % 1000000);
		break;
	case 112:
		g_tims.InputLine(3, (beaconData.Optional / 1000000) - 1, beaconData.Optional % 1000000);
		break;
	case 113:
		g_tims.InputLine(4, (beaconData.Optional / 100) - 1, beaconData.Optional % 100);
		break;
	case 114:
		g_tims.InputLine(5, (beaconData.Optional / 100) - 1, beaconData.Optional % 100);
		break;
	case 115:
		g_tims.InputLine(6, (beaconData.Optional / 100) - 1, beaconData.Optional % 100);
		break;
	case 116:
		g_tims.InputLine(0, (beaconData.Optional / 10000) - 1, beaconData.Optional % 10000);
		break;
	case 117:
		g_tims.SetArrowState(beaconData.Optional);
		break;
	case 118:
		g_tims.SetAfteruent(1, beaconData.Optional, 0);
		break;
	case 119:
		g_tims.SetAfteruent(2, beaconData.Optional, 0);
		break;
	case 122:
		g_tims.SetAfteruent(0, beaconData.Optional / 100, beaconData.Optional % 100);
		break;
	case 123:
		g_tims.SetAfteruent(3, beaconData.Optional, 0);
		break;
	case 124:
		g_tims.SetLastStop(0, beaconData.Optional);
		break;
	case 125:
		g_tims.SetLastStop(1, beaconData.Optional);
		break;
	case 126:
		g_tims.SetLastStop(2, beaconData.Optional);
		break;
	case 127:
		g_tims.SetLastStation(beaconData.Optional);
		break;
	case 128:
		g_tims.SetTimeStationTime(0, beaconData.Optional);
		break;
	case 129:
		g_tims.SetTimeStationTime(1, beaconData.Optional);
		break;
	case 130:
		g_tims.SetTimeStationTime(2, beaconData.Optional);
		break;
	case 131:
		g_tims.SetTimeStationTime(3, beaconData.Optional);
		break;
	case 132:
		g_tims.SetTimeStationTime(4, beaconData.Optional);
		break;
	case 133:
		g_tims.SetTimeStationName(0, beaconData.Optional);
		break;
	case 134:
		g_tims.SetTimeStationName(1, beaconData.Optional);
		break;
	}
}

ATS_API void WINAPI Dispose(void) 
{
}