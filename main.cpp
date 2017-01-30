#include <stdio.h>
#include <iostream>
using namespace std;

#include <string>


//Mini calc + - * /

int main()
{
	float a, b, res;
	bool correct_operands = false;
	int check1 = 0;
	int check2 = 0;

	while(!correct_operands)
	{
		a = 0;
		b = 0;
		cout << "First Operand: ";
		cin >> a;
		if (!cin.fail())
			check1 = 1;
		else
			check1 = 0;
		cout << "Second Operand: ";
		cin >> b;
		if (!cin.fail())
			check2 = 1;
		else
			check2 = 0;
		if (check2 == 1 && check1 == 1)
		{
			correct_operands = true;
			cout << "Correct Values Entered\n";
		}
		else
			cout << "Incorrect Values Entered\n";
		cin.clear();
	}

	cout << "Exited While";

	system("pause");
	return 0;
}