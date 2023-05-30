//TIMS.h
//このファイルではTIMS装置をシミュレートします

//諸数値定義（TIMS表示器）
#define TIMS_DECELERATION 19.5F //減速定数（減速度[km/h/s] * 7.2）
#define TIMS_OFFSET 0.5F //車上子オフセット

#define RANDOMIZE_UNITTIMS 30 //TIMSユニット表示の更新確率
#define RANDOMIZE_UNITLAMP 10 //ユニット表示灯の更新確率
#define UPDATE_SPAN 180000 //運行情報更新の最低間隔
#define RELAY_REQUIREMENT 7.5f //走行検知リレー動作する速度

//TIMS表示器
class TIMS
{
public:
	//汎用的な値
	/*
	int emgNotch; //非常ブレーキ
	int *brakeNotch; //ブレーキノッチ
	int *powerNotch; //力行ノッチ
	int *reverser; //レバーサー
	*/
	float BcPressure; //BC圧力[kPa]
	float MrPressure; //MR圧力[kPa]
	float Current; //電流[A]
	double Location; //列車位置[m]

	//TIMS表示器
	int Kind; //列車種別
	int Number[4]; //列車番号
	int Charactor; //列番記号
	int From; //始発駅
	int For; //列車行先
	int Destination; //行先駅
	int This; //自駅
	int Next; //次駅
	int NextBlink; //次駅（点滅）
	int NextBlinkLamp; //次駅停車表示灯
	int NextTime[3]; //次駅到着時刻
	int NextTrack; //次駅到着番線
	int Crawl; //徐行区間
	int CrawlLimit; //徐行速度
	int AfterNumber[4]; //次運用列車番号
	int AfterChara; //次運用列番記号
	int AfterTime[2][3]; //次運用着発時刻
	int AfterKind; //次運用列車種別
	int TrainArrow; //行路表矢印
	int Station[10]; //駅名表示
	int Arrive[10][3]; //到着時刻
	int Leave[10][3]; //発車時刻
	int Track[10]; //到着番線
	int LimitA[10]; //最高速度（上）
	int LimitL[10]; //最高速度（下）
	int Span[10][2]; //駅間走行時間
	int Before; //直前採時駅
	int BeforeTime[3]; //直前採時駅到着時刻
	int BeforeTrack; //直前採時駅番線
	int After; //次採時駅
	int AfterTimeA[3]; //次採時駅到着時刻
	int AfterTimeL[3]; //次採時駅発車時刻
	int AfterTrack; //次採時駅番線
	int Last; //降車駅
	int LastTimeA[3]; //降車駅到着時刻
	int LastTimeL[3]; //降車駅発車時刻
	int LastTrack; //降車駅番線
	int HiddenLine[10]; //更新時に1行ずつ非表示にする
	float StartingPoint; //距離計算の起点
	int Distance[3]; //走行距離
	int TimsSpeed[3]; //TIMS速度
	int UnitTims[10]; //TIMSユニット表示
	int UnitState[10]; //ユニット表示灯

	int Direction; //進行方向
	int ArrowDirection; // 進行方向矢印
	int PassMode; //通過設定

	void Load(void) //初期化
	{
		Kind = 0;
		Charactor = 0;
		From = 0;
		For = 0;
		Destination = 0;
		This = 0;
		Next = 0;
		NextBlink = 0;
		NextBlinkLamp = 0;
		NextTrack = 0;
		Crawl = 0;
		CrawlLimit = 0;
		AfterChara = 0;
		Last = 0;
		LastTrack = 0;
		TrainArrow = 0;

		m_dist = 0;

		m_pushUpFlag = -1;
		m_pushUpBeacon = 0;
		m_pushUpCount = 0;
		m_tmrVisible = g_time;

		m_thisName = 0;
		m_nextName = 0;
		m_nextTrack = 0;
		m_distance = 0;
		m_direction = 1;
		m_distDef = 0;

		ResetArray();

		StartingPoint = 0.0f;
		Distance[0] = 0;
		Distance[1] = 10;
		Distance[2] = 10;
		TimsSpeed[0] = 0;
		TimsSpeed[1] = 0;
		TimsSpeed[2] = 10;
	}

