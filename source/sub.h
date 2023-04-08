//sub.h
//ブレーキ減圧などの機能を実装します

class Sub
{
private:
	int m_tmrBcPressCut;
	int m_relayD; //走行検知リレー
	int m_lbInit; //初回起動時断流器音
	int m_airHigh; //非常ブレーキ緩解音
	int m_airApply; //ブレーキ昇圧音
	float m_bcPrsDelta;
	int m_emgAnnounce; //非常ブレーキ放送
	int m_airApplyEmg; //ブレーキ昇圧音（非常）
	int m_updateInfo; //運行情報更新
	int m_deltaUpdate; //
public:
	int BcPressCut; //ブレーキ減圧する
	int RelayState; //走行検知リレー

	int AccelCutting; //力行遅延
	int RelayD; //走行検知リレー
	int LbInit; //初回起動時断流器音
	int AirHigh; //非常ブレーキ緩解音
	int AirApply; //ブレーキ昇圧音
	int EmgAnnounce; //非常ブレーキ放送
	int AirApplyEmg; //ブレーキ昇圧音（非常）
	int UpdateInfo; //運行情報更新

	int event_lbInit; //初回起動時力行遅延
	int BcPressure;

	void load(void)
	{
		BcPressCut = 0;
		RelayState = 0;
		AccelCutting = 0;
		m_tmrBcPressCut = 0;
		m_relayD = ATS_SOUND_STOP;
		m_lbInit = ATS_SOUND_STOP;
		m_airHigh = ATS_SOUND_STOP;
		AirApply = ATS_SOUND_STOP;
		m_bcPrsDelta = 0.0F;
		m_emgAnnounce = ATS_SOUND_STOP;
		m_airApplyEmg = ATS_SOUND_STOP;
		m_updateInfo = ATS_SOUND_STOP;
		m_deltaUpdate = g_time - UPDATE_SPAN;
	}

	void Execute(void)
	{
		//初回起動時力行遅延
		if (event_lbInit > 0 && g_powerNotch > 0)
		{
			event_lbInit = 0;
			AccelCutting = g_time + 500;
			m_lbInit = ATS_SOUND_PLAY;
		}

		//初回起動時断流器音
		LbInit = m_lbInit;
		m_lbInit = ATS_SOUND_CONTINUE;

		if (m_tmrBcPressCut < g_time)
		{
			BcPressCut = 0;
			m_tmrBcPressCut = 0;
		}
		else
		{
			BcPressCut = 1;
		}

		//緩解遅延タイマーの防護
		if (fabs(g_speed) > 3.0f)
		{
			m_tmrBcPressCut = 0;
			BcPressCut = 0;
		}

		//走行検知リレー
		if (RelayState == 0 && fabs(g_speed) >= RELAY_REQUIREMENT)
		{
			RelayState = 1;
			m_relayD = ATS_SOUND_PLAY;
		}
		else if (RelayState == 1 && fabs(g_speed) <= RELAY_REQUIREMENT)
		{
			RelayState = 0;
			m_relayD = ATS_SOUND_PLAY;
		}

		RelayD = m_relayD;
		m_relayD = ATS_SOUND_CONTINUE;

		//非常ブレーキ緩解音
		AirHigh = m_airHigh;
		m_airHigh = ATS_SOUND_CONTINUE;

		//ブレーキ昇圧音
		if (g_brakeNotch > 0 && BcPressure > 40.0F && BcPressure > m_bcPrsDelta)
			AirApply = ATS_SOUND_PLAYLOOPING;
		else
			AirApply = ATS_SOUND_STOP;

		m_bcPrsDelta = BcPressure; //BC圧力[kPa]

		//非常ブレーキ放送
		EmgAnnounce = m_emgAnnounce;
		m_emgAnnounce = ATS_SOUND_CONTINUE;

		//ブレーキ昇圧音（非常）
		AirApplyEmg = m_airApplyEmg;
		m_airApplyEmg = ATS_SOUND_CONTINUE;

		//運行情報更新
		if (g_speed > 10.0f && g_time > m_deltaUpdate + UPDATE_SPAN)
		{
			m_deltaUpdate = g_time;
			
			if (rand() % 200 == 0)
				m_updateInfo = ATS_SOUND_PLAY;
		}
		UpdateInfo = m_updateInfo;//運行情報更新
		m_updateInfo = ATS_SOUND_CONTINUE;
	}

	//非常ブレーキ緩解音を再生
	void PlaySoundAirHigh(int state, int newState)
	{
		if (state == g_emgBrake && newState < g_emgBrake && BcPressure >340)
		{
			m_airHigh = ATS_SOUND_PLAY;
			m_tmrBcPressCut = g_time + 1000; //ブレーキ減圧のタイマー
		}
	}

	//非常ブレーキ放送を再生する
	void PlaySoundEmgAnnounce(int state, int newState)
	{
		if (state != g_emgBrake && newState == g_emgBrake && g_speed > 5.0F)//非常ブレーキ放送速度
		{
			m_emgAnnounce = ATS_SOUND_PLAY;
		}
	}

	//初回起動時断
	void SetLbInit(int pos, int enable)
	{
		if (pos == 0 && enable == 1)
		{
			event_lbInit = 1;
		}
	}

	//ブレーキ昇圧音（非常）を再生する
	void PlaySoundAirApplyEmg(int state, int newState)
	{
		if (state != g_emgBrake && newState == g_emgBrake && BcPressure < 150.0f)
		{
			m_airApplyEmg = ATS_SOUND_PLAY;
		}
	}
};