#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include <string>  
#include <fstream>
#include <sstream>
#define BIT_SIZE 32
#define FRAME 512
#define TOTAL_FRAMES 1024
#define MAX_SIZE FRAME*TOTAL_FRAMES // 512 * 1024 = 524888
#define BITTEST 0
using namespace std;



int PM[MAX_SIZE], BM[BIT_SIZE], MASK0[BIT_SIZE], MASK1[BIT_SIZE], TLB[4][3] = { { 0, -1, -1 }, {1, -1, -1}, {2, -1, -1}, {3, -1, -1} }, bits[32], test = 0;
string output; 

void CreateMasks();
int FindEmptyFrames(int amount);
void initSeg(int s, int f);
void initPage(int p, int s, int f);
int ReadorWrite(unsigned o, unsigned VA);
int FindInTLB(int sp, int w);
void UpdateTLBLRU(int i);
void UpdateTLBEntry(int sp, int s, int p);
void DecToBinary(unsigned int dec);
void reset();

int main()
{
	for (int i = 0; i < 2; i++)
	{
		test = i;

		if (!test) cout << "TLB OFF, FIRST RUN: \n";
		else cout << "TLB OFF, SECOND RUN: \n";
		//cin >> test;
		CreateMasks();
		BM[0] = BM[0] | MASK1[0];
		if (BITTEST) DecToBinary(BM[0]);

		int s, f, p, o, VA, test;
		string input, tmp;
		ifstream inFile;
		string x;// = "input1.txt";
		cout << "enter input1: \n";
		cin >> x;
		inFile.open(x);
		if (!inFile) {
			cout << "Unable to open file";
			exit(1); // terminate with error
		}

		getline(inFile, input);
		stringstream ss(input);
		while (getline(ss, tmp, ' '))
		{
			s = stoi(tmp);
			getline(ss, tmp, ' ');
			f = stoi(tmp);
			initSeg(s, f);
		}
		getline(inFile, input);
		ss.str(input);
		ss.clear();
		while (getline(ss, tmp, ' '))
		{
			p = stoi(tmp);
			getline(ss, tmp, ' ');
			s = stoi(tmp);
			getline(ss, tmp, ' ');
			f = stoi(tmp);
			initPage(p, s, f);
		}

		inFile.close();

		//x = "input2.txt";
		cout << "enter input2: \n";
		cin >> x;
		inFile.open(x);
		if (!inFile) {
			cout << "Unable to open file";
			exit(1); // terminate with error
		}

		getline(inFile, input);
		ss.str(input);
		ss.clear();
		while (getline(ss, tmp, ' '))
		{
			o = stoi(tmp);
			getline(ss, tmp, ' ');
			VA = stoi(tmp);
			test = ReadorWrite(o, VA);
			if (test == 0) output.append("err ");
			else if (test == -1) output.append("pf ");
			else output.append(to_string(test) + " ");
		}
		cout << "enter output " << i + 1<< " name (REMEMBER TLB): \n";
		cin >> x;
		//x.append("49938855.txt");
		ofstream outfile{ x };
		outfile << output;
		outfile.close();
		reset();
	}
	
	return 0;
}

void reset()
{
	int BMt[BIT_SIZE], MASK0t[BIT_SIZE], MASK1t[BIT_SIZE], TLBt[4][3] = { { 0, -1, -1 },{ 1, -1, -1 },{ 2, -1, -1 },{ 3, -1, -1 } }, bitst[32];
	string outputt;

	for (int i = 0; i < MAX_SIZE; i++) PM[i] = 0;
	swap(BM, BMt);
	swap(MASK0, MASK0t);
	swap(MASK1, MASK1t);
	swap(TLB, TLBt) ;
	swap(bits, bitst);
	swap(output, outputt);
}

void UpdateTLBEntry(int sp, int s, int p)
{
	for (int j = 0; j < 4; j++)
	{
		if (TLB[j][0] == 0)
		{
			TLB[j][1] = sp;
			TLB[j][2] = PM[PM[s] + p];
			UpdateTLBLRU(j);
			return;
		}
	}
}

void UpdateTLBLRU(int i)
{
	int temp = TLB[i][0];
	TLB[i][0] = 3;
	for (int j = 0; j < 4; j++)
	{
		if (i != j && TLB[j][0] > temp)
		{
			TLB[j][0]--;
		}
	}
}