	void Execute(void) //毎フレーム実行
	{
		float speed = fabsf(g_speed); //速度の絶対値[km/h]
		float def = speed / 3600 * g_deltaT; //1フレームで動いた距離（絶対値）[m]
		float def2 = g_speed / 3600 * g_deltaT; //1フレームで動いた距離（相対値）[m]
		m_dist -= def; //残り距離を減算する

		//TIMS表示器メーター
		m_timerTims -= abs(g_deltaT); //毎フレーム減算
		if (m_timerTims < 0)
		{
			double temp_distance = ((Location / 10) - (StartingPoint / 10)) * m_direction + m_distance + (m_distDef / 10);

			//キロ程
			Distance[0] = temp_distance / 100;
			Distance[1] = (temp_distance / 10) - (Distance[0] * 10);
			Distance[2] = temp_distance - (Distance[0] * 100 + Distance[1] * 10);
			if (temp_distance >= 100 && Distance[0] == 0) { Distance[0] = 10; }
			if (temp_distance >= 10 && Distance[1] == 0) { Distance[1] = 10; }

			//速度計
			TimsSpeed[0] = fabs(g_speed) / 100;
			TimsSpeed[1] = (fabs(g_speed) / 10) - (TimsSpeed[0] * 10);
			TimsSpeed[2] = fabs(g_speed) - (TimsSpeed[0] * 100 + TimsSpeed[1] * 10);
			if (fabs(g_speed) >= 1000 && TimsSpeed[0] == 0) { TimsSpeed[0] = 10; }
			if (fabs(g_speed) >= 100 && TimsSpeed[1] == 0) { TimsSpeed[1] = 10; }
			if (TimsSpeed[2] == 0) { TimsSpeed[2] = 10; }

			m_timerTims = 1200;
		}

		//ユニット表示
		for (int i = 0; i < 10; i++) //状態の更新、ユニット状態を10回ループ
		{
			//TIMSユニット表示
			if (Current > 0 && UnitTims[i] != 1 && rand() % RANDOMIZE_UNITTIMS == 0) { UnitTims[i] = 1; }
			else if (Current < 0 && UnitTims[i] != 2 && rand() % RANDOMIZE_UNITTIMS == 0) { UnitTims[i] = 2; }
			else if (Current == 0 && UnitTims[i] != 0 && rand() % RANDOMIZE_UNITTIMS == 0) { UnitTims[i] = 0; }

			//ユニット表示灯
			if (Current != 0 && UnitState[i] != 1 && rand() % RANDOMIZE_UNITLAMP == 0) { UnitState[i] = 1; }
			else if (Current == 0 && UnitState[i] != 0 && rand() % RANDOMIZE_UNITLAMP == 0) { UnitState[i] = 2; }
		}

		//進行方向矢印
		switch (Direction)
		{
		case 0:
		default:
			ArrowDirection = 0;
			break;
		case 1:
			switch (g_reverser)
			{
			case 1:
				ArrowDirection = 1;
				break;
			case -1:
				ArrowDirection = 2;
				break;
			case 0:
			default:
				ArrowDirection = 0;
				break;
			}
			break;
		case 2:
			switch (g_reverser)
			{
			case 1:
				ArrowDirection = 2;
				break;
			case -1:
				ArrowDirection = 1;
				break;
			case 0:
			default:
				ArrowDirection = 0;
				break;
			}
			break;
		}

		//次駅を点滅させる
		if (speed * speed / TIMS_DECELERATION >= m_dist - 50 && m_dist > 0 && !m_blinking) //速度照査（パターン)
		{
			m_blinking = true;

			if (m_pushUpFlag == 1) //停車駅のとき
			{
				m_pushUpFlag = 2;
			}
		}

		//駅名表示の点滅
		NextBlink = m_pushUpFlag == 2 ? ((g_time % 750) / 375) : 1; //iniでいじれるようにする
		NextBlinkLamp = m_pushUpFlag == 2 ? 1 : 0;

		//プッシュアップイベント
		if (g_speed != 0) //駅ジャンプを除外する
		{
			if ((m_pushUpFlag == 2 && (m_pushUpBeacon == 1 || g_speed < 2.0f)) || (m_pushUpFlag == -1 && m_dist <= 0))
			{
				m_pushUpFlag = 0;

				//ステップ更新の回数だけループ
				for (; m_pushUpCount > 0; m_pushUpCount--)
				{
					if (m_pushUpCount > 1)
					{
						for (int i = 0; i < 10; i++)
						{
							PushUp(i);
						}
					}
					else
					{
						m_tmrVisible = g_time;
						for (int i = 0; i < 10; i++)
						{
							m_update[i] = 1;

							//3回目の時、次駅を最新にする、採時駅を最新にする、列番を更新する
							if (i == 3)
							{
								PushNext();
								PushTimeStation();
								PushTrainNumber();
							}
						}
					}
				}

				//ページめくりしない
				if (m_option > 0)
				{					
					PushNext();
					PushTimeStation();
					PushTrainNumber();
				}
			}
		}
		//以後駅ジャンプ
		else if (m_pushUpFlag == 3 || m_pushUpBeacon == 2)
		{
			m_pushUpFlag = 0;
			PushNext();
			PushTimeStation();
			PushTrainNumber();

			for (int i = 0; i < 10; i++)
			{
				PushUp(i);
			}
		}
		else if (m_pushUpFlag == -1) //起動時の初期化
		{
			PushNext();
			PushTimeStation();
			PushTrainNumber();
		}

		//ステップ更新の処理
		for (int i = 0; i < 10; i++)
		{
			if (g_time >= m_tmrVisible + (i * (LineUpdate * (2 / 3))) && g_time <= m_tmrVisible + LineUpdate * (i + 1) - (LineUpdate * (1 / 3)))
			{
				HiddenLine[i] = 1;

				if (m_update[i] == 1)
				{
					m_update[i] = 0;
					PushUp(i);
				}
			}
			else
			{
				HiddenLine[i] = 0;
			}
		}
	}

