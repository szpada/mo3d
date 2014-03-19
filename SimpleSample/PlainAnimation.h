#pragma once

class CPlainAnimation
{

public:

	CPlainAnimation()
	{

	}

	~CPlainAnimation(void)
	{
	}

	double GetY(double _t)
	{
		if(_t < 0.5)
			return 1.0 - 2.0*_t;
		else
			return 0.0;
	}

	double GetX(double _t)
	{
		double v=0.0;
		if(_t < 0.7)
		{
			v = (0.8/0.7);
			return v*_t;
		}
		else if(_t <= 1.0)
		{
			v = (0.8/0.7) - ((_t-0.7)*3.33)*((0.8/0.7)-1.0);
			return v*_t;
		}
		else
			return 1.0;

		/*if(_t < 0.9)
		{
			v = (0.95/0.9);
			return v*_t;
		}
		else if(_t <= 1.0)
		{
			v = (0.95/0.9)- ((_t-0.9)*10.0)*((0.95/0.9)-1.0);
			return v*_t;
		}
		else
			return 1.0;*/
	}

	double GetFi(double _t)
	{
		if(_t <= 0.65)
			return 40.0*(1.0-exp((_t-0.65)/0.2));
		else
			return 0.0;
	}
};