int FindInTLB(int sp, int w)
{
	for (int i = 0; i < 4; i++)
	{
		if (TLB[i][1] == sp)
		{
			UpdateTLBLRU(i);
			return TLB[i][2] + w;
		}
	}
	return -1;
}

int ReadorWrite(unsigned o, unsigned VA)
{
	if (BITTEST) DecToBinary(BM[0]);
	unsigned s, p, w, sp, f;
	VA = VA << 4;
	sp = VA >> 13;
	s = VA >> 23;
	VA = VA << 9;
	p = VA >> 22;
	VA = VA << 10;
	w = VA >> 23;

	if (test)
	{
		f = FindInTLB(sp, w);
		if (f != -1)
		{
			output.append("h ");
			return f;
		}
		output.append("m ");
	}
	if (o == 0)
	{
		if (PM[s] == -1 || PM[PM[s] + p] == -1) return -1; //handle error at caller
		if (PM[s] == 0 || PM[PM[s] + p] == 0) return 0;
		UpdateTLBEntry(sp, s, p);
		return PM[PM[s] + p] + w;
	}
	else if (o == 1)
	{
		if (PM[s] == -1 || PM[PM[s] + p] == -1) return -1;
		if (PM[s] == 0)
		{
			int f = FindEmptyFrames(2);
			PM[s] = f;
			BM[f / FRAME / BIT_SIZE] = BM[f / FRAME / BIT_SIZE] | MASK1[f / FRAME] | MASK1[(f / FRAME) + 1];
			if (BITTEST) DecToBinary(BM[0]);
		}
		if (PM[PM[s] + p] == 0)
		{
			int f = FindEmptyFrames(1);
			PM[PM[s] + p] = f;
			BM[f / FRAME / BIT_SIZE] = BM[f / FRAME / BIT_SIZE] | MASK1[f / FRAME];
			if (BITTEST) DecToBinary(BM[0]);

		}
		UpdateTLBEntry(sp, s, p);
		return PM[PM[s] + p] + w;
	}
}

void initPage(int p, int s, int f)
{
//	int test = BM[f / FRAME / BIT_SIZE] & MASK1[f / FRAME];
//	if (test == 0)
	{
		if (s >= 0 && p >= 0) PM[PM[s] + p] = f;
		else cout << "error";
		if (f > 0) BM[f / FRAME / BIT_SIZE] = BM[f / FRAME / BIT_SIZE] | MASK1[f / FRAME];
	}
//	else cout << "Page already mapped?";
}

void initSeg(int s, int f)
{
//	int test = BM[f / FRAME / BIT_SIZE] & (MASK1[f / FRAME] | MASK1[(f / FRAME) + 1]);
//	if (test == 0)
	{
		if (s >= 0) PM[s] = f;
		else cout << "error in s";
		if (f > 0) BM[f / FRAME / BIT_SIZE] = BM[f / FRAME / BIT_SIZE] | MASK1[f / FRAME] | MASK1[(f / FRAME) + 1];
		//else cout << "f";
	}
//	else cout << "Seg already mapped?";
}

int FindEmptyFrames(int amount)
{
	int test;
	switch (amount) {
	case 1:
		for (int i = 0; i < BIT_SIZE; i++)
		{
			for (int j = 0; j < BIT_SIZE; j++)
			{
				test = BM[i] & MASK1[j];
				if (test == 0)
				{
					return (i*BIT_SIZE + j) * FRAME;
				}
			}
		}
		break;
	case 2:
		for (int i = 0; i < BIT_SIZE; i++)
		{
			for (int j = 0; j < BIT_SIZE; j++)
			{
				test = BM[i] & MASK1[j];
				if (test == 0)
				{
					test = BM[i] & MASK1[j + 1];
					if (test == 0)
					{
						return (i*BIT_SIZE + j) * FRAME;
					}
				}
			}
		}
	}
	return -1;
}

void CreateMasks()
{
	MASK1[BIT_SIZE - 1] = 1;
	MASK0[BIT_SIZE - 1] = ~1;
	for (int i = BIT_SIZE - 2; i >= 0; i--)
	{
		MASK1[i] = MASK1[i + 1] << 1;
		MASK0[i] = ~MASK0[i];
	}
}

void DecToBinary(unsigned int n)
{
	int c, k, i = 0;
	for (c = 31; c >= 0; c--, i++)
	{
		k = n >> c;

		if (k & 1)
			bits[i] = 1;
		else
			bits[i] = 0;
	}
}
