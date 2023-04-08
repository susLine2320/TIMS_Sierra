//spp.h
//このファイルでは誤通過防止装置をシミュレートします

//諸数値定義（誤通過防止装置）
#define SPP_DECELERATION 19.5F //減速定数（減速度[km/h/s] * 7.2）
#define SPP_OFFSET 0.5F //車上子オフセット[m]
#define SPP_ALARMTYPE 1

//誤通過防止装置
class SPP
{
private:
	float m_dist; //停止予定点距離
	bool m_played;
	bool m_halt;
	int m_haltChime;
	int m_haltChime2;
	int m_haltChime3;
	int m_passAlarm;
public:
	int HaltChime; //停車チャイム
	int HaltChime2; //停車チャイムループ
	int HaltChime3; //停車チャイムループ（減少しない）
	int PassAlarm; //通過チャイム
	int ServiceNotch; //ATS確認ノッチ

	void Initialize(void) //初期化
	{
		m_dist = 0;
		m_haltChime = ATS_SOUND_STOP;
		m_haltChime2 = ATS_SOUND_STOP;
		m_haltChime3 = ATS_SOUND_STOP;
	}

	void Execute(void)//毎フレーム実行
	{
		float speed = fabsf(g_speed);
		float def = speed / 3600 * g_deltaT;
		m_dist -= def; //残り距離を減算する

		//警報を鳴らす
		if (speed * speed / SPP_DECELERATION >= m_dist - 50 && m_dist > 0 && !m_played)
		{
			m_played = true;
			if (m_halt) //停車の場合
			{
				m_haltChime = ATS_SOUND_PLAY;
				m_haltChime3 = ATS_SOUND_PLAYLOOPING;
				if (g_brakeNotch >= ServiceNotch)
					m_haltChime2 = -1200; //ブレーキを込めた場合は音量を小さくする（-12dB）
				else
					m_haltChime2 = 0;
			}
			else //通過の場合
				m_passAlarm = ATS_SOUND_PLAY;
		}

		//サウンドを更新
		HaltChime = m_haltChime;
		HaltChime2 = m_haltChime2;
		HaltChime3 = m_haltChime3;
		PassAlarm = m_passAlarm;

		//CONTINUEにしておく
		if (m_haltChime == ATS_SOUND_PLAY) { m_haltChime = ATS_SOUND_CONTINUE; }
		if (m_haltChime2 == ATS_SOUND_PLAY) { m_haltChime2 = ATS_SOUND_CONTINUE; }
		if (m_haltChime3 == ATS_SOUND_PLAYLOOPING) { m_haltChime3 = ATS_SOUND_CONTINUE; }
		m_passAlarm = ATS_SOUND_CONTINUE;
	}
	
	void NotchChanged(void)
	{
		// ブレーキを込めた場合は音量を小さくする
		if (m_haltChime2 == 0 && g_brakeNotch >= ServiceNotch) { m_haltChime2 = -1200; }
	}
	

	//開扉時に実行
	void StopChime(void)
	{
		m_haltChime = ATS_SOUND_STOP;
		m_haltChime2 = ATS_SOUND_STOP;
		m_haltChime3 = ATS_SOUND_STOP;
	}

	//#105で実行
	void Recieve(int data)
	{
		m_halt = data >= 0;
		m_dist = abs(data) - SPP_OFFSET;
		m_played = false;
		m_haltChime = ATS_SOUND_STOP;
		m_haltChime2 = ATS_SOUND_STOP;
		m_haltChime3 = ATS_SOUND_STOP;
	}
};