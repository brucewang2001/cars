#ifndef WIEGAND_H_
#define WIEGAND_H_

class WIEGAND {

public:
	WIEGAND();
	void begin();
	bool available();
	unsigned long getCode();
	int getWiegandType();
    static int _pinD0;
    static int _pinD1;
	
private:
	static void ReadD0();
	static void ReadD1();
	static bool DoWiegandConversion ();
	static unsigned long GetCardId (unsigned long *codehigh, unsigned long *codelow, char bitlength);
	
	static unsigned long 	_cardTempHigh;
	static unsigned long 	_cardTemp;
	static unsigned long 	_lastWiegand;
	static unsigned long 	_sysTick;
	static int				_bitCount;	
	static int				_wiegandType;
	static unsigned long	_code;
};

#endif