	//開扉時に実行
	void DoorOpen(void)
	{
		m_door = 0; //ドア状態
		m_tmrVisible = 0; //表示の更新をさせない
	}

	//閉扉時に実行
	void DoorClose(void)
	{
		m_door = 1; //ドア状態
	}

	//以下地上子通過時に実行

	//次駅接近（105）
	void Recieve(int data, int option)
	{
		m_pushUpFlag = data >= 0 ? 1 : -1;
		m_pushUpBeacon = 0;
		m_pushUpCount = abs(data) > 100000 ? 0 : abs(data) / 10000 > 0 ? abs(data) / 10000 : 1;
		m_option = abs(option) > 0 ? 1 : 0;

		m_dist = abs(data % 10000) - TIMS_OFFSET;
		m_blinking = false;

		if (g_speed == 0) //駅ジャンプを除外する
		{
			m_pushUpFlag = abs(data) < 100000 ? 3 : 0;
			m_pushUpCount = 0;
			m_dist = 0;
		}
	}

	//自駅、次駅の設定（106）
	void SetNext(int data)
	{
		m_thisName = data / 1000;
		m_nextName = data % 1000;
	}

	//次駅の時刻設定（107）
	void SetNextTime(int data)
	{
		m_nextTime[0] = data / 10000;
		m_nextTime[1] = ((data % 10000) - (data % 100)) / 100;
		m_nextTime[2] = (data % 100) / 5;
	}

	//次駅到着番線の設定（108）
	void SetNextTrack(int data)
	{
		m_nextTrack = data;
	}

	//列車種別の設定（101）
	void SetKind(int data)
	{
		m_kind = data;
	}

