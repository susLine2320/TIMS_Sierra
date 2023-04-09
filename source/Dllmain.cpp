// Ats.cpp : DLL �A�v���P�[�V�����p�̃G���g�� �|�C���g���`���܂��B
// (c) Line-16 2023
// Umicorn�l�Akikuike�l�̃\�[�X�R�[�h���Q�l�ɂ����Ă��������Ă��܂��B

#include "stdafx.h"
#include <stdlib.h>
#include "atsplugin.h"
#include "main.h"
#include "tims.h"
#include "Meter.h"
#include "dead.h"
#include "spp.h"
#include "sub.h"

TIMS g_tims; //TIMS�\����
Meter g_meter; //���[�^�[�\����
SPP g_spp; //��ʉߖh�~���u
Sub g_sub; //���̑�
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

	g_tims.Execute(); //TIMS�\����
	g_meter.Execute(); //���[�^�[�\����
	g_dead.execute(); //�d���֘A
	g_spp.Execute(); //��ʉߖh�~���u
	g_sub.Execute(); //���@�\

	// �n���h���o��
	if(g_sub.BcPressCut == 1)
		g_output.Brake = 1; //EB�ɉ����Ƀu���[�L����������
	else
		g_output.Brake = g_brakeNotch;

	g_output.Reverser = g_reverser;

	if (g_time > g_sub.AccelCutting && g_dead.VCB_ON == 1)
		g_output.Power = g_powerNotch;
	else
		g_output.Power = 0;

	g_output.ConstantSpeed = ATS_CONSTANTSPEED_CONTINUE;

	// �p�l���o��
	//���[�^�[�\����
	// �d���v
	panel[21] = g_meter.AMMeterD[4]; //�d���v[����]
	panel[22] = g_meter.AMMeterD[0]; //�d���v[1000��]
	panel[23] = g_meter.AMMeterD[1]; //�d���v[100��]
	panel[24] = g_meter.AMMeterD[2]; //�d���v[10��]
	panel[25] = g_meter.AMMeterD[3]; //�d���v[1��]
	panel[26] = g_meter.AMMeterA[1]; //�d���v[�w�j]
	panel[27] = g_meter.AMMeter[0]; //�d���O���t�i+�j
	panel[28] = g_meter.AMMeter[1]; //�d���O���t�i-�j
	//�u���[�L�V�����_�E����C�_��
	panel[122] = g_meter.BcCaution ? ((g_time % 1000) / 500) : 0; //200kPa�x��
	panel[123] = g_meter.BCMeter[0]; //BC�O���t(0�`180kPa�j
	panel[124] = g_meter.BCMeter[1]; //BC�O���t(200�`380kPa�j
	panel[125] = g_meter.BCMeter[2]; //BC�O���t(400�`580kPa�j
	panel[126] = g_meter.BCMeter[3]; //BC�O���t(600�`780kPa�j
	panel[127] = g_meter.MRMeter[0]; //MR�O���t(750�`795kPa�j
	panel[128] = g_meter.MRMeter[1]; //MR�O���t(800�`845kPa�j
	panel[129] = g_meter.MRMeter[2]; //MR�O���t(850�`895kPa�j
	// �u���[�L�V�����_�E����C�_���i�f�W�^���\���j
	/*
	panel[122] = g_meter.BCPressD[0]; //�u���[�L�V�����_[100��]
	panel[123] = g_meter.BCPressD[1]; //�u���[�L�V�����_[10��]
	panel[124] = g_meter.BCPressD[2]; //�u���[�L�V�����_[1��]
	panel[125] = g_meter.BCPress; //�u���[�L�V�����_[�w�j]
	panel[122] = g_meter.MRPressD[0]; //����C�_��[100��]
	panel[123] = g_meter.MRPressD[1]; //����C�_��[10��]
	panel[124] = g_meter.MRPressD[2]; //����C�_��[1��]
	panel[125] = g_meter.MRPress; //����C�_��[�w�j]
	*/
	// ���x�v
	panel[45] = g_meter.SpeedD[0]; //���x�v[100��]
	panel[46] = g_meter.SpeedD[1]; //���x�v[10��]
	panel[47] = g_meter.SpeedD[2]; //���x�v[1��]
	panel[50] = g_meter.Speed; //���x�v[�w�j]
	// �u���[�L�w�ߌv
	panel[32] = g_meter.AccelDelay; //�͍s�w��
	panel[30] = g_meter.BrakeDelay; //�u���[�L�w��
	panel[31] = g_meter.BrakeDelay == g_emgBrake ? 1 : 0; //���u���[�L
	//�d����
	panel[217] = g_dead.AC; //��
	panel[218] = g_dead.DC; //����
	panel[219] = g_dead.CVacc; //����d���ُ�
	panel[220] = g_dead.CVacc10; //����d��[10��]
	panel[221] = g_dead.CVacc1; //����d��[1��]
	panel[222] = g_dead.ACacc; //�𗬓d���ُ�
	panel[223] = g_dead.ACacc10000; //�𗬓d��[10000��]
	panel[224] = g_dead.ACacc1000; //�𗬓d��[1000��]
	panel[225] = g_dead.ACacc100; //�𗬓d��[100��]
	panel[226] = g_dead.DCacc; //�����d���ُ�
	panel[227] = g_dead.DCacc1000; //�����d��[1000��]
	panel[228] = g_dead.DCacc100; //�����d��[100��]
	panel[229] = g_dead.DCacc10; //�����d��[10��]
	panel[230] = g_dead.Cvmeter; //����w�j
	panel[231] = g_dead.Acmeter; //�𗬎w�j
	panel[232] = g_dead.Dcmeter; //�����w�j
	panel[233] = g_dead.Accident; //����
	panel[234] = g_dead.Tp; //�O��
	panel[236] = g_dead.VCB; //VCB
	panel[235] = g_dead.alert_ACDC > 0 ? g_dead.alert_ACDC + ((g_time % 800) / 400) : 0; //�𒼐؊�

	//TIMS�㕔�\��
	// ���v
	panel[37] = (g_time / 3600000) % 24; //�f�W�^����
	panel[38] = g_time / 60000 % 60; //�f�W�^����
	panel[39] = g_time / 1000 % 60; //�f�W�^���b
	// TIMS���x�v
	panel[100] = g_tims.TimsSpeed[0]; //TIMS���x�v[100��]
	panel[101] = g_tims.TimsSpeed[1]; //TIMS���x�v[10��]
	panel[102] = g_tims.TimsSpeed[2]; //TIMS���x�v[1��]
	// ���s����
	panel[103] = g_tims.Distance[0]; //���s����[km]
	panel[104] = g_tims.Distance[1]; //���s����[100m]
	//panel[105] = g_tims.Distance[2]; //���s����[10m]
	// ���
	panel[109] = g_tims.ArrowDirection; //�i�s�������
	// ���j�b�g�\����
	panel[41] = g_tims.UnitState[0]; //���j�b�g�\����1
	panel[42] = g_tims.UnitState[1]; //���j�b�g�\����2
	panel[43] = g_tims.UnitState[2]; //���j�b�g�\����3
	panel[44] = g_tims.UnitState[3]; //���j�b�g�\����4
	panel[105] = g_tims.UnitTims[0]; //TIMS���j�b�g�\��1
	panel[106] = g_tims.UnitTims[1]; //TIMS���j�b�g�\��2
	panel[107] = g_tims.UnitTims[2]; //TIMS���j�b�g�\��3
	panel[108] = g_tims.UnitTims[3]; //TIMS���j�b�g�\��4

	//TIMS�S�ʕ\��
	panel[110] = g_tims.TrainArrow; //�s�H�\���
	panel[111] = g_tims.Kind; //��Ԏ��
	panel[112] = g_tims.Number[0]; //��Ԕԍ�[1000��]
	panel[113] = g_tims.Number[1]; //��Ԕԍ�[100��]
	panel[114] = g_tims.Number[2]; //��Ԕԍ�[10��]
	panel[115] = g_tims.Number[3]; //��Ԕԍ�[1��]
	panel[116] = g_tims.Charactor; //��Ԕԍ�[�L��]

	panel[117] = g_tims.Number[3] != 0 ? 1 : 0; //�ݒ芮��
	panel[118] = g_tims.PassMode; //�ʉߐݒ�
	panel[119] = g_tims.NextBlinkLamp; //���w��ԕ\����
	panel[120] = g_tims.From; //�^�s�p�^�[���n��
	panel[121] = g_tims.Destination; //�^�s�p�^�[���s��
	panel[208] = g_tims.For; //��ԍs��

	// �X�^�t�e�[�u��
	//�d�񋤒�
	panel[130] = g_tims.HiddenLine[0] ? 0 : g_tims.Station[0]; //�w���\��1
	panel[131] = g_tims.HiddenLine[1] ? 0 : g_tims.Station[1]; //�w���\��2
	panel[132] = g_tims.HiddenLine[2] ? 0 : g_tims.Station[2]; //�w���\��3
	panel[133] = g_tims.HiddenLine[3] ? 0 : g_tims.Station[3]; //�w���\��4
	panel[134] = g_tims.HiddenLine[4] ? 0 : g_tims.Station[4]; //�w���\��5

	panel[135] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][0]; //��������1H
	panel[136] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][1]; //��������1M
	panel[137] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][2]; //��������1S
	panel[138] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][0]; //��������2H
	panel[139] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][1]; //��������2M
	panel[140] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][2]; //��������2S
	panel[141] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][0]; //��������3H
	panel[142] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][1]; //��������3M
	panel[143] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][2]; //��������3S
	panel[144] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][0]; //��������4H
	panel[145] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][1]; //��������4M
	panel[146] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][2]; //��������4S
	panel[147] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][0]; //��������5H
	panel[148] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][1]; //��������5M
	panel[149] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][2]; //��������5S

	panel[150] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][0]; //���Ԏ���1H
	panel[151] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][1]; //���Ԏ���1M
	panel[152] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][2]; //���Ԏ���1S
	panel[153] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][0]; //���Ԏ���2H
	panel[154] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][1]; //���Ԏ���2M
	panel[155] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][2]; //���Ԏ���2S
	panel[156] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][0]; //���Ԏ���3H
	panel[157] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][1]; //���Ԏ���3M
	panel[158] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][2]; //���Ԏ���3S
	panel[159] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][0]; //���Ԏ���4H
	panel[160] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][1]; //���Ԏ���4M
	panel[161] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][2]; //���Ԏ���4S
	panel[162] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][0]; //���Ԏ���5H
	panel[163] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][1]; //���Ԏ���5M
	panel[164] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][2]; //���Ԏ���5S

	panel[165] = g_tims.HiddenLine[0] ? 0 : g_tims.Track[0]; //���w�Ԑ�1
	panel[166] = g_tims.HiddenLine[1] ? 0 : g_tims.Track[1]; //���w�Ԑ�2
	panel[167] = g_tims.HiddenLine[2] ? 0 : g_tims.Track[2]; //���w�Ԑ�3
	panel[168] = g_tims.HiddenLine[3] ? 0 : g_tims.Track[3]; //���w�Ԑ�4
	panel[169] = g_tims.HiddenLine[4] ? 0 : g_tims.Track[4]; //���w�Ԑ�5
	/*
	//��ԃX�^�t
	panel[170] = g_tims.HiddenLine[5] ? 0 : g_tims.Station[5]; //�w���\��6
	panel[171] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][0]; //��������6H
	panel[172] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][1]; //��������6M
	panel[173] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][2]; //��������6S
	panel[174] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][0]; //���Ԏ���6H
	panel[175] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][1]; //���Ԏ���6M
	panel[176] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][2]; //���Ԏ���6S
	panel[177] = g_tims.HiddenLine[5] ? 0 : g_tims.Track[5]; //���w�Ԑ�6

	panel[178] = g_tims.HiddenLine[0] ? 0 : g_tims.Span[0][0]; //�w�ԑ��s����1M
	panel[179] = g_tims.HiddenLine[0] ? 0 : g_tims.Span[0][1]; //�w�ԑ��s����1S
	panel[180] = g_tims.HiddenLine[1] ? 0 : g_tims.Span[1][0]; //�w�ԑ��s����2M
	panel[181] = g_tims.HiddenLine[1] ? 0 : g_tims.Span[1][1]; //�w�ԑ��s����2S
	panel[182] = g_tims.HiddenLine[2] ? 0 : g_tims.Span[2][0]; //�w�ԑ��s����3M
	panel[183] = g_tims.HiddenLine[2] ? 0 : g_tims.Span[2][1]; //�w�ԑ��s����3S
	panel[184] = g_tims.HiddenLine[3] ? 0 : g_tims.Span[3][0]; //�w�ԑ��s����4M
	panel[185] = g_tims.HiddenLine[3] ? 0 : g_tims.Span[3][1]; //�w�ԑ��s����4S
	panel[186] = g_tims.HiddenLine[4] ? 0 : g_tims.Span[4][0]; //�w�ԑ��s����5M
	panel[187] = g_tims.HiddenLine[4] ? 0 : g_tims.Span[4][1]; //�w�ԑ��s����5S

	panel[188] = g_tims.HiddenLine[0] ? 0 : g_tims.LimitA[0]; //�������x1IN
	panel[189] = g_tims.HiddenLine[0] ? 0 : g_tims.LimitL[0]; //�������x1OUT
	panel[190] = g_tims.HiddenLine[1] ? 0 : g_tims.LimitA[1]; //�������x2IN
	panel[191] = g_tims.HiddenLine[1] ? 0 : g_tims.LimitL[1]; //�������x2OUT
	panel[192] = g_tims.HiddenLine[2] ? 0 : g_tims.LimitA[2]; //�������x3IN
	panel[193] = g_tims.HiddenLine[2] ? 0 : g_tims.LimitL[2]; //�������x3OUT
	panel[194] = g_tims.HiddenLine[3] ? 0 : g_tims.LimitA[3]; //�������x4IN
	panel[195] = g_tims.HiddenLine[3] ? 0 : g_tims.LimitL[3]; //�������x4OUT
	panel[196] = g_tims.HiddenLine[4] ? 0 : g_tims.LimitA[4]; //�������x5IN
	panel[197] = g_tims.HiddenLine[4] ? 0 : g_tims.LimitL[4]; //�������x5OUT
	panel[198] = g_tims.HiddenLine[5] ? 0 : g_tims.LimitA[5]; //�������x6IN
	panel[199] = g_tims.HiddenLine[5] ? 0 : g_tims.LimitL[5]; //�������x6OUT
	*/
	//�d�ԃX�^�t
	panel[170] = g_tims.After; //���̎��w
	panel[171] = g_tims.AfterTimeA[0]; //���̎��w������H
	panel[172] = g_tims.AfterTimeA[1]; //���̎��w������M
	panel[173] = g_tims.AfterTimeA[2]; //���̎��w������S
	panel[174] = g_tims.AfterTimeL[0]; //���̎��w������H
	panel[175] = g_tims.AfterTimeL[1]; //���̎��w������M
	panel[176] = g_tims.AfterTimeL[2]; //���̎��w������S
	panel[177] = g_tims.AfterTrack; //���̎��w�Ԑ�

	panel[178] = g_tims.Before; //���O�̎��w
	panel[179] = g_tims.BeforeTime[0]; //���O�̎��w������H
	panel[180] = g_tims.BeforeTime[1]; //���O�̎��w������M
	panel[181] = g_tims.BeforeTime[2]; //���O�̎��w������S
	panel[182] = g_tims.BeforeTrack; //���O�̎��w�Ԑ�

	panel[183] = g_tims.Last; //�~�ԉw
	panel[184] = g_tims.LastTimeA[0]; //�~�ԉw������H
	panel[185] = g_tims.LastTimeA[1]; //�~�ԉw������M
	panel[186] = g_tims.LastTimeA[2]; //�~�ԉw������S
	panel[187] = g_tims.LastTimeL[0]; //�~�ԉw������H
	panel[188] = g_tims.LastTimeL[1]; //�~�ԉw������M
	panel[189] = g_tims.LastTimeL[2]; //�~�ԉw������S
	panel[190] = g_tims.LastTrack; //�~�ԉw�Ԑ�

	panel[191] = g_tims.AfterKind; //��Ԏ��
	panel[192] = g_tims.AfterNumber[0]; //��Ԕԍ�[1000��]
	panel[193] = g_tims.AfterNumber[1]; //��Ԕԍ�[100��]
	panel[194] = g_tims.AfterNumber[2]; //��Ԕԍ�[10��]
	panel[195] = g_tims.AfterNumber[3]; //��Ԕԍ�[1��]
	panel[196] = g_tims.AfterChara; //��Ԕԍ�[�L��]

	panel[197] = g_tims.AfterTime[0][0]; //���s�H������H
	panel[198] = g_tims.AfterTime[0][1]; //���s�H������M
	panel[199] = g_tims.AfterTime[0][2]; //���s�H������S
	panel[200] = g_tims.AfterTime[1][0]; //���s�H������H
	panel[201] = g_tims.AfterTime[1][1]; //���s�H������M
	panel[202] = g_tims.AfterTime[1][2]; //���s�H������S


	//���w�\��
	panel[203] = g_tims.HiddenLine[3] ? 0 : (g_tims.Next * g_tims.NextBlink); //�w���\���i���w�A�_�ł���j
	panel[204] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[0]; //���w������H
	panel[205] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[1]; //���w������M
	panel[206] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[2]; //���w������S
	panel[207] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTrack; //���w���Ԑ�

	// �T�E���h�o��
	sound[100] = g_sub.AirApply; //�u���[�L������
	sound[101] = g_sub.AirApplyEmg; //�u���[�L�������i���j
	sound[103] = g_sub.AirHigh; //���u���[�L�ɉ���
	sound[105] = g_sub.EmgAnnounce; //���u���[�L����
	sound[106] = g_sub.UpdateInfo; //�^�s���X�V

	sound[14] = g_spp.HaltChime3; //��ԃ`���C�����[�v�i�������Ȃ��j
	sound[15] = g_spp.HaltChime; //��ԃ`���C��
	sound[16] = g_spp.PassAlarm; //�ʉ߃`���C��
	sound[17] = g_spp.HaltChime2; //��ԃ`���C�����[�v

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
		if (g_speed != 0)//�w�W�����v�����O����
			g_spp.Recieve(beaconData.Optional % 100000);
		break;
	case 100://���
		g_tims.SetNumber(beaconData.Optional / 100, beaconData.Optional % 100);
		break;
	case 101://���
		g_tims.SetKind(beaconData.Optional);
		break;
	case 102: //�i�s����
		g_tims.SetDirection(beaconData.Optional);
		break;
	case 103: //���s����
		g_tims.SetDistance(beaconData.Distance, beaconData.Optional);
		break;
	case 104://�^�s�p�^�[��
		g_tims.SetLeg(beaconData.Optional);
		break;
	case 105://���w�ڋ�
		if (g_speed != 0)//�w�W�����v�����O����
			g_spp.Recieve(beaconData.Optional % 10000);
		g_tims.Recieve(beaconData.Optional % 10000000, beaconData.Optional / 10000000); //�w�W�����v�����O���Ȃ�
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