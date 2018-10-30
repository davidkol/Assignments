#include <avr/io.h>
#include "lcd.h"
#include "avr.h"

char input[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//char modinput[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int i;

struct morseLetter
{
	// values used to play fixed frequency for needed times
	char * repr;
} morseLetter;

int pause;

struct morseLetter letters[26];
struct morseLetter nums[10];
int encrypted = 0;

char * numDotsDashes[10] = {"22222","12222","11222","11122","11112","11111","21111","22111","22211","22221"};

char * alphaDotsDashes[26] = {"12","2111","2121","211","1","1121","221","1111","11","1222","212","1211","22",
"21","222","1221","2212","121","111","2","112","1112","122","2112","2122","2211"};

void wait2(unsigned short msec)
{
	TCCR0 = 2;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 8) * 0.00001);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

void make_noise(int t, int k)
{
	int i;
	for (i = 0; i < k; i++)
	{
		SET_BIT(PORTA, 0);
		wait2(t);
		CLR_BIT(PORTA, 0);
		wait2(t);
	}
}

void playdot()
{
	make_noise(114, 110);
	wait_avr(100);
}

void playdash()
{
	make_noise(114, 220);
	wait_avr(100);
}

void playChar(struct morseLetter m)
{
	int j;
	for (j = 0; m.repr[j]; ++j)
	{
		if (m.repr[j] == '1')
			playdot();
		else
			playdash();
	}
}

void playmessage()
{
	int j;
	for (j = 0; input[j]; ++j)
	{
		if (input[j] >= '0' && input[j] <= '9')
		{
			playChar(nums[(int)input[j] - 48]);
		}
		if (input[j] >= 'A' && input[j] <= 'Z')
		{
			playChar(letters[(int)input[j]-65]);
		}
		wait_avr(500);
	}
}

void checkforinput();
int is_pressed(int r, int c);
int get_key();
int b_get_key();

int main(void)
{
	TCCR1B |= (1 << CS10); // Set up timer
	SET_BIT(DDRA, 0);
	//int j;
	for (i = 0; i < 10; ++i)
	{
		nums[i].repr = numDotsDashes[i];
	}
	for (i = 0; i < 26; ++i)
	{
		letters[i].repr = alphaDotsDashes[i];
	}
	i = 0;
	ini_lcd();
	clr_lcd();
	
	while (1)
	{
		checkforinput();
		
	}
}

void checkforinput()
{
	int k = b_get_key();
	switch (k){
		case 1:
			input[i] = '1';
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 2:
			if (input[i] < 'A' || input[i] > 'C' || input[i] == '2') input[i] = 'A';
			else if (input[i] == 'C') input[i] = '2';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 3:
			if (input[i] < 'D' || input[i] > 'F' || input[i] == '3') input[i] = 'D';
			else if (input[i] == 'F') input[i] = '3';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 5:
			if (input[i] < 'G' || input[i] > 'I' || input[i] == '4') input[i] = 'G';
			else if (input[i] == 'I') input[i] = '4';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 6:
			if (input[i] < 'J' || input[i] > 'L' || input[i] == '5') input[i] = 'J';
			else if (input[i] == 'L') input[i] = '5';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 7:
			if (input[i] < 'M' || input[i] > 'O' || input[i] == '6') input[i] = 'M';
			else if (input[i] == 'O') input[i] = '6';			
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 9:
			if (input[i] < 'P' || input[i] > 'S' || input[i] == '7') input[i] = 'P';
			else if (input[i] == 'S') input[i] = '7';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 10:
			if (input[i] < 'T' || input[i] == '8') input[i] = 'T';
			else if (input[i] >= 'V') input[i] = '8';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 11:
			if (input[i] < 'W' || input[i] > 'Z' || input[i] == '9') input[i] = 'W';
			else if (input[i] == 'Z') input[i] = '9';
			else input[i]++;
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 14:
			input[i] = '0';
			pos_lcd(0,i);
			put_lcd(input[i]);
		break;
		case 13:
			i++;
		break;
		case 4:
			if (encrypted == 1)
			{
				modplay();
			}
			else
			{
				playmessage();
			}
		break;
		case 8:
			clr_lcd();
			for (i = 0; input[i]; i++) input[i] = 0;
			i = 0;
		break;
		case 12:
			encryptCC();
		break;
		case 15:
		encryptOTP();
		break;
		case 16:
			encryptRC4();
		break;
	}
}
void modput()
{
	char data[16];
	int j;
	for (j = 0; j < i; j++)
	{
		data[j] = (input[j] % 26) + 65;
		pos_lcd(1, j);
		put_lcd(data[j]);
	}
}

void modplay()
{
	char data[16];
	int j;
	for (j = 0; j < i; j++)
	{
		data[j] = (input[j] % 26) + 65;
		pos_lcd(1, j);
		put_lcd(data[j]);
	}
	for (j = 0; data[j]; ++j)
	{
		if (data[j] >= '0' && data[j] <= '9')
		{
			playChar(nums[data[j] - 48]);
		}
		if (data[j] >= 'A' && data[j] <= 'Z')
		{
			playChar(letters[data[j]-65]);
		}
		wait_avr(500);
	}
	
}

int getnumx()
{
	int k, j = 0; 
	while (1)
	{
		k = b_get_key();
		switch (k)
		{
			case 1:
			case 2:
			case 3:
				return k+j;
			case 5:
			case 6:
			case 7:
				return k-1 + j;
			case 9:
			case 10:
			case 11:
				return k-2 + j;
			case 14:
				return 0;
			case 13:
				j = b_get_key();
				switch (j)
				{
					case 1:
					case 2:
					case 3:
						j =	j*10;
						break;
					case 5:
					case 6:
					case 7:
						j = (j-1)*10;
						break;
					case 9:
					case 10:
					case 11:
						j = (j-2)*10;
						break;
				}
			break;
		}
	}
}

int getnum()
{
	int k, j = 0;
	int l = 0;
	int inp[2] = {0,0};
	while (l < 2)
	{
		k = b_get_key();
		switch (k)
		{
			case 1:
			case 2:
			case 3:
				inp[l++] = k;
				break;
			case 5:
			case 6:
			case 7:
				inp[l++] = k - 1;
				break;
			case 9:
			case 10:
			case 11:
				inp[l++] = k - 2;
				break;
			case 13:
				l = 3;
				break;
		}
	}
	if (l == 1 || l == 3)
		return inp[0];
	else if (l == 2)
	{
		return inp[0] * 10 + inp[1];
	}
	else // should never happen
		printf("ERROR\n");
}

void encryptOTP()
{
	char * key = "0101010101010101";
	int j;
	for (j = 0; j< i; j++)
	{
		input[j] ^= key[j];
	}
	pos_lcd(1,0);
	if (encrypted == 0)
	{
		encrypted = 1;
		modput();
	}
	else
	{
		encrypted = 0;
		puts_lcd2(input);
	}
}

void encryptCC()
{
	int j, k = getnum();	
	for (j = 0; j < i; j++)
	{
		input[j] = (input[j] + k) % 26 + 65;
	}
	pos_lcd(1,0);
	if (encrypted == 0)
	{
		encrypted = 1;
		modput();
	}
	else
	{
		encrypted = 0;
		puts_lcd2(input);
	}
}

void encryptRC4()
{
	char * state = "0101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101";
	int x, y;
	int sx, sy;
	sx = sy = 0;
	x = y = 0;
	int j;
	for (j = 0; j < i; j++)
	{
		sx = state[ ++x];
		y += sx;
		sy = state[ y ];
		input[j] ^= state[ ( sx+sy )];
	}
	pos_lcd(1,0);
	if (encrypted == 0) 
	{
		encrypted = 1;
		modput();
	}
	else 
	{
		encrypted = 0;
		puts_lcd2(input);
	}
}

int is_pressed(int r, int c)
{
	DDRC = 0;
	PORTC = 0;
	SET_BIT(PORTC, c+4);
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	if (GET_BIT(PINC, c+4))
		{
			return 0;
		}
	else
		{
			return 1;
		}

}

int get_key()
{
	int r,c;
	for (r = 0; r < 4; ++r)
	{
		for (c = 0; c < 4; ++c)
		{
			if (is_pressed(r,c))
			{
				return r*4 + c + 1;
			}
		}
	}
	return 0;
}

int b_get_key()
{
	int k = get_key();
	if (k == 0) return 0;
	while (k == get_key());
	return k;
}



/*
// to play letter
// play fixed frequency for morseRepr[0||1]*time unit, pause for 1/4 time unit
// for letter in message
//     char%48 for numbers
//     char%65 for letters

// read in message
// old school phone method
// press # to advance to letter (as opposed to waiting)
// press A to “send”

void morseSwitch(char c)
{
	if (c >= '0' && c <= '9')
	{
		// play nums[(int)c%48]
	}
	else if (c >= 'A' && c <= 'Z')
	{
		// play letters[(int)c%65]
	}
	else
	return;
	// or play longer silence?
}

char * getMessage()
{
	char message[16];

	// your logic
	
	playMessage(message);
}

void playMessage(char* message)
{
	// TODO
}

int main()
{
	int i, j;
	for (i = 0; i < 10; ++i)
	{
		nums[i].repr = numDotsDashes[i];
	}
	for (i = 0; i < 26; ++i)
	{
		letters[i].repr = alphaDotsDashes[i];
	}
	while (1)
	{
		getMessage();
		playMessage();
	}
	return 0;
}
*/