	//列車番号の設定（100，150）
	void SetNumber(int data, int chara)
	{
		m_charactor = chara; //記号は下2桁
		m_Number[0] = data >= 1000 ? (data / 1000) % 10 == 0 ? 10 : (data / 1000) % 10 : 0; //1000未満は0，その他は数値通り
		m_Number[1] = data >= 100 ? (data / 100) % 10 == 0 ? 10 : (data / 100) % 10 : 0; //100未満は0，0は10，その他は数値通り
		m_Number[2] = data >= 10 ? (data / 10) % 10 == 0 ? 10 : (data / 10) % 10 : 0; //10未満は0，0は10，その他は数値通り
		m_Number[3] = data % 10 == 0 ? 10 : data % 10; //0は10，その他は数値通り
	}

	//運行区間の設定（104）
	void SetLeg(int data)
	{
		m_from = data / 1000;
		m_destination = data % 1000;
	}

	//行先の設定（109）
	void SetFor(int data)
	{
		m_for = data;
	}

	//徐行区間の設定（135）
	void SetCrawl(int data)
	{
		if (data > 0)
		{
			Crawl = 1;
			CrawlLimit = data / 5;
		}
		else
		{
			Crawl = 0;
			CrawlLimit = 0;
		}
	}

	//次運用の設定（118，119，122，123）
	void SetAfteruent(int type, int data, int chara)
	{
		switch (type)
		{
		case 0://列番
			m_afterChara = chara; //記号は下2桁
			m_afterNumber[0] = data >= 1000 ? (data / 1000) % 10 == 0 ? 10 : (data / 1000) % 10 : 0;
			m_afterNumber[1] = data >= 100 ? (data / 100) % 10 == 0 ? 10 : (data / 100) % 10 : 0;
			m_afterNumber[2] = data >= 10 ? (data / 10) % 10 == 0 ? 10 : (data / 10) % 10 : 0;
			m_afterNumber[3] = data % 10 == 0 ? 10 : data % 10;
			break;
		case 1://到着時刻
			m_afterTime[0][0] = data / 10000;
			m_afterTime[0][1] = ((data % 10000) - (data % 100)) / 100;
			m_afterTime[0][2] = (data % 100) / 5;
			break;
		case 2://発車時刻
			m_afterTime[1][0] = data / 10000;
			m_afterTime[1][1] = ((data % 10000) - (data % 100)) / 100;
			m_afterTime[1][2] = (data % 100) / 5;
			break;
		case 3://種別
			m_afterKind = data;
			break;
		default:
			break;
		}
	}

	//進行方向設定（102）
	void SetDirection(int direction)
	{
		switch (direction)
		{
		case 1://左
			Direction = 1;
			break;
		case 2://右
			Direction = 2;
			break;
		case 0://例外
		default:
			Direction = 0;
			break;
		}
	}

	//降車駅設定（124～126）
	void SetLastStop(int type, int data)
	{
		switch (type)
		{
		case 0://到着時刻
			m_lastTimeA[0] = data / 10000;
			m_lastTimeA[1] = ((data % 10000) - (data % 100)) / 100;
			m_lastTimeA[2] = (data % 100) / 5;
			break;
		case 1://発車時刻
			m_lastTimeL[0] = data / 10000;
			m_lastTimeL[1] = ((data % 10000) - (data % 100)) / 100;
			m_lastTimeL[2] = (data % 100) / 5;
			break;
		case 2://番線
			m_lastTrack = data;
			break;
		default:
			break;
		}
	}

	//降車駅名設定（127）
	void SetLastStation(int data)
	{
		m_lastName = data;
	}

	//採時駅時刻番線設定（128～132）
	void SetTimeStationTime(int type, int data)
	{
		switch (type)
		{
		case 0://直前採時駅時刻
			m_beforeTime[0] = data / 10000;
			m_beforeTime[1] = ((data % 10000) - (data % 100)) / 100;
			m_beforeTime[2] = (data % 100) / 5;
			break;
		case 1://次採時駅到着時刻
			m_afterTimeA[0] = data / 10000;
			m_afterTimeA[1] = ((data % 10000) - (data % 100)) / 100;
			m_afterTimeA[2] = (data % 100) / 5;
			break;
		case 2://次採時駅発車時刻
			m_afterTimeL[0] = data / 10000;
			m_afterTimeL[1] = ((data % 10000) - (data % 100)) / 100;
			m_afterTimeL[2] = (data % 100) / 5;
			break;
		case 3://直前採時駅番線
			m_beforeTrack = data;
			break;
		case 4://次採時駅番線
			m_afterTrack = data;
			break;
		default:
			break;
		}
	}

	//採時駅駅名設定（133，134）
	void SetTimeStationName(int type, int data)
	{
		if (type == 0)//直前採時駅設定
			m_beforeName = data;
		else//次採時駅設定
			m_afterName = data;
	}

	//行路表矢印の設定（117）
	void SetArrowState(int data)
	{
		TrainArrow = data;
	}

	//走行距離の設定（103）
	void SetDistance(float distance, int data)
	{
		StartingPoint = distance; //距離計算の起点
		m_direction = data >= 1000000 ? -1 : 1;
		m_distance = data % 1000000; //走行距離
	}

	//走行距離の設定（snp2-114）
	void SetPositionDef(int data)
	{
		m_distDef = data; //列車位置の補正
	}

	//スタフテーブルの書き込み（110～116）
	void InputLine(int column, int line, int data)
	{
		switch (column)
		{
		case 0: //駅間走行時間
			Span[line][0] = data / 100;
			Span[line][1] = (data % 100) / 5;
			break;
		case 9: //駅間走行時間snp2
			Span[line][0] = data / 100;
			Span[line][1] = (data % 100) / 5;
			if (Span[line][0] == 0)
				Span[line][0] = 60;
			else if (Span[line][0] == 60)
				Span[line][0] = 0;
			if (Span[line][1] == 0)
				Span[line][1] = 12;
			else if (Span[line][1] == 12)
				Span[line][1] = 0;
			break;
		case 1: //駅名表示
			Station[line] = data;
			break;
		case 2: //到着時刻
			Arrive[line][0] = data / 10000;
			Arrive[line][1] = ((data % 10000) - (data % 100)) / 100;
			Arrive[line][2] = (data % 100) / 5;
			break;
		case 3: //発車時刻
			Leave[line][0] = data / 10000;
			Leave[line][1] = ((data % 10000) - (data % 100)) / 100;
			Leave[line][2] = (data % 100) / 5;
			break;
		case 7: //到着時刻snp2
			Arrive[line][0] = data / 10000;
			Arrive[line][1] = ((data % 10000) - (data % 100)) / 100;
			Arrive[line][2] = (data % 100) / 5;
			if (Arrive[line][0] == 0)
				Arrive[line][0] = 24;
			else if (Arrive[line][0] == 24)
				Arrive[line][0] = 0;
			if (Arrive[line][1] == 0)
				Arrive[line][1] = 60;
			else if (Arrive[line][1] == 60)
				Arrive[line][1] = 0;
			if (Arrive[line][2] == 0)
				Arrive[line][2] = 12;
			else if (Arrive[line][2] == 12)
				Arrive[line][2] = 0;
			break;
		case 8: //発車時刻snp2
			Leave[line][0] = data / 10000;
			Leave[line][1] = ((data % 10000) - (data % 100)) / 100;
			Leave[line][2] = (data % 100) / 5;
			if (Leave[line][0] == 0)
				Leave[line][0] = 24;
			else if (Leave[line][0] == 24)
				Leave[line][0] = 0;
			if (Leave[line][1] == 0)
				Leave[line][1] = 60;
			else if (Leave[line][1] == 60)
				Leave[line][1] = 0;
			if (Leave[line][2] == 0)
				Leave[line][2] = 12;
			else if (Leave[line][2] == 12)
				Leave[line][2] = 0;
			break;
		case 4: //番線
			Track[line] = data;
			break;
		case 5: //進入制限
			LimitA[line] = data / 5;
			break;
		case 6://進出制限
			LimitL[line] = data / 5;
			break;
		}
	}

private:
	int m_door; //ドア状態
	float m_dist; //停止予定点距離
	bool m_blinking; //次駅点滅かどうか

	int m_pushUpFlag; //表示更新のフラグ
	int m_pushUpBeacon; //表示更新の地上子
	int m_pushUpCount; //表示更新の繰り返し数
	int m_tmrVisible; //モニタのステップ更新
	int m_update[7]; //ステップ更新の状態
	int m_thisName; //自駅駅名
	int m_nextName; //次駅駅名
	int m_nextTime[3]; //次駅時刻
	int m_nextTrack; // 次駅到着番線
	int m_beforeName; //直前採時駅名
	int m_beforeTime[3]; //直前採時駅時刻
	int m_beforeTrack; // 前採時駅番線
	int m_afterName; //次採時駅名
	int m_afterTimeA[3]; //次採時駅着時刻
	int m_afterTimeL[3]; //次採時駅発時刻
	int m_afterTrack; //次採時駅番線
	int m_lastName; //降車駅駅名
	int m_lastTimeA[3]; //降車駅到着時刻
	int m_lastTimeL[3]; //降車駅出発時刻
	int m_lastTrack; //降車駅番線
	int m_from; //運行パターン発駅
	int m_destination; //運行パターン着駅
	int m_for; //列車行先
	int m_option; //次駅と採時駅のみ更新
	int m_kind; //列車種別
	int m_Number[4]; //列車番号
	int m_charactor; //列番記号
	int m_afterNumber[4]; //次運用列番
	int m_afterChara; //次運用列番記号
	int m_afterTime[2][3]; //次運用着発時刻
	int m_afterKind; //次運用種別

	double m_distance; //走行距離
	int m_direction; //距離減算するかどうか
	int m_distDef; //列車位置の補正
	int m_timerTims; //TIMS表示器更新タイマー

	//スタフテーブルのプッシュアップ
	void PushUp(int line)
	{
		if (line == 9)//10行目の時刻以外リセット
		{
			Station[line] = 0;
			Track[line] = 0;
			LimitA[line] = 0;
			LimitL[line] = 0;
			Span[line][0] = 0;
			Span[line][1] = 0;
		}
		else //1から9行目の繰り上げ
		{
			Station[line] = Station[line + 1];
			Track[line] = Track[line + 1];
			LimitA[line] = LimitA[line + 1];
			LimitL[line] = LimitL[line + 1];
			Span[line][0] = Span[line + 1][0];
			Span[line][1] = Span[line + 1][1];
		}

		for (int j = 0; j < 3; j++)//着発時刻のリセット
		{
			if (line == 9) //10行目を空にする
			{
				switch (j)
				{
				case 0:
				case 1:
				case 2:
					Arrive[line][j] = 0;
					Leave[line][j] = 0;
					break;
				}
			}
			else //1から9行目は繰り上げ
			{
				Arrive[line][j] = Arrive[line + 1][j];
				Leave[line][j] = Leave[line + 1][j];
			}
		}
	}

	//入力されている次駅を画面に反映
	void PushNext(void)
	{
		This = m_thisName;
		Next = m_nextName;
		NextTime[0] = m_nextTime[0];
		NextTime[1] = m_nextTime[1];
		NextTime[2] = m_nextTime[2];
		NextTrack = m_nextTrack;
	}

	//入力されている採時駅を画面に反映
	void PushTimeStation(void)
	{
		Before = m_beforeName; //直前採時駅
		After = m_afterName; //次採時駅
		Last = m_lastName; //降車駅

		BeforeTime[0] = m_beforeTime[0];
		BeforeTime[1] = m_beforeTime[1];
		BeforeTime[2] = m_beforeTime[2];

		AfterTimeA[0] = m_afterTimeA[0];
		AfterTimeA[1] = m_afterTimeA[1];
		AfterTimeA[2] = m_afterTimeA[2];

		AfterTimeL[0] = m_afterTimeL[0];
		AfterTimeL[1] = m_afterTimeL[1];
		AfterTimeL[2] = m_afterTimeL[2];

		LastTimeA[0] = m_lastTimeA[0];
		LastTimeA[1] = m_lastTimeA[1];
		LastTimeA[2] = m_lastTimeA[2];

		LastTimeL[0] = m_lastTimeL[0];
		LastTimeL[1] = m_lastTimeL[1];
		LastTimeL[2] = m_lastTimeL[2];

		BeforeTrack = m_beforeTrack;
		AfterTrack = m_afterTrack;
		LastTrack = m_lastTrack;

		From = m_from;
		Destination = m_destination;
		For = m_for;
	}

	//入力されている列車番号と種別を画面に反映
	void PushTrainNumber(void)
	{
		//現在種別列番
		Kind = m_kind % 100;
		PassMode = (m_kind >= 100) ? (m_kind / 100) : 0; //通過設定

		Number[0] = m_Number[0];
		Number[1] = m_Number[1];
		Number[2] = m_Number[2];
		Number[3] = m_Number[3];
		Charactor = m_charactor;

		//次行路列番
		AfterKind = m_afterKind;
		AfterNumber[0] = m_afterNumber[0];
		AfterNumber[1] = m_afterNumber[1];
		AfterNumber[2] = m_afterNumber[2];
		AfterNumber[3] = m_afterNumber[3];
		AfterChara = m_afterChara;

		//次行路発着時刻
		AfterTime[0][0] = m_afterTime[0][0];
		AfterTime[0][1] = m_afterTime[0][1];
		AfterTime[0][2] = m_afterTime[0][2];
		AfterTime[1][0] = m_afterTime[1][0];
		AfterTime[1][1] = m_afterTime[1][1];
		AfterTime[1][2] = m_afterTime[1][2];
	}

	//配列の初期化
	void ResetArray(void)
	{
		for (int c3 = 0; c3 < 3; c3++)//3回ループ、降車駅のリセット
		{
			LastTimeA[c3] = 0;
			LastTimeL[c3] = 0;
			m_nextTime[c3] = 0;

			for (int c3_2 = 0; c3_2 < 2; c3_2++)//2回ループ、次行路時刻のリセット
			{
				AfterTime[c3_2][c3] = 0;
			}
		}

		for (int st3 = 0; st3 < 3; st3++)//3回ループ、採時駅のリセット
		{
			BeforeTime[st3] = 0;
			AfterTimeA[st3] = 0;
			AfterTimeL[st3] = 0;
			m_beforeTime[st3] = 0;
			m_afterTimeA[st3] = 0;
			m_afterTimeL[st3] = 0;
		}
		/*
		for (int o3 = 0; o3 < 3; o3++)//3回ループ、色々リセット
		{

		}
		*/
		for (int c4 = 0; c4 < 4; c4++)//4回ループ、列番のリセット
		{
			Number[c4] = 10;
			AfterNumber[c4] = 10;
		}

		for (int c7 = 0; c7 < 7; c7++)//7回ループ、アップデートをリセット
		{
			m_update[c7] = 0;
		}

		for (int c10 = 0; c10 < 10; c10++)//10回ループ、時刻以外のスタフテーブルのリセット
		{
			Station[c10] = 0;
			Track[c10] = 0;
			LimitA[c10] = 0;
			LimitL[c10] = 0;

			HiddenLine[c10] = 0;
			UnitTims[c10] = 0;
			UnitState[c10] = 0;

			Arrive[c10][0] = 0;
			Leave[c10][0] = 0;
			Span[c10][0] = 0;

			Arrive[c10][1] = 0;
			Leave[c10][1] = 0;
			Span[c10][1] = 0;

			Arrive[c10][2] = 0;
			Leave[c10][2] = 0;
		}
	}
};
