//dead.h
//デッドセクションでの扱いを定義します

class DEAD
{
private:
public:
	int AC;
	int DC;
	int CVacc;
	int CVacc10;
	int CVacc1;
	int ACacc;
	int ACacc10000;
	int ACacc1000;
	int ACacc100;
	int DCacc;
	int DCacc1000;
	int DCacc100;
	int DCacc10;
	int Cvmeter;
	int Acmeter;
	int Dcmeter;
	int Accident;
	int Tp;
	int VCB_ON;
	int VCB_OFF;
	int VCB;
	int tmr_ACDC;
	int step_ACDC;
	int alert_ACDC;

	void load(void)
	{
		AC = 0;
		DC = 1;
		CVacc = 0;
		CVacc10 = 10;
		CVacc1 = 10;
		ACacc = 0;
		ACacc10000 = 10;
		ACacc1000 = 10;
		ACacc100 = 10;
		DCacc = 0;
		DCacc1000 = 10;
		DCacc100 = 10;
		DCacc10 = 10;
		Cvmeter = 100;
		Acmeter = 0;
		Dcmeter = 1600;
		Accident = 0;
		Tp = 0;
		VCB_ON = 1;
		VCB_OFF = 0;
		VCB = 1;
		tmr_ACDC = g_time;
		step_ACDC = 3;
		alert_ACDC = 0;
	}

	void execute(void)
	{
		int temp_time = g_time - tmr_ACDC;
		switch (step_ACDC)
		{
		case 0: //直流→VCB切
			VCB_OFF = 1;
			VCB_ON = 0;
			VCB = 2;

			if (temp_time > 250)
				Accident = 1; //事故
			if (temp_time > 500)
			{
				DCacc = 1;
				DCacc1000 = 10;
				DCacc100 = 10;
				DCacc10 = 10;
				Dcmeter = 0;
			}
			if (temp_time > 750)
			{
				CVacc = 1;
				Cvmeter = 83;
				CVacc10 = 8;
				CVacc1 = 3;
			}
			if (temp_time > 1000)
				Tp = 1; //三相
			break;
		case 1: //無電区間
			VCB_OFF = 0;
			VCB_ON = 0;
			VCB = 0;

			AC = 0;
			DC = 0;
			ACacc = 0;
			ACacc10000 = 10;
			ACacc1000 = 10;
			ACacc100 = 10;
			DCacc = 0;
			DCacc1000 = 10;
			DCacc100 = 10;
			DCacc10 = 10;
			Acmeter = 0;
			Dcmeter = 0;
			break;
		case 2: //AC有電区間
			VCB_OFF = 0;
			VCB_ON = 1;
			VCB = 1;

			DC = 0;
			DCacc = 0;
			DCacc1000 = 10;
			DCacc100 = 10;
			DCacc10 = 10;
			Dcmeter = 0;

			AC = 1;
			if (temp_time > 500)
			{
				ACacc = 1;
				ACacc10000 = 10;
				ACacc1000 = 2;
				ACacc100 = 2;
				Acmeter = 220;
			}
			if (temp_time > 1000)
			{
				ACacc = 1;
				ACacc10000 = 1;
				ACacc1000 = 1;
				ACacc100 = 2;
				Acmeter = 11200;
			}
			if (temp_time > 2000)
			{
				ACacc = 0;
				ACacc10000 = 10;
				ACacc1000 = 10;
				ACacc100 = 10;
				Acmeter = 15500;
			}
			if (temp_time > 3000)
			{
				ACacc = 0;
				ACacc10000 = 10;
				ACacc1000 = 10;
				ACacc100 = 10;
				Acmeter = 18000;
			}
			if (temp_time > 4000)
			{
				ACacc = 0;
				ACacc10000 = 10;
				ACacc1000 = 10;
				ACacc100 = 10;
				Acmeter = 20000;
			}
			if (temp_time > 5000)
			{
				ACacc = 0;
				ACacc10000 = 10;
				ACacc1000 = 10;
				ACacc100 = 10;
				Acmeter = 22000;
			}
			if (temp_time > 8000)
				Accident = 0; //事故
			if (temp_time > 12000)
				Tp = 0; //三相
			if (temp_time > 13000)
			{
				CVacc = 0;
				CVacc10 = 10;
				CVacc1 = 10;
				Cvmeter = 100;
			}
			break;
		case 3: //DC有電区間
			VCB_OFF = 0;
			VCB_ON = 1;
			VCB = 1;

			AC = 0;
			ACacc = 0;
			ACacc10000 = 10;
			ACacc1000 = 10;
			ACacc100 = 10;
			Acmeter = 0;

			DC = 1;
			if (temp_time > 500)
			{
				DCacc = 1;
				DCacc1000 = 10;
				DCacc100 = 1;
				DCacc10 = 8;
				Dcmeter = 180;
			}
			if (temp_time > 1000)
			{
				DCacc = 1;
				DCacc1000 = 10;
				DCacc100 = 7;
				DCacc10 = 4;
				Dcmeter = 740;
			}
			if (temp_time > 2000)
			{
				DCacc = 0;
				DCacc1000 = 10;
				DCacc100 = 10;
				DCacc10 = 10;
				Dcmeter = 1380;
			}
			if (temp_time > 3000)
			{
				DCacc = 0;
				DCacc1000 = 10;
				DCacc100 = 10;
				DCacc10 = 10;
				Dcmeter = 1500;
			}
			if (temp_time > 4000)
			{
				DCacc = 0;
				DCacc1000 = 10;
				DCacc100 = 10;
				DCacc10 = 10;
				Dcmeter = 1600;
			}
			if (temp_time > 7000)
				Accident = 0; //事故
			if (temp_time > 11000)
				Tp = 0; //三相
			if (temp_time > 12000)
			{
				CVacc = 0;
				CVacc10 = 10;
				CVacc1 = 10;
				Cvmeter = 100;
			}
			break;
		case 4: //交流→VCB切
			VCB_OFF = 1;
			VCB_ON = 0;
			VCB = 2;

			if (temp_time > 250)
				Accident = 1; //事故
			if (temp_time > 500)
			{
				ACacc = 1;
				ACacc10000 = 10;
				ACacc1000 = 10;
				ACacc100 = 10;
				Acmeter = 0;
			}
			if (temp_time > 750)
			{
				CVacc = 1;
				Cvmeter = 83;
				CVacc10 = 8;
				CVacc1 = 3;
			}
			if (temp_time > 1000)
				Tp = 1; //三相
			break;
		}
	}

	//交直切換を行う
	void SetACDC(int state)
	{
		step_ACDC = state;
		tmr_ACDC = g_time;
	}

	//交直切換のお知らせ
	void AlartACDC(int state)
	{
		alert_ACDC = state;
	}
};