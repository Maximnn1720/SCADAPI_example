#include "stdafx.h"

#include "\WinDDK\Include\SCADAPIX.hxx"//"\Users\maxim\source\repos\API\Include\SCADAPIX.hxx"
#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <vector>
#include <fstream>
#include <math.h>

using namespace std;

#pragma comment(lib, "SCADAPIX.lib" )

#pragma region Объявление переменных
enum DistrictWind { W_Ia, W_I, W_II, W_III, W_IV, W_V, W_VI, W_VII};
enum DistrictSnow { S_I = 1, S_II, S_III, S_IV, S_V, S_VI };
enum TypeArea { A, B, C };

//ГЕОМЕТРИЯ
double H_FERM, L_FERM, RAD, d_RAM, d_PRG, d_KF, Hk, HR = 4.0;
int COUNT_PANELS, COUNT_FAHVERK = 2, Type;
int RM_COUNT = 1;
int Runs_Multiplier = 2;
string Kw;

//НАГРУЗКИ
double Q[4];
DistrictWind DisWind;
DistrictSnow DisSnow;
TypeArea TArea;
#pragma endregion

#pragma region Функции


template <typename T>	std::string to_string(const T &t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

double c(double X, string direction) {
	double c_, e;
	double H = Hk;
	double L = d_RAM * (RM_COUNT - 1);
	double B = L_FERM;

	if (direction == "D") {
		c_ = 0.8;
	}
	else if (direction == "E") {
		c_ = 0.5;
	}
	else if (direction == "ПОПЕРЁК") {
		if (2 * H >= L) {
			e = L;
		}
		else {
			e = 2 * H;
		}

			if ((X <= e / 5)) c_ = 1;
			else if ((X > e / 5) && (X <= e)) c_ = 0.8;
			else if (X > e) c_ = 0.5;		
	}
	else if (direction == "ВДОЛЬ") {
		if (2 * H >= B) {
			e = B;
		}
		else {
			e = 2 * H;
		}

		if ((X <= e / 5)) c_ = 1;
		else if ((X > e / 5) && (X <= e)) c_ = 0.8;
		else if (X > e) c_ = 0.5;
	}
	else c_ = 0;
	
	return c_;

}

double cs(double X, string direction, double strip) {
	double cs_, e;
	double H = Hk;
	double L = d_RAM * (RM_COUNT - 1);
	double B = L_FERM;

	if (direction == "D") {
		cs_ = c(X, direction);
	}
	else if (direction == "E") {
		cs_ = c(X, direction);
	}
	else if (direction == "ПОПЕРЁК") {
		if (2 * H >= L) {
			e = L;
		}
		else {
			e = 2 * H;
		}

		if ((c(X - strip / 2, direction) == c(X + strip / 2, direction)) || (c(X - strip / 2, direction) == 0) || (c(X + strip / 2, direction) == 0)) cs_ = c(X, direction);
		else if (abs(X - e / 5) < abs(X - e)) {
			cs_ = (c(X - strip / 2, direction)*(e / 5 - (X - strip / 2)) + c(X + strip / 2, direction)*(-e / 5 + (X + strip / 2))) / strip;
		}
		else if (abs(X - e / 5) > abs(X - e)) {
			cs_ = (c(X - strip / 2, direction)*(e - (X - strip / 2)) + c(X + strip / 2, direction)*(-e + (X + strip / 2))) / strip;
		}
		else cs_ = 0;
		
	}
	else if (direction == "ВДОЛЬ") {
		if (2 * H >= B) {
			e = B;
		}
		else {
			e = 2 * H;
		}
		if (c(X - strip / 2, direction) == c(X + strip / 2, direction)) cs_ = c(X, direction);
		else if (abs(X - e / 5) < abs(X - e)) {
			cs_ = (c(X - strip / 2, direction)*(e / 5 - (X - strip / 2)) + c(X + strip / 2, direction)*(-e / 5 + (X + strip / 2))) / strip;
		}
		else if (abs(X - e / 5) > abs(X - e)) {
			cs_ = (c(X - strip / 2, direction)*(e - (X - strip / 2)) + c(X + strip / 2, direction)*(-e + (X + strip / 2))) / strip;
		}
		else cs_ = 0;
		
	}
	else cs_ = 0;

	return cs_;

}

double k(double Ze, TypeArea AreaType) {
	double alfa, k10, ζ10;
	
	switch (AreaType)
	{
	case (A):
		alfa = 0.15;
		k10 = 1.0;
		ζ10 = 0.76;
		break;
	case (B):
		alfa = 0.20;
		k10 = 0.65;
		ζ10 = 1.06;
		break;
	case (C):
		alfa = 0.25;
		k10 = 0.4;
		ζ10 = 1.78;
		break;
	}
	if (Ze < 5) Ze = 5;
	return k10 * pow((Ze / 10), (2 * alfa));
	
}

double ζ(double Ze, TypeArea AreaType) {
	double alfa, k10, ζ10;

	switch (AreaType)
	{
	case (A):
		alfa = 0.15;
		k10 = 1.0;
		ζ10 = 0.76;
		break;
	case (B):
		alfa = 0.20;
		k10 = 0.65;
		ζ10 = 1.06;
		break;
	case (C):
		alfa = 0.25;
		k10 = 0.4;
		ζ10 = 1.78;
		break;
	}
	if (Ze < 5) Ze = 5;
	return ζ10 * pow((Ze / 10), (-alfa));
	
}

double v(double h, double b) {
	int i = 0, j = 0;
	double v_;

	double ϰ[7] = { 5.00, 10.00, 20.00, 40.00, 80.00, 160.00, 350.00 };
	
	double ρ[7] = { 0.10, 5.00, 10.00, 20.00, 40.00, 80.00, 160.00 };
	
	double table[7][7] = { { 0.95, 0.92, 0.88, 0.83, 0.76, 0.67, 0.56 }, 
						   { 0.89, 0.87, 0.84, 0.80, 0.73, 0.65, 0.54 },
						   { 0.85, 0.84, 0.81, 0.77, 0.71, 0.64, 0.53 },
						   { 0.80, 0.78, 0.76, 0.73, 0.68, 0.61, 0.51 },
						   { 0.72, 0.72, 0.70, 0.67, 0.63, 0.57, 0.48 },
						   { 0.63, 0.63, 0.61, 0.59, 0.56, 0.51, 0.44 },
						   { 0.53, 0.53, 0.52, 0.50, 0.47, 0.44, 0.38 } };

	while (ϰ[i + 1] <= h) {
		i++;
	} 
	while (ρ[j + 1] <= b) {
		j++;
	}

	if (h = 0 ) v_ = 0;
	else v_ = table[j][i];

	return v_;

}

double SnowLoad(DistrictSnow Dis) {
	double Sg;
	switch (Dis)
	{
	case (S_I):
		Sg = 0.8;
		break;
	case (S_II):
		Sg = 1.2;
		break;
	case (S_III):
		Sg = 1.8;
		break;
	case (S_IV):
		Sg = 2.4;
		break;
	case (S_V):
		Sg = 3.2;
		break;
	case (S_VI):
		Sg = 4.0;
		break;
	}
	return Sg;
}

double WindLoad(double Ze, DistrictWind Dis, TypeArea AreaType, double X, string direction, double strip, double H) {
	
	double W0, Yf, Wm, Wp;

	Yf = 1.4;

	switch (Dis)
	{
	case (W_Ia):    
		W0 = 0.17;
		break;
	case (W_I):
		W0 = 0.23;
		break;
	case (W_II):
		W0 = 0.30;
		break;
	case (W_III):
		W0 = 0.38;
		break;
	case (W_IV):
		W0 = 0.48;
		break;
	case (W_V):
		W0 = 0.60;
		break;
	case (W_VI):
		W0 = 0.73;
		break;
	case (W_VII):
		W0 = 0.85;
		break;
	}

	Wm = W0 * k(Ze, AreaType) * cs(X, direction, strip);
	Wp = Wm * ζ(Ze, AreaType) * v(H, strip);
	return (Wm + Wp) * Yf;

}
#pragma endregion

void _tmain()
{
	ScadAPI	SCAD_MODEL(NULL);
	UINT n, i, Node[4];

	const static UnitsAPI Un[3] = { { "m", 1 },{ "cm", 100 },{ "кН", 9.81 } };

	class ELEMENTS {
	public:
		int NOTES_COUNTS;
		vector<int> NOTES_NUMBER;
		int ELEMENTS_COUNTS;
		vector<int> ELEMENTS_NUMBER;

		ELEMENTS()
		{
			NOTES_COUNTS = 0;
			ELEMENTS_COUNTS = 0;
		}

		void set_NOTES_NUMBER(int ELEMENT_NOTES_NUMBER)
		{
			NOTES_COUNTS++;
			NOTES_NUMBER.insert(NOTES_NUMBER.end(), ELEMENT_NOTES_NUMBER);
		}
		void set_ELEMENTS_NUMBER(int ELEMENT_NOTES_NUMBER)
		{
			ELEMENTS_COUNTS++;
			ELEMENTS_NUMBER.insert(ELEMENTS_NUMBER.end(), ELEMENT_NOTES_NUMBER);
		}
	};

	class RIGIN_ELEMENTS {
	public:
		int TYPE_RIGIN_COUNTS;
		vector <vector <int>> RIGIN_ELEMENTS_NUMBER ;

		RIGIN_ELEMENTS(int COUNT_RIGIN)
		{
			TYPE_RIGIN_COUNTS = COUNT_RIGIN;
			RIGIN_ELEMENTS_NUMBER.resize(1 + TYPE_RIGIN_COUNTS);
		}

		void SET_ELEMENTS_NUMBER(int TYPE_RIGIN, int ELEMENT_NUMBER)
		{
			RIGIN_ELEMENTS_NUMBER[TYPE_RIGIN].push_back(ELEMENT_NUMBER);
		}

		void RIGIN_INSERT(ScadAPI SCAD_MODEL, int TYPE_RIGIN, LPCSTR NAME_RIGIN)
		{
			UINT *Elem = new UINT[RIGIN_ELEMENTS_NUMBER[TYPE_RIGIN].size()];
			for (int i = 0; i < RIGIN_ELEMENTS_NUMBER[TYPE_RIGIN].size(); i++) 
			{
				Elem[i] = RIGIN_ELEMENTS_NUMBER[TYPE_RIGIN][i];
			}		
			ApiSetRigidElem(SCAD_MODEL, ApiSetRigid(SCAD_MODEL, "STZ RUSSIAN p_wide_h 18 TMP 1.2e-005"), RIGIN_ELEMENTS_NUMBER[TYPE_RIGIN].size(), Elem);
			ApiSetRigidName(SCAD_MODEL, TYPE_RIGIN, NAME_RIGIN);
			delete[] Elem;
		}
	};

#pragma region Открытие файла и загрузка параметров

	setlocale(LC_ALL, "rus");
	string str;
	char buffer[250];

	cout << "Открываем файл\n";
	ifstream INPUTFILE;
	INPUTFILE.open("PARAMETRS.sys");

	if (INPUTFILE.is_open()) {

		setlocale(LC_ALL, "rus");

		cout << "Файл открыт!\n";
		getline(INPUTFILE, str);
		cout << "Высота фермы, м (H_FERM)\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//H_FERM = atof(buffer);
		H_FERM = 3;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Пролёт фермы, м(L_FERM)\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//L_FERM = atof(buffer);
		L_FERM = 12;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Количество панелей фермы, шт (COUNT_PANELS)" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//COUNT_PANELS = atoi(buffer);
		COUNT_PANELS = 3;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Угол наклона кровли, % (RAD)\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//RAD = atof(buffer);
		RAD = 0.2;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Шаг рам, м (d_RAM)\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//d_RAM = atof(buffer);
		d_RAM = 6;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Высота колонны, м (Hk)\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//Hk = atof(buffer);
		Hk = 6;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Количество пролётов, м\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//RM_COUNT = atof(buffer);
		RM_COUNT = 10;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Полезная нагрузка на прогоны, кН/м2 (Q[1])\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		Q[1] = atof(buffer);
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Полезная нагрузка на НП, кН/м2 (Q[2])\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		Q[2] = atof(buffer);
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Полезная нагрузка на колонны, кН/м2 (Q[3])\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		Q[3] = atof(buffer);
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Снеговой район\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		DisSnow = static_cast<DistrictSnow>(atoi(buffer));
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Ветровой район\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		DisWind = static_cast<DistrictWind>(atoi(buffer));
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Тип Местности\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		TArea = static_cast<TypeArea>(atoi(buffer));
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Множитель прогонов\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		Runs_Multiplier = atoi(buffer);
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Шаг колонн фахверка\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		//COUNT_FAHVERK = atoi(buffer);
		COUNT_FAHVERK = 2;
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Отметка распорки, м\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		HR = atof(buffer);
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Тип\n" << endl;

		getline(INPUTFILE, str);
		strcpy(buffer, str.c_str());
		Type = atoi(buffer);
		cout << str << endl;

		getline(INPUTFILE, str);
		cout << "Коэффциент жесткости\n" << endl;

		getline(INPUTFILE, str);
		Kw = str;
		cout << str << endl;
	}
	else {
		cout << "Не удалось открыть файл\n";
		system("pause");
		exit(0);
	}


	INPUTFILE.close();
#pragma endregion

	ELEMENTS **ELM = new ELEMENTS*[RM_COUNT];
	for (i = 1; i <= RM_COUNT; i++) {
		ELM[i] = new ELEMENTS[16];
	}

	ELEMENTS **KF_ELM = new ELEMENTS*[2];
	for (i = 1; i <= 2; i++) {
		KF_ELM[i] = new ELEMENTS[((COUNT_PANELS * 2) / COUNT_FAHVERK) + 100];
	}

	RIGIN_ELEMENTS RIGIN_ELM(11);


	if (ApiCreate(&SCAD_MODEL) != APICode_OK) ApiMsg("Error");  //  создание объекта API и контроль
	if (ApiClear(SCAD_MODEL) != APICode_OK) ApiMsg("Error");    //  после открытия можно не делать
	if (ApiSetLanguage(SCAD_MODEL, 1) != APICode_OK) ApiMsg("Error");
	ApiSetName(SCAD_MODEL, "TestNewProject");
	ApiSetUnits(SCAD_MODEL, Un);
	if (ApiSetTypeSchema(SCAD_MODEL, 5) != APICode_OK) ApiMsg("Error");

	/*
	ELM[j][1]    - Верхний пояс
	ELM[j][2]    - Нижний пояс
	ELM[j][3]    - Решётка
	ELM[j][4]    - Колонны лево
	ELM[j][5]    - Колонны право
	ELM[j][6]    - Прогоны
	ELM[j][7]    - Связи по кровле
	ELM[j][8]    - Распорка лево
	ELM[j][9]    - Распорка право
	ELM[j][10]   - Распорка фахверка
	ELM[j][11]   - Распорка нижний пояс
	ELM[j][12]   - Распорка парная нижний пояс лево
	ELM[j][13]   - Распорка парная нижний пояс право
	ELM[j][14]   - Связи по рядовым колоннам
	ELM[j][15]   - Ветровые связи
	KF_ELM[k][n] - Колонны фахверка
	*/

#pragma region Узлы
	double x, y, z;
	int ApiNode[4];
	int COUNT_FERM;
	int j = 1;
	
	if (Type == 2) {
		for (int k = 1; k <= 2; k++)
		{
			ApiNode[1] = ApiNodeAddOne(SCAD_MODEL, 0, (j - 1) * d_RAM, Hk);
			ApiNode[2] = ApiNodeAddOne(SCAD_MODEL, L_FERM, (j - 1) * d_RAM, Hk);
			ApiNode[3] = ApiNodeAddOne(SCAD_MODEL, L_FERM / 2, (j - 1) * d_RAM, L_FERM / 2 * RAD + Hk);

			//ВП
			ELM[j][1].set_NOTES_NUMBER(ApiNode[1]);

			for (i = 1; i <= (COUNT_PANELS*Runs_Multiplier) - 1; i++) {
				x = ApiGetNode(SCAD_MODEL, ApiNode[1])->x + i * ((ApiGetNode(SCAD_MODEL, ApiNode[3])->x - ApiGetNode(SCAD_MODEL, ApiNode[1])->x) / (COUNT_PANELS*Runs_Multiplier));
				z = ApiGetNode(SCAD_MODEL, ApiNode[1])->z + i * ((ApiGetNode(SCAD_MODEL, ApiNode[3])->z - ApiGetNode(SCAD_MODEL, ApiNode[1])->z) / (COUNT_PANELS*Runs_Multiplier));
				ApiNodeAddOne(SCAD_MODEL, x, (j - 1) * d_RAM, z);
				ELM[j][1].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
			}

			ELM[j][1].set_NOTES_NUMBER(ApiNode[3]);

			for (i = 1; i <= (COUNT_PANELS*Runs_Multiplier) - 1; i++) {
				x = ApiGetNode(SCAD_MODEL, ApiNode[3])->x + i * ((ApiGetNode(SCAD_MODEL, ApiNode[2])->x - ApiGetNode(SCAD_MODEL, ApiNode[3])->x) / (COUNT_PANELS*Runs_Multiplier));
				z = ApiGetNode(SCAD_MODEL, ApiNode[3])->z + i * ((ApiGetNode(SCAD_MODEL, ApiNode[2])->z - ApiGetNode(SCAD_MODEL, ApiNode[3])->z) / (COUNT_PANELS*Runs_Multiplier));
				ApiNodeAddOne(SCAD_MODEL, x, (j - 1) * d_RAM, z);
				ELM[j][1].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
			}

			ELM[j][1].set_NOTES_NUMBER(ApiNode[2]);


			//Колонны фахверка

			for (int m = 0, n = 0; m <= (COUNT_PANELS * Runs_Multiplier * 2); m = m + COUNT_FAHVERK * Runs_Multiplier, n++) {
				i = 0;
				while (i < Hk + (ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[m])->z - ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[0])->z)) {
					KF_ELM[k][n].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[m])->x, (j - 1) * d_RAM, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[0])->z - Hk + i));
					if ((i + 1 > HR) && (i < HR)) {
						ELM[j][10].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[m])->x, (j - 1) * d_RAM, HR));
						KF_ELM[k][n].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
					}
					if (i == HR) {
						ELM[j][10].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
					}
					i++;
				}
				KF_ELM[k][n].set_NOTES_NUMBER(ELM[j][1].NOTES_NUMBER[m]);
			}
			j = RM_COUNT;
		}
	}
	
	if (Type != 3) {

		if (Type == 2) {
			j = 2;
		}
		else {
			j = 1;
			RM_COUNT = 2;
		}

		for (j; j <= RM_COUNT - 1; j++)
		{
			ApiNode[1] = ApiNodeAddOne(SCAD_MODEL, 0, (j - 1) * d_RAM, Hk);
			ApiNode[2] = ApiNodeAddOne(SCAD_MODEL, L_FERM, (j - 1) * d_RAM, Hk);
			ApiNode[3] = ApiNodeAddOne(SCAD_MODEL, L_FERM / 2, (j - 1) * d_RAM, L_FERM / 2 * RAD + Hk);
			ApiNode[4] = ApiNodeAddOne(SCAD_MODEL, L_FERM / 2, (j - 1) * d_RAM, (L_FERM / 2 * RAD) - H_FERM + Hk);


			//ВП
			ELM[j][1].set_NOTES_NUMBER(ApiNode[1]);

			for (i = 1; i <= (COUNT_PANELS*Runs_Multiplier) - 1; i++) {
				x = ApiGetNode(SCAD_MODEL, ApiNode[1])->x + i * ((ApiGetNode(SCAD_MODEL, ApiNode[3])->x - ApiGetNode(SCAD_MODEL, ApiNode[1])->x) / (COUNT_PANELS*Runs_Multiplier));
				z = ApiGetNode(SCAD_MODEL, ApiNode[1])->z + i * ((ApiGetNode(SCAD_MODEL, ApiNode[3])->z - ApiGetNode(SCAD_MODEL, ApiNode[1])->z) / (COUNT_PANELS*Runs_Multiplier));
				ApiNodeAddOne(SCAD_MODEL, x, (j - 1) * d_RAM, z);
				ELM[j][1].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
			}

			ELM[j][1].set_NOTES_NUMBER(ApiNode[3]);

			for (i = 1; i <= (COUNT_PANELS*Runs_Multiplier) - 1; i++) {
				x = ApiGetNode(SCAD_MODEL, ApiNode[3])->x + i * ((ApiGetNode(SCAD_MODEL, ApiNode[2])->x - ApiGetNode(SCAD_MODEL, ApiNode[3])->x) / (COUNT_PANELS*Runs_Multiplier));
				z = ApiGetNode(SCAD_MODEL, ApiNode[3])->z + i * ((ApiGetNode(SCAD_MODEL, ApiNode[2])->z - ApiGetNode(SCAD_MODEL, ApiNode[3])->z) / (COUNT_PANELS*Runs_Multiplier));
				ApiNodeAddOne(SCAD_MODEL, x, (j - 1) * d_RAM, z);
				ELM[j][1].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
			}

			ELM[j][1].set_NOTES_NUMBER(ApiNode[2]);


			//НП
			ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[Runs_Multiplier])->x + (ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[Runs_Multiplier])->z - ApiGetNode(SCAD_MODEL, ApiNode[4])->z)*RAD, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[1])->y, ApiGetNode(SCAD_MODEL, ApiNode[4])->z);
			ELM[j][2].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));

			for (i = 1 + Runs_Multiplier; i < (COUNT_PANELS*Runs_Multiplier); i++) {
				ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[i])->x, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[i])->y, ApiGetNode(SCAD_MODEL, ApiNode[4])->z);
				ELM[j][2].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
			}
			ELM[j][2].set_NOTES_NUMBER(ApiNode[4]);

			for (i = (COUNT_PANELS*Runs_Multiplier) + 1; i <= 2 * (COUNT_PANELS*Runs_Multiplier) - 1 - Runs_Multiplier; i++) {
				ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[i])->x, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[i])->y, ApiGetNode(SCAD_MODEL, ApiNode[4])->z);
				ELM[j][2].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
			}
			ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[2 * (COUNT_PANELS*Runs_Multiplier) - Runs_Multiplier])->x - (ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[2 * (COUNT_PANELS*Runs_Multiplier) - Runs_Multiplier])->z - ApiGetNode(SCAD_MODEL, ApiNode[4])->z)*RAD, ApiGetNode(SCAD_MODEL, ELM[j][1].NOTES_NUMBER[1])->y, ApiGetNode(SCAD_MODEL, ApiNode[4])->z);
			ELM[j][2].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));

			//Распорки по НП
			//if (COUNT_PANELS % 2 != 0) {
				ELM[j][11].set_NOTES_NUMBER(ApiNode[4]);
			/*}
			else {
				ELM[j][12].set_NOTES_NUMBER(ELM[j][2].NOTES_NUMBER[((ELM[j][2].NOTES_COUNTS - 1) - Runs_Multiplier * 2) / 2]);
				ELM[j][13].set_NOTES_NUMBER(ELM[j][2].NOTES_NUMBER[(ELM[j][2].NOTES_COUNTS - 1 - ((ELM[j][2].NOTES_COUNTS - 1) - Runs_Multiplier * 2) / 2)]);
			}*/

			//Колонны
			if (Type != 0) {
				i = 0;
				while (i < Hk) {
					ELM[j][4].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ApiNode[1])->x, (j - 1) * d_RAM, ApiGetNode(SCAD_MODEL, ApiNode[1])->z - Hk + i));
					if (i == HR) {
						ELM[j][8].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
					}
					ELM[j][5].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ApiNode[2])->x, (j - 1) * d_RAM, ApiGetNode(SCAD_MODEL, ApiNode[2])->z - Hk + i));
					if (i == HR) {
						ELM[j][9].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
					}
					if ((i + 1 > HR) && (i < HR)) {
						ELM[j][4].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ApiNode[1])->x, (j - 1) * d_RAM, HR));
						ELM[j][8].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
						ELM[j][5].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ApiNode[2])->x, (j - 1) * d_RAM, HR));
						ELM[j][9].set_NOTES_NUMBER(ApiGetQuantityNode(SCAD_MODEL));
					}
					i++;
				}
				ELM[j][4].set_NOTES_NUMBER(ApiNode[1]);
				ELM[j][5].set_NOTES_NUMBER(ApiNode[2]);
			}

			if (Type == 1) {
				//Ветровые связи
				ELM[j][15].set_NOTES_NUMBER(ELM[j][4].NOTES_NUMBER[ELM[j][4].NOTES_COUNTS - 1]);
				ELM[j][15].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[ELM[j][4].NOTES_COUNTS - 1])->x + 0.2, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[ELM[j][4].NOTES_COUNTS - 1])->y, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[ELM[j][4].NOTES_COUNTS - 1])->z));
				ELM[j][15].set_NOTES_NUMBER(ELM[j][5].NOTES_NUMBER[ELM[j][5].NOTES_COUNTS - 1]);
				ELM[j][15].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[ELM[j][5].NOTES_COUNTS - 1])->x + 0.2, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[ELM[j][5].NOTES_COUNTS - 1])->y, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[ELM[j][5].NOTES_COUNTS - 1])->z));
			}
		}
	}

	if (Type == 3) {
		int X = 0;
		int DELTA = 0;
		j = 1;
		

		while ((RM_COUNT - DELTA) > 3) {
			for (i = 1; i <= RM_COUNT - DELTA; i++)
			{
				//ВП
				ELM[j][1].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, X, (i - 1) * d_RAM, 0));
				//НП
				ELM[j][2].set_NOTES_NUMBER(ApiNodeAddOne(SCAD_MODEL, X + H_FERM, (i - 1) * d_RAM, 0));
			}
			X = X + 3 * H_FERM;
			j++;
			DELTA = DELTA + 2;
		}
		COUNT_FERM = j;

		for (int j = 1; j < COUNT_FERM; j++)
		{
			for (i = 0; i < ELM[j][1].NOTES_COUNTS - 1; i++)
			{
				if (i < (ELM[j][1].NOTES_COUNTS - 1) / 2) {
					ELM[j][3].set_NOTES_NUMBER(ELM[j][1].NOTES_NUMBER[i]);
					ELM[j][3].set_NOTES_NUMBER(ELM[j][2].NOTES_NUMBER[i + 1]);
				}
				else {
					ELM[j][3].set_NOTES_NUMBER(ELM[j][2].NOTES_NUMBER[i]);
					ELM[j][3].set_NOTES_NUMBER(ELM[j][1].NOTES_NUMBER[i + 1]);
				}
			}
		}

		for (int j = 1; j < COUNT_FERM; j++)
		{
			for (i = 0; i <= ELM[j][1].NOTES_COUNTS - 1; i++)
			{
				ELM[j][4].set_NOTES_NUMBER(ELM[j][1].NOTES_NUMBER[i]);
				ELM[j][4].set_NOTES_NUMBER(ELM[j][2].NOTES_NUMBER[i]);
			}
		}
	}
#pragma endregion


#pragma region Элементы
	
	if (Type != 3) {

		if (Type != 2) {
			j = 1;
			RM_COUNT = 1;
		}

		//ВП
		for (int j = 1; j <= RM_COUNT; j++)
		{
			for (i = 0; i < (ELM[j][1].NOTES_COUNTS) - 1; i++) {
				Node[0] = ELM[j][1].NOTES_NUMBER[i]; Node[1] = ELM[j][1].NOTES_NUMBER[i + 1];
				ELM[j][1].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}

		}
		
		if (Type == 2) j = 2;
		else {
			j = 1;
			RM_COUNT = 2;
		}
		
		for (j; j <= RM_COUNT - 1; j++)
		{
			//НП
			for (i = 0; i < (ELM[j][2].NOTES_COUNTS) - 1; i++) {
				Node[0] = ELM[j][2].NOTES_NUMBER[i]; Node[1] = ELM[j][2].NOTES_NUMBER[i + 1];
				ELM[j][2].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
			//Решётка
			bool creat_ST = false;
			for (i = 0; i <= ELM[j][2].NOTES_COUNTS - 1; i = i + 2 * Runs_Multiplier) {

				if (creat_ST == false) {
					if ((i > (ELM[j][2].NOTES_COUNTS - 1) / 2) && (COUNT_PANELS % 2 == 0)) {
						creat_ST = true;
						Node[0] = ELM[j][2].NOTES_NUMBER[(ELM[j][2].NOTES_COUNTS - 1) / 2]; Node[1] = ELM[j][1].NOTES_NUMBER[(ELM[j][1].NOTES_COUNTS - 1) / 2];
						ELM[j][3].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
					}
				}

				Node[0] = ELM[j][2].NOTES_NUMBER[i]; Node[1] = ELM[j][1].NOTES_NUMBER[i];
				ELM[j][3].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[j][2].NOTES_NUMBER[i]; Node[1] = ELM[j][1].NOTES_NUMBER[i + Runs_Multiplier];
				ELM[j][3].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[j][2].NOTES_NUMBER[i]; Node[1] = ELM[j][1].NOTES_NUMBER[i + 2 * Runs_Multiplier];
				ELM[j][3].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
			

			if (Type != 0) {
				//Колонна
				for (i = 0; i < ELM[j][4].NOTES_COUNTS - 1; i++) {
					Node[0] = ELM[j][4].NOTES_NUMBER[i]; Node[1] = ELM[j][4].NOTES_NUMBER[i + 1];
					ELM[j][4].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				}

				for (i = 0; i < ELM[j][5].NOTES_COUNTS - 1; i++) {
					Node[0] = ELM[j][5].NOTES_NUMBER[i]; Node[1] = ELM[j][5].NOTES_NUMBER[i + 1];
					ELM[j][5].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				}
			}
		}
		// Ветровые связи
		if (Type == 1) {
			j = 1;
			Node[0] = ELM[j][15].NOTES_NUMBER[0]; Node[1] = ELM[j][15].NOTES_NUMBER[1];
			ELM[j][15].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 55, 2, Node));
			Node[0] = ELM[j][15].NOTES_NUMBER[2]; Node[1] = ELM[j][15].NOTES_NUMBER[3];
			ELM[j][15].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 55, 2, Node));
		}
	}
	if (Type == 2) {

	//Колонны фахверка

	for (int k = 1; k <= 2; k++)
	{	
		for (int n = 0; n <= ((COUNT_PANELS * 2) / COUNT_FAHVERK) + 1; n++) {

			for (int m = 0; m < KF_ELM[k][n].NOTES_COUNTS - 1; m++) {
				Node[0] = KF_ELM[k][n].NOTES_NUMBER[m]; Node[1] = KF_ELM[k][n].NOTES_NUMBER[m + 1];
				KF_ELM[k][n].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}		
		}
	}
	
	
	//Прогоны
	for (int j = 1; j < RM_COUNT; j++) {
		for (i = 0; i <= (ELM[j][1].NOTES_COUNTS) - 1; i++) {
			Node[0] = ELM[j][1].NOTES_NUMBER[i]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[i];
			ELM[j][6].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
		}	
	}
	
	
	//Распорки
	if (HR!=0) {
		for (int j = 2; j < RM_COUNT - 1; j++) {
			Node[0] = ELM[j][8].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][8].NOTES_NUMBER[0];
			ELM[j][8].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			Node[0] = ELM[j][9].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][9].NOTES_NUMBER[0];
			ELM[j][9].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
		}
		
		j = 1;

		Node[0] = ELM[j][10].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][8].NOTES_NUMBER[0];
		ELM[j][10].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

		Node[0] = ELM[j][10].NOTES_NUMBER[ELM[j][10].NOTES_COUNTS - 1]; Node[1] = ELM[j + 1][9].NOTES_NUMBER[0];
		ELM[j][10].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

		for (int k = 1; k <= 2; k++)
		{			
			for (int i = 0; i < ELM[j][10].NOTES_COUNTS - 1; i++) {
					Node[0] = ELM[j][10].NOTES_NUMBER[i]; Node[1] = ELM[j][10].NOTES_NUMBER[i + 1];
					ELM[j][10].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
			j = RM_COUNT;

		}

		Node[0] = ELM[j - 1][8].NOTES_NUMBER[ELM[j - 1][8].NOTES_COUNTS - 1]; Node[1] = ELM[j][10].NOTES_NUMBER[0];
		ELM[j][10].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

		Node[0] = ELM[j - 1][9].NOTES_NUMBER[ELM[j - 1][9].NOTES_COUNTS - 1]; Node[1] = ELM[j][10].NOTES_NUMBER[ELM[j][10].NOTES_COUNTS - 1];
		ELM[j][10].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

	}

	
		for (int j = 2; j < RM_COUNT - 1; j++)
		{
			//if (COUNT_PANELS % 2 != 0) {
				Node[0] = ELM[j][11].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][11].NOTES_NUMBER[0];
				ELM[j][11].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				/*}
			else {
				Node[0] = ELM[j][12].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][12].NOTES_NUMBER[0];
				ELM[j][12].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[j][13].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][13].NOTES_NUMBER[0];
				ELM[j][13].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}*/
		}
	
		
			//Cвязи по кровле
			for (int j = 1; j < RM_COUNT; j++) {
				if (j <= ((RM_COUNT) / 2)) {
					Node[0] = ELM[j][1].NOTES_NUMBER[0]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[0 + Runs_Multiplier];
					ELM[j][7].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				
					Node[0] = ELM[j][1].NOTES_NUMBER[ELM[j][1].NOTES_COUNTS - Runs_Multiplier - 1]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[ELM[j][1].NOTES_COUNTS - 1];
					ELM[j][7].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));					
				}
				else {
					Node[0] = ELM[j][1].NOTES_NUMBER[0 + Runs_Multiplier]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[0];
					ELM[j][7].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
					
					Node[0] = ELM[j][1].NOTES_NUMBER[ELM[j][1].NOTES_COUNTS - 1]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[ELM[j][1].NOTES_COUNTS - Runs_Multiplier - 1];
					ELM[j][7].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				}
			}

			for (int j = 1; j <= RM_COUNT; j = j + RM_COUNT - 2) {
				for (i = Runs_Multiplier; i < (ELM[j][1].NOTES_COUNTS - Runs_Multiplier - 1); i = i + Runs_Multiplier) {
					if (j == 1) {
						Node[0] = ELM[j][1].NOTES_NUMBER[i]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[i + Runs_Multiplier];
						ELM[j][7].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
					}
					else {
						Node[0] = ELM[j][1].NOTES_NUMBER[i + Runs_Multiplier]; Node[1] = ELM[j + 1][1].NOTES_NUMBER[i];
						ELM[j][7].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
					}
				}
			}
			
			//Связи по рядовым колоннам

			#pragma region Связи
			if (HR != 0) {
				//1
				i = 0;
				Node[0] = KF_ELM[1][0].NOTES_NUMBER[0];
				while (ApiGetNode(SCAD_MODEL, ELM[3][4].NOTES_NUMBER[i])->z < HR)
				{
					i++;
					Node[1] = ELM[2][4].NOTES_NUMBER[i];
				}
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[2][4].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][0].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[2][4].NOTES_NUMBER[i];
				Node[1] = KF_ELM[1][0].NOTES_NUMBER[KF_ELM[1][0].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][0].NOTES_NUMBER[i];
				Node[1] = ELM[2][4].NOTES_NUMBER[ELM[2][4].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//2
				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = ELM[2][5].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[2][5].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[2][5].NOTES_NUMBER[i];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				Node[1] = ELM[2][5].NOTES_NUMBER[ELM[2][5].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//3
				Node[0] = KF_ELM[2][0].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][4].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][4].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][0].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][4].NOTES_NUMBER[i];
				Node[1] = KF_ELM[2][0].NOTES_NUMBER[KF_ELM[2][0].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][0].NOTES_NUMBER[i];
				Node[1] = ELM[RM_COUNT - 1][4].NOTES_NUMBER[ELM[RM_COUNT - 1][4].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//4
				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][5].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][5].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][5].NOTES_NUMBER[i];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				Node[1] = ELM[RM_COUNT - 1][5].NOTES_NUMBER[ELM[RM_COUNT - 1][5].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//5
				Node[0] = KF_ELM[1][0].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][0].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][1].NOTES_NUMBER[i];
				Node[1] = KF_ELM[1][0].NOTES_NUMBER[KF_ELM[1][0].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][0].NOTES_NUMBER[i];
				Node[1] = KF_ELM[1][1].NOTES_NUMBER[KF_ELM[1][1].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//6
				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[i];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//7
				Node[0] = KF_ELM[2][0].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][0].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][1].NOTES_NUMBER[i];
				Node[1] = KF_ELM[2][0].NOTES_NUMBER[KF_ELM[2][0].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][0].NOTES_NUMBER[i];
				Node[1] = KF_ELM[2][1].NOTES_NUMBER[KF_ELM[2][1].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//8
				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[i];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[i];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_COUNTS - 1];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
			else {
				//1
				i = 0;
				Node[0] = KF_ELM[1][0].NOTES_NUMBER[0];
				Node[1] = ELM[2][4].NOTES_NUMBER[ELM[2][4].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[2][4].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][0].NOTES_NUMBER[KF_ELM[1][0].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//2
				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = ELM[2][5].NOTES_NUMBER[ELM[2][5].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[2][5].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//3
				Node[0] = KF_ELM[2][0].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][4].NOTES_NUMBER[ELM[RM_COUNT - 1][4].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][4].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][0].NOTES_NUMBER[KF_ELM[2][0].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//4
				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][5].NOTES_NUMBER[ELM[RM_COUNT - 1][5].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][5].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//5
				Node[0] = KF_ELM[1][0].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][1].NOTES_NUMBER[KF_ELM[1][1].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][0].NOTES_NUMBER[KF_ELM[1][0].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//6
				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[2][((COUNT_PANELS * 2) / COUNT_FAHVERK)].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//7
				Node[0] = KF_ELM[2][0].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][1].NOTES_NUMBER[KF_ELM[2][1].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[2][1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[2][0].NOTES_NUMBER[KF_ELM[2][0].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				//8
				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK) - 1].NOTES_NUMBER[0];
				Node[1] = KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].NOTES_NUMBER[KF_ELM[1][((COUNT_PANELS * 2) / COUNT_FAHVERK)].ELEMENTS_COUNTS];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}

			//Распорки по НП
			char *strt;



			/*if (COUNT_PANELS % 2 == 0) {
				i = (ELM[2][1].NOTES_COUNTS + 1) / 2 - 2 - 1;

				Node[0] = ELM[2][12].NOTES_NUMBER[0];
				Node[1] = ELM[3][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[3][12].NOTES_NUMBER[0];
				Node[1] = ELM[2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][12].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[RM_COUNT - 2][12].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				i = (ELM[2][1].NOTES_COUNTS + 1) / 2 + 2 - 1;

				Node[0] = ELM[2][13].NOTES_NUMBER[0];
				Node[1] = ELM[3][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[3][13].NOTES_NUMBER[0];
				Node[1] = ELM[2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][13].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[RM_COUNT - 2][13].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
			else
			{*/
				i = (ELM[2][1].NOTES_COUNTS + 1) / 2 - 1;

				Node[0] = ELM[2][11].NOTES_NUMBER[0];
				Node[1] = ELM[3][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[3][11].NOTES_NUMBER[0];
				Node[1] = ELM[2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));

				Node[0] = ELM[RM_COUNT - 1][11].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 2][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				Node[0] = ELM[RM_COUNT - 2][11].NOTES_NUMBER[0];
				Node[1] = ELM[RM_COUNT - 1][1].NOTES_NUMBER[i];
				ELM[1][14].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			//}
		
		#pragma endregion
		}
	if (Type == 3) {
		//ВП
		for (int j = 1; j < COUNT_FERM; j++)
		{
			for (i = 0; i < (ELM[j][1].NOTES_COUNTS) - 1; i++) {
				Node[0] = ELM[j][1].NOTES_NUMBER[i]; Node[1] = ELM[j][1].NOTES_NUMBER[i + 1];
				ELM[j][1].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}

		}
		//НП
		for (int j = 1; j < COUNT_FERM; j++)
		{
			for (i = 0; i < (ELM[j][2].NOTES_COUNTS) - 1; i++) {
				Node[0] = ELM[j][2].NOTES_NUMBER[i]; Node[1] = ELM[j][2].NOTES_NUMBER[i + 1];
				ELM[j][2].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
		}
		//Решётка
		for (int j = 1; j < COUNT_FERM; j++)
		{
			for (i = 0; i < (ELM[j][3].NOTES_COUNTS) - 1; i = i + 2) {
				Node[0] = ELM[j][3].NOTES_NUMBER[i]; Node[1] = ELM[j][3].NOTES_NUMBER[i + 1];
				ELM[j][3].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
			}
		}

		//РП
			for (int j = 1; j < COUNT_FERM; j++)
			{
				for (i = 0; i < (ELM[j][4].NOTES_COUNTS) - 1; i = i + 2) {
					Node[0] = ELM[j][4].NOTES_NUMBER[i]; Node[1] = ELM[j][4].NOTES_NUMBER[i + 1];
					ELM[j][4].set_ELEMENTS_NUMBER(ApiElemAddData(SCAD_MODEL, 5, 2, Node));
				}

			}
	}
#pragma endregion


#pragma region Шарниры
	if (Type == 2) {
		for (j = 1; j <= RM_COUNT; j++) {
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][1].ELEMENTS_NUMBER[COUNT_PANELS * Runs_Multiplier - 1], 2, 0);
			for (i = 0; i < (ELM[j][6].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][6].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][6].ELEMENTS_NUMBER[i], 2, 0);
			}
			for (i = 0; i < (ELM[j][7].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][7].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][7].ELEMENTS_NUMBER[i], 2, 0);
			}
		}

		for (j = 2; j <= RM_COUNT - 1; j++) {
			for (i = 0; i < (ELM[j][3].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][3].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][3].ELEMENTS_NUMBER[i], 2, 0);
			}
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][4].ELEMENTS_NUMBER[ELM[j][4].ELEMENTS_COUNTS - 1], 2, 0);
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][5].ELEMENTS_NUMBER[ELM[j][5].ELEMENTS_COUNTS - 1], 2, 0);
		}

		for (j = 2; j < RM_COUNT - 1; j++) {
			if (HR != 0)
			{
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][8].ELEMENTS_NUMBER[0], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][8].ELEMENTS_NUMBER[0], 2, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][9].ELEMENTS_NUMBER[0], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][9].ELEMENTS_NUMBER[0], 2, 0);
			}

			/*if (COUNT_PANELS % 2 != 0)
			{*/
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][11].ELEMENTS_NUMBER[0], 2, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][11].ELEMENTS_NUMBER[0], 2, 0);
			/*}
			else
			{
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][12].ELEMENTS_NUMBER[0], 2, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][12].ELEMENTS_NUMBER[0], 2, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][13].ELEMENTS_NUMBER[0], 2, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][13].ELEMENTS_NUMBER[0], 2, 0);
			}*/
		}

		for (int k = 1; k <= 2; k++)
		{
			for (int n = 0; n < ((COUNT_PANELS * 2) / COUNT_FAHVERK) + 1; n++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, KF_ELM[k][n].ELEMENTS_NUMBER[KF_ELM[k][n].ELEMENTS_COUNTS - 1], 2, 0);
			}
		}

		for (i = 0; i < (ELM[1][14].ELEMENTS_COUNTS); i++) {
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[1][14].ELEMENTS_NUMBER[i], 1, 0);
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[1][14].ELEMENTS_NUMBER[i], 2, 0);
		}

		for (i = 0; i < (ELM[1][10].ELEMENTS_COUNTS); i++) {
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[1][10].ELEMENTS_NUMBER[i], 1, 0);
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[1][10].ELEMENTS_NUMBER[i], 2, 0);
		}

		for (i = 0; i < (ELM[RM_COUNT][10].ELEMENTS_COUNTS); i++) {
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[RM_COUNT][10].ELEMENTS_NUMBER[i], 1, 0);
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[RM_COUNT][10].ELEMENTS_NUMBER[i], 2, 0);
		}
	}
	else if (Type != 3) {
		j = 1;
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][1].ELEMENTS_NUMBER[COUNT_PANELS * Runs_Multiplier - 1], 2, 0);
			for (i = 0; i < (ELM[j][3].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][3].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][3].ELEMENTS_NUMBER[i], 2, 0);
			}
		if (Type == 1){
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][4].ELEMENTS_NUMBER[ELM[j][4].ELEMENTS_COUNTS - 1], 2, 0);
			ApiSetJoint(SCAD_MODEL, SgDirectUY | SgDirectUZ, ELM[j][5].ELEMENTS_NUMBER[ELM[j][5].ELEMENTS_COUNTS - 1], 2, 0);
		}
	}
	else if (Type == 3) {
		for (j = 1; j < COUNT_FERM; j++) {
			for (i = 0; i < (ELM[j][1].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUZ, ELM[j][1].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUZ, ELM[j][1].ELEMENTS_NUMBER[i], 2, 0);
			}
			for (i = 0; i < (ELM[j][2].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUZ, ELM[j][2].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUZ, ELM[j][2].ELEMENTS_NUMBER[i], 2, 0);
			}
			for (i = 0; i < (ELM[j][3].ELEMENTS_COUNTS); i++) {
				ApiSetJoint(SCAD_MODEL, SgDirectUZ, ELM[j][3].ELEMENTS_NUMBER[i], 1, 0);
				ApiSetJoint(SCAD_MODEL, SgDirectUZ, ELM[j][3].ELEMENTS_NUMBER[i], 2, 0);
			}
		}
	}
#pragma endregion 
	
	
#pragma region Связи
	if (Type == 2) {
		for (j = 2; j <= RM_COUNT - 1; j++) {
			Node[0] = ELM[j][4].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUZ | SgDirectUY, 1, Node, TRUE);

			Node[0] = ELM[j][5].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUZ | SgDirectUY, 1, Node, TRUE);
		}

		for (int k = 1; k <= 2; k++)
		{
			for (int n = 0; n < ((COUNT_PANELS * 2) / COUNT_FAHVERK) + 1; n++) {
				Node[0] = KF_ELM[k][n].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
				ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUX | SgDirectUZ | SgDirectUX, 1, Node, TRUE);
			}
		}
	}
	else if (Type == 1) {
		j = 1;
		for (i = 1; i <= ApiGetQuantityNode(SCAD_MODEL); i++) {
			Node[0] = i; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectY, 1, Node, TRUE);
		}

		Node[0] = ELM[j][4].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
		ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUZ | SgDirectUY | SgDirectUX, 1, Node, TRUE);

		Node[0] = ELM[j][5].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
		ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUZ | SgDirectUY | SgDirectUX, 1, Node, TRUE);

		Node[0] = ELM[j][15].NOTES_NUMBER[1]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
		ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUZ | SgDirectUY | SgDirectUX, 1, Node, TRUE);

		Node[0] = ELM[j][15].NOTES_NUMBER[3]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
		ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY | SgDirectUZ | SgDirectUY | SgDirectUX, 1, Node, TRUE);
	}
	else if (Type == 0) {
		j = 1;
		for (i = 1; i <= ApiGetQuantityNode(SCAD_MODEL); i++) {
			Node[0] = i; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectY, 1, Node, TRUE);
		}
		Node[0] = ELM[j][1].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
		ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectY, 1, Node, TRUE);

		Node[0] = ELM[j][1].NOTES_NUMBER[ELM[j][1].NOTES_COUNTS - 1]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
		ApiSetBound(SCAD_MODEL, SgDirectZ | SgDirectY, 1, Node, TRUE);
	}
	else if (Type == 3) {
		for (i = 1; i <= ApiGetQuantityNode(SCAD_MODEL); i++) {
			Node[0] = i; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectZ, 1, Node, TRUE);
		}

		for (j = 1; j < COUNT_FERM; j++) {
			Node[0] = ELM[j][2].NOTES_NUMBER[0]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectY | SgDirectZ, 1, Node, TRUE);

			Node[0] = ELM[j][2].NOTES_NUMBER[ELM[j][2].NOTES_COUNTS - 1]; Node[1] = 0; Node[2] = 0; Node[3] = 0;
			ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ, 1, Node, TRUE);
		}
	}
		
#pragma endregion 

	
#pragma region Жесткости
	double Size[6];
	UINT Elem[4];

	if (Type == 2) {
		// ВП	
		for (j = 1; j <= RM_COUNT; j++) {
			for (i = 0; i < (ELM[j][1].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(1, ELM[j][1].ELEMENTS_NUMBER[i]);
			}
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 1, "ВП");

		for (j = 2; j <= (RM_COUNT - 1); j++) {
			for (i = 0; i < (ELM[j][2].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(2, ELM[j][2].ELEMENTS_NUMBER[i]);
			}
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 2, "НП");


		// Раскосы
		for (j = 2; j <= (RM_COUNT - 1); j++) {
			for (i = 0; i < (ELM[2][3].ELEMENTS_COUNTS - 2); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(3, ELM[j][3].ELEMENTS_NUMBER[i + 1]);
				Size[0] = 90;
				Elem[0] = ELM[j][3].ELEMENTS_NUMBER[i + 1];
				ApiSetSystemCoordElem(SCAD_MODEL, ApiGroupRod, ApiRodCornerInDegrees, 1, Size, 1, Elem);
			}
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 3, "2хРС");

		// Раскосы
		for (j = 2; j <= (RM_COUNT - 1); j++) {
			
			RIGIN_ELM.SET_ELEMENTS_NUMBER(4, ELM[j][3].ELEMENTS_NUMBER[(ELM[2][3].ELEMENTS_COUNTS - 1) / 2]);
				Size[0] = 90;
				Elem[0] = ELM[j][3].ELEMENTS_NUMBER[(ELM[2][3].ELEMENTS_COUNTS - 1) / 2];
				ApiSetSystemCoordElem(SCAD_MODEL, ApiGroupRod, ApiRodCornerInDegrees, 1, Size, 1, Elem);
			
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 4, "1хРС");

		// Опорные раскосы
		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i <= 1; i++)
			{
				if (i == 0)
				{
					RIGIN_ELM.SET_ELEMENTS_NUMBER(5, ELM[j][3].ELEMENTS_NUMBER[0]);
				}
				else
				{
					RIGIN_ELM.SET_ELEMENTS_NUMBER(5, ELM[j][3].ELEMENTS_NUMBER[ELM[j][3].ELEMENTS_COUNTS - 1]);
				}
			}
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 5, "ОР");

		// Колонны
		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < ELM[2][4].ELEMENTS_COUNTS; i++)
			{
				RIGIN_ELM.SET_ELEMENTS_NUMBER(6, ELM[j][4].ELEMENTS_NUMBER[i]);
			}
		}

		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < ELM[2][5].ELEMENTS_COUNTS; i++)
			{
				RIGIN_ELM.SET_ELEMENTS_NUMBER(6, ELM[j][5].ELEMENTS_NUMBER[i]);
			}
		}

		for (int k = 1; k <= 2; k++)
		{
			for (int n = 0; n <= ((COUNT_PANELS * 2) / COUNT_FAHVERK) + 1; n++) {

				for (int i = 0; i < KF_ELM[k][n].ELEMENTS_COUNTS; i++) {
					RIGIN_ELM.SET_ELEMENTS_NUMBER(6, KF_ELM[k][n].ELEMENTS_NUMBER[i]);
					Size[0] = 90;
					Elem[0] = KF_ELM[k][n].ELEMENTS_NUMBER[i];
					ApiSetSystemCoordElem(SCAD_MODEL, ApiGroupRod, ApiRodCornerInDegrees, 1, Size, 1, Elem);
				}
			}
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 6, "К");

		// Прогоны
		for (int j = 1; j < RM_COUNT; j++) {
			for (i = 0; i < (ELM[j][6].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(7, ELM[j][6].ELEMENTS_NUMBER[i]);
				if (i < (ELM[j][6].ELEMENTS_COUNTS - 1) / 2) Size[0] = -atan(RAD);
				else if (i == (ELM[j][6].ELEMENTS_COUNTS - 1) / 2) Size[0] = 0;
				else if (i > (ELM[j][6].ELEMENTS_COUNTS - 1) / 2) Size[0] = atan(RAD);
				Elem[0] = ELM[j][6].ELEMENTS_NUMBER[i];
				ApiSetSystemCoordElem(SCAD_MODEL, ApiGroupRod, ApiRodCornerInDegrees, 1, Size, 1, Elem);
			}
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 7, "ПРГ");

		// Распорки нижнего пояса
		for (j = 2; j < RM_COUNT - 1; j++) {
			//if (COUNT_PANELS % 2 != 0)
			//{
				RIGIN_ELM.SET_ELEMENTS_NUMBER(8, ELM[j][11].ELEMENTS_NUMBER[0]);
				/*}
			else
			{
				RIGIN_ELM.SET_ELEMENTS_NUMBER(7, ELM[j][12].ELEMENTS_NUMBER[0]);
				RIGIN_ELM.SET_ELEMENTS_NUMBER(7, ELM[j][13].ELEMENTS_NUMBER[0]);
			}*/
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 8, "РП_НП");

		// Связи
		for (j = 1; j <= RM_COUNT; j++) {
			for (i = 0; i < (ELM[j][7].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(9, ELM[j][7].ELEMENTS_NUMBER[i]);
			}
		}
		for (i = 0; i < ELM[1][14].ELEMENTS_COUNTS; i++) {
			RIGIN_ELM.SET_ELEMENTS_NUMBER(9, ELM[1][14].ELEMENTS_NUMBER[i]);
		}
		RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 9, "СВ");

		// Распорки колонн
		if (HR != 0)
		{
			for (int j = 1; j < RM_COUNT; j++) {
				for (i = 0; i < (ELM[j][8].ELEMENTS_COUNTS); i++) {
					RIGIN_ELM.SET_ELEMENTS_NUMBER(10, ELM[j][8].ELEMENTS_NUMBER[i]);
				}
			}
			for (int j = 1; j < RM_COUNT; j++) {
				for (i = 0; i < (ELM[j][9].ELEMENTS_COUNTS); i++) {
					RIGIN_ELM.SET_ELEMENTS_NUMBER(10, ELM[j][9].ELEMENTS_NUMBER[0]);
				}
			}


			for (i = 0; i < (ELM[1][10].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(10, ELM[1][10].ELEMENTS_NUMBER[i]);
			}

			for (i = 0; i < (ELM[RM_COUNT][10].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(10, ELM[RM_COUNT][10].ELEMENTS_NUMBER[i]);
			}
			RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 10, "РП_К");
		}
	}
	else if (Type != 3) {
	j = 1;
	// ВП	
		for (i = 0; i < (ELM[j][1].ELEMENTS_COUNTS); i++) {
			RIGIN_ELM.SET_ELEMENTS_NUMBER(1, ELM[j][1].ELEMENTS_NUMBER[i]);
		}
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 1, "ВП");

		for (i = 0; i < (ELM[j][2].ELEMENTS_COUNTS); i++) {
			RIGIN_ELM.SET_ELEMENTS_NUMBER(2, ELM[j][2].ELEMENTS_NUMBER[i]);
		}
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 2, "НП");

	
	// Раскосы
		for (i = 0; i < (ELM[j][3].ELEMENTS_COUNTS - 2); i++) {
			RIGIN_ELM.SET_ELEMENTS_NUMBER(3, ELM[j][3].ELEMENTS_NUMBER[i + 1]);
		}
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 3, "РС");
	
	
	// Опорные раскосы
		for (i = 0; i <= 1; i++)
		{
			if (i == 0)
			{
				RIGIN_ELM.SET_ELEMENTS_NUMBER(4, ELM[j][3].ELEMENTS_NUMBER[0]);
			}
			else
			{
				RIGIN_ELM.SET_ELEMENTS_NUMBER(4, ELM[j][3].ELEMENTS_NUMBER[ELM[j][3].ELEMENTS_COUNTS - 1]);
			}
		}

	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 4, "ОР");
	
	// Колонны
	if (Type == 1) {
		for (i = 0; i < ELM[j][4].ELEMENTS_COUNTS; i++)
		{
			RIGIN_ELM.SET_ELEMENTS_NUMBER(5, ELM[j][4].ELEMENTS_NUMBER[i]);
		}

		for (i = 0; i < ELM[j][5].ELEMENTS_COUNTS; i++)
		{
			RIGIN_ELM.SET_ELEMENTS_NUMBER(5, ELM[j][5].ELEMENTS_NUMBER[i]);
		}
	
	
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 5, "К");

	if (Type = 1) {
		UINT *Elem = new UINT[2];
		str = "SPRING ";
		str = str + Kw + " Type 55";
		strcpy(buffer, str.c_str());
		cout << buffer;
		n = ApiSetRigid(SCAD_MODEL, buffer);
		Elem[0] = ELM[j][15].ELEMENTS_NUMBER[0];   Elem[1] = ELM[j][15].ELEMENTS_NUMBER[1];
		ApiSetRigidElem(SCAD_MODEL, n, 6, Elem);
	}
	
	}
}
	else if (Type == 3) {
	// ВП	
		for (j = 1; j < COUNT_FERM; j++) {
			for (i = 0; i < (ELM[j][1].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(1, ELM[j][1].ELEMENTS_NUMBER[i]);
			}
		}
	// НП
		for (j = 1; j < COUNT_FERM; j++) {
			for (i = 0; i < (ELM[j][2].ELEMENTS_COUNTS); i++) {
				RIGIN_ELM.SET_ELEMENTS_NUMBER(1, ELM[j][2].ELEMENTS_NUMBER[i]);
			}
		}
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 1, "ПРГ");
	// Решётка
	for (j = 1; j < COUNT_FERM; j++) {
		for (i = 0; i < (ELM[j][3].ELEMENTS_COUNTS); i++) {
			RIGIN_ELM.SET_ELEMENTS_NUMBER(2, ELM[j][3].ELEMENTS_NUMBER[i]);
		}
	}
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 2, "СВ");

	// РП
	for (j = 1; j < COUNT_FERM; j++) {
		for (i = 0; i < (ELM[j][4].ELEMENTS_COUNTS); i++) {
			RIGIN_ELM.SET_ELEMENTS_NUMBER(3, ELM[j][4].ELEMENTS_NUMBER[i]);
		}
	}
	RIGIN_ELM.RIGIN_INSERT(SCAD_MODEL, 3, "ВП");
}

#pragma endregion


#pragma region Нагрузки


if (Type == 2) {
	d_PRG = ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[1])->x - ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->x;
	d_KF = - ApiGetNode(SCAD_MODEL, KF_ELM[1][1].NOTES_NUMBER[0])->x + ApiGetNode(SCAD_MODEL, KF_ELM[1][2].NOTES_NUMBER[0])->x;
	double SIZE_LOAD[4];

	UINT *Elem = new UINT[1];
	Elem = new UINT[ApiGetElemQuantity(SCAD_MODEL)];

	#pragma region СВ
		i = 1;
		ApiSetLoadDescription(SCAD_MODEL, 1, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.3");
		ApiSetLoadName(SCAD_MODEL, 1, "СВ");
		for (i = 1; i <= ApiGetElemQuantity(SCAD_MODEL); i++) ApiSetWeight(SCAD_MODEL, 1, 1, &i, 1.3, TRUE, FALSE);
#pragma endregion


	#pragma region ПОСТОЯННАЯ
		ApiSetLoadDescription(SCAD_MODEL, 2, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
		ApiSetLoadName(SCAD_MODEL, 2, "ПОСТОЯННАЯ");

		//Прогоны
		SIZE_LOAD[0] = Q[1] * d_PRG;;
		for (int j = 1; j < RM_COUNT; j++) {
			for (i = 0; i < (ELM[j][6].ELEMENTS_COUNTS); i++) {
				Elem[i] = ELM[j][6].ELEMENTS_NUMBER[i];
			}
			ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][6].ELEMENTS_COUNTS, Elem);
			memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
		}


		//НП
		SIZE_LOAD[0] = Q[2] * d_RAM;
		for (int j = 2; j < RM_COUNT; j++) {
			for (i = 0; i < (ELM[j][2].ELEMENTS_COUNTS); i++) {
				Elem[i] = ELM[j][2].ELEMENTS_NUMBER[i];
			}
			ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][2].ELEMENTS_COUNTS, Elem);
			memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
		}

		// Колонны

		SIZE_LOAD[0] = Q[3] * d_RAM;
		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < ELM[2][4].ELEMENTS_COUNTS; i++)
			{
				Elem[i] = ELM[j][4].ELEMENTS_NUMBER[i];
			}
			ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][4].ELEMENTS_COUNTS, Elem);
			memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
		}

		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < ELM[2][5].ELEMENTS_COUNTS; i++)
			{
				Elem[i] = ELM[j][5].ELEMENTS_NUMBER[i];
			}
			ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][5].ELEMENTS_COUNTS, Elem);
			memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
		}

		SIZE_LOAD[0] = Q[3] * d_KF;
		for (int k = 1; k <= 2; k++)
		{
			for (int n = 0; n <= ((COUNT_PANELS * 2) / COUNT_FAHVERK) + 1; n++) {

				for (int i = 0; i < KF_ELM[k][n].ELEMENTS_COUNTS; i++) {
					Elem[i] = KF_ELM[k][n].ELEMENTS_NUMBER[i];
				}
				ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, KF_ELM[k][n].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
			}
		}
#pragma endregion		


	#pragma region СНЕГ
		ApiSetLoadDescription(SCAD_MODEL, 3, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
		ApiSetLoadName(SCAD_MODEL, 3, "СНЕГ");

		//Прогоны
		SIZE_LOAD[0] = SnowLoad(DisSnow) * d_PRG;
		for (int j = 1; j < RM_COUNT; j++) {
			for (i = 0; i < (ELM[j][6].ELEMENTS_COUNTS); i++) {
				Elem[i] = ELM[j][6].ELEMENTS_NUMBER[i];
			}
			ApiAppendForce(SCAD_MODEL, 3, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][6].ELEMENTS_COUNTS, Elem);
			memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
		}
#pragma endregion


	#pragma region ВЕТЕР ПОПЕРЁК
		ApiSetLoadDescription(SCAD_MODEL, 4, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
		ApiSetLoadName(SCAD_MODEL, 4, "ВЕТЕР + ПУЛЬС X");

		//КОЛОННЫ
		for (int k = 1; k <= 2; k++) {
			for (int m = 0; m <= ((COUNT_PANELS * 2) / COUNT_FAHVERK); m++) {
				for (i = 0; i < (KF_ELM[k][m].ELEMENTS_COUNTS); i++) {
					if (m == 0) {
						SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "D", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (-1);
						SIZE_LOAD[1] = 0;
						SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "D", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (-1);
						SIZE_LOAD[3] = 1;
						Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
						ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
						memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						if (k == 1) {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
					else if (m == ((COUNT_PANELS * 2) / COUNT_FAHVERK)) {
						SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "E", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (-1);
						SIZE_LOAD[1] = 0;
						SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "E", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (-1);
						SIZE_LOAD[3] = 1;
						Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
						ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
						memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						if (k == 1) {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							//memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
					else {
						if (k == 1) {
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (1);
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[3] = 1;
							ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (-1);
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[3] = 1;
							ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
				}
			}

		}

		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < (ELM[j][4].ELEMENTS_COUNTS); i++) {
				SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "D", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
				SIZE_LOAD[1] = 0;
				SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "D", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
				SIZE_LOAD[3] = 1;
				Elem[0] = ELM[j][4].ELEMENTS_NUMBER[i];
				ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][4].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
			}

			for (i = 0; i < (ELM[j][5].ELEMENTS_COUNTS); i++) {
				SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "E", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
				SIZE_LOAD[1] = 0;
				SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "E", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
				SIZE_LOAD[3] = 1;
				Elem[0] = ELM[j][5].ELEMENTS_NUMBER[i];
				ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][5].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

			}
		}
#pragma endregion


	#pragma region ВЕТЕР ВДОЛЬ
		ApiSetLoadDescription(SCAD_MODEL, 5, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
		ApiSetLoadName(SCAD_MODEL, 5, "ВЕТЕР + ПУЛЬС Y");

		//КОЛОННЫ
		for (int k = 1; k <= 2; k++) {
			for (int m = 0; m <= ((COUNT_PANELS * 2) / COUNT_FAHVERK); m++) {
				for (i = 0; i < (KF_ELM[k][m].ELEMENTS_COUNTS); i++) {
					if (m == 0) {
						SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (1);
						SIZE_LOAD[1] = 0;
						SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (1);
						SIZE_LOAD[3] = 1;
						Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
						ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
						memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						if (k == 1) {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "D", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "D", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "E", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "E", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							//memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
					else if (m == ((COUNT_PANELS * 2) / COUNT_FAHVERK)) {
						SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (-1);
						SIZE_LOAD[1] = 0;
						SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM / 2 * (-1);
						SIZE_LOAD[3] = 1;
						Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
						ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
						memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						if (k == 1) {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "D", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "D", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "E", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "E", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
					else {
						if (k == 1) {
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "D", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (-1);
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "D", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[3] = 1;
							ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "E", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (-1);
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "E", d_KF, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_KF * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[3] = 1;
							ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
				}
			}

		}

		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < (ELM[j][4].ELEMENTS_COUNTS); i++) {
				SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (1);
				SIZE_LOAD[1] = 0;
				SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (1);
				SIZE_LOAD[3] = 1;
				Elem[0] = ELM[j][4].ELEMENTS_NUMBER[i];
				ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][4].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
			}

			for (i = 0; i < (ELM[j][5].ELEMENTS_COUNTS); i++) {
				SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
				SIZE_LOAD[1] = 0;
				SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "ВДОЛЬ", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
				SIZE_LOAD[3] = 1;
				Elem[0] = ELM[j][5].ELEMENTS_NUMBER[i];
				ApiAppendForce(SCAD_MODEL, 5, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][5].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

			}
		}
#pragma endregion


	#pragma region ВЕТЕР ПОПЕРЁК БЕЗ ПУЛЬСАЦИИ
		ApiSetLoadDescription(SCAD_MODEL, 6, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
		ApiSetLoadName(SCAD_MODEL, 6, "ВЕТЕР X");

		//КОЛОННЫ
		for (int k = 1; k <= 2; k++) {
			for (int m = 0; m <= ((COUNT_PANELS * 2) / COUNT_FAHVERK); m++) {
				for (i = 0; i < (KF_ELM[k][m].ELEMENTS_COUNTS); i++) {
					if (m == 0) {
						SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "D", d_RAM, 0) * d_RAM / 2 * (-1);
						SIZE_LOAD[1] = 0;
						SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "D", d_RAM, 0) * d_RAM / 2 * (-1);
						SIZE_LOAD[3] = 1;
						Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
						ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
						memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						if (k == 1) {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
					else if (m == ((COUNT_PANELS * 2) / COUNT_FAHVERK)) {
						SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "E", d_RAM, 0) * d_RAM / 2 * (-1);
						SIZE_LOAD[1] = 0;
						SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->y, "E", d_RAM, 0) * d_RAM / 2 * (-1);
						SIZE_LOAD[3] = 1;
						Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
						ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
						memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						if (k == 1) {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF / 2 * (-1);
							SIZE_LOAD[3] = 1;
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							//memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
					else {
						if (k == 1) {
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF * (1);
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF * (1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[3] = 1;
							ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
						else {
							Elem[0] = KF_ELM[k][m].ELEMENTS_NUMBER[i];
							SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF * (-1);
							SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, KF_ELM[k][m].NOTES_NUMBER[i])->x, "ПОПЕРЁК", d_KF, 0) * d_KF * (-1);
							SIZE_LOAD[1] = 0;
							SIZE_LOAD[3] = 1;
							ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceY, 4, SIZE_LOAD, KF_ELM[k][m].ELEMENTS_COUNTS, Elem);
							memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
						}
					}
				}
			}

		}

		for (j = 2; j <= (RM_COUNT - 1); j++)
		{
			for (i = 0; i < (ELM[j][4].ELEMENTS_COUNTS); i++) {
				SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "D", d_RAM, 0) * d_RAM * (-1);
				SIZE_LOAD[1] = 0;
				SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "D", d_RAM, 0) * d_RAM * (-1);
				SIZE_LOAD[3] = 1;
				Elem[0] = ELM[j][4].ELEMENTS_NUMBER[i];
				ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][4].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
			}

			for (i = 0; i < (ELM[j][5].ELEMENTS_COUNTS); i++) {
				SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "E", d_RAM, 0) * d_RAM * (-1);
				SIZE_LOAD[1] = 0;
				SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "E", d_RAM, 0) * d_RAM * (-1);
				SIZE_LOAD[3] = 1;
				Elem[0] = ELM[j][5].ELEMENTS_NUMBER[i];
				ApiAppendForce(SCAD_MODEL, 6, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][5].ELEMENTS_COUNTS, Elem);
				memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

			}
		}
#pragma endregion

}

else if (Type < 2)  {
	d_PRG = ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[1])->x - ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->x;
	double SIZE_LOAD[4];

	UINT *Elem = new UINT[1];
	Elem = new UINT[ApiGetElemQuantity(SCAD_MODEL)];

	#pragma region СВ
	i = 1;
	ApiSetLoadDescription(SCAD_MODEL, 1, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.3");
	ApiSetLoadName(SCAD_MODEL, 1, "СВ");
	for (i = 1; i <= ApiGetElemQuantity(SCAD_MODEL); i++) ApiSetWeight(SCAD_MODEL, 1, 1, &i, 1.3, TRUE, FALSE);
#pragma endregion


	#pragma region ПОСТОЯННАЯ
	ApiSetLoadDescription(SCAD_MODEL, 2, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
	ApiSetLoadName(SCAD_MODEL, 2, "ПОСТОЯННАЯ");

	j = 1;
	//ВП
	SIZE_LOAD[0] = Q[1] * d_RAM;
	for (i = 0; i < (ELM[j][1].ELEMENTS_COUNTS); i++) {
		Elem[i] = ELM[j][1].ELEMENTS_NUMBER[i];
	}
	ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][1].ELEMENTS_COUNTS, Elem);
	memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));


	//НП
	SIZE_LOAD[0] = Q[2] * d_RAM;
	for (i = 0; i < (ELM[j][2].ELEMENTS_COUNTS); i++) {
		Elem[i] = ELM[j][2].ELEMENTS_NUMBER[i];
	}
	ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][2].ELEMENTS_COUNTS, Elem);
	memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

	// Колонны

	SIZE_LOAD[0] = Q[3] * d_RAM;
	for (i = 0; i < ELM[j][4].ELEMENTS_COUNTS; i++)
	{
		Elem[i] = ELM[j][4].ELEMENTS_NUMBER[i];
	}
	ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][4].ELEMENTS_COUNTS, Elem);
	memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

	for (i = 0; i < ELM[j][5].ELEMENTS_COUNTS; i++)
	{
		Elem[i] = ELM[j][5].ELEMENTS_NUMBER[i];
	}
	ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][5].ELEMENTS_COUNTS, Elem);
	memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

#pragma endregion	

	#pragma region СНЕГ
	ApiSetLoadDescription(SCAD_MODEL, 3, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
	ApiSetLoadName(SCAD_MODEL, 3, "СНЕГ");
	j = 1;
	//ВП
	SIZE_LOAD[0] = SnowLoad(DisSnow) * d_RAM;
	for (i = 0; i < (ELM[j][1].ELEMENTS_COUNTS); i++) {
		Elem[i] = ELM[j][1].ELEMENTS_NUMBER[i];
	}
	ApiAppendForce(SCAD_MODEL, 3, ApiForceEvenlyGlobal, SgForceZ, 1, SIZE_LOAD, ELM[j][1].ELEMENTS_COUNTS, Elem);
	memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));

#pragma endregion

	if (Type == 1) {

	#pragma region ВЕТЕР ПОПЕРЁК
	ApiSetLoadDescription(SCAD_MODEL, 4, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
	ApiSetLoadName(SCAD_MODEL, 4, "ВЕТЕР + ПУЛЬС X");

	//КОЛОННЫ
	j = 1;
	for (i = 0; i < (ELM[j][4].ELEMENTS_COUNTS); i++) {
		SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "D", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
		SIZE_LOAD[1] = 0;
		SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][4].NOTES_NUMBER[i])->y, "D", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
		SIZE_LOAD[3] = 1;
		Elem[0] = ELM[j][4].ELEMENTS_NUMBER[i];
		ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][4].ELEMENTS_COUNTS, Elem);
		memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
	}

	for (i = 0; i < (ELM[j][5].ELEMENTS_COUNTS); i++) {
		SIZE_LOAD[0] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "E", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
		SIZE_LOAD[1] = 0;
		SIZE_LOAD[2] = WindLoad(ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i + 1])->z, DisWind, TArea, ApiGetNode(SCAD_MODEL, ELM[j][5].NOTES_NUMBER[i])->y, "E", d_RAM, ApiGetNode(SCAD_MODEL, ELM[1][1].NOTES_NUMBER[0])->z) * d_RAM * (-1);
		SIZE_LOAD[3] = 1;
		Elem[0] = ELM[j][5].ELEMENTS_NUMBER[i];
		ApiAppendForce(SCAD_MODEL, 4, ApiForceTrapezPartGlobal, SgForceX, 4, SIZE_LOAD, ELM[j][5].ELEMENTS_COUNTS, Elem);
		memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
	}

#pragma endregion

	}


}

else if (Type == 3){
	double SIZE_LOAD[4];

	UINT *Elem = new UINT[1];
	Elem = new UINT[ApiGetQuantityNode(SCAD_MODEL)];

	ApiSetLoadDescription(SCAD_MODEL, 1, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.2");
	ApiSetLoadName(SCAD_MODEL, 1, "1");
 
	for (j = 1; j < COUNT_FERM; j++) {
		SIZE_LOAD[0] = -j;
		for (i = 0; i < ELM[j][1].NOTES_COUNTS; i++) {
			Elem[i] = ELM[j][1].NOTES_NUMBER[i];
		}
		ApiAppendForce(SCAD_MODEL, 1, ApiFоrceNode, SgForceX, 1, SIZE_LOAD, ELM[j][1].NOTES_COUNTS, Elem);
		//memset(Elem, 0, sizeof(UINT)*ApiGetElemQuantity(SCAD_MODEL));
	}
}
#pragma endregion


#pragma region Пример кода на API
		/*
	// Элементы
	ApiElemAdd(SCAD_MODEL, 21);
	for (i = 0; i<4; i++) {
		n = i * 6 + 1;
		for (j = 0; j<3; j++) {
			Node[0] = 4 * i + j + 1;   Node[1] = Node[0] + 1;
			ApiElemUpdate(SCAD_MODEL, n++, 5, 2, Node);
			Node[0] = Node[1];   Node[1] = Node[0] + 4;
			if (i < 3) ApiElemUpdate(SCAD_MODEL, n++, 5, 2, Node);
		}
	}

	Node[0] = 2;   Node[1] = 6; 	Node[2] = 3;   Node[3] = 7;
	n = ApiElemAddData(SCAD_MODEL, 41, 4, Node);
	Node[0] = 10;   Node[1] = 14; 	Node[2] = 11;   Node[3] = 15;
	n = ApiElemAddData(SCAD_MODEL, 41, 4, Node);
	ApiElemSetName(SCAD_MODEL, n, "Plate");

	// жесткости
	// пользовательское сечение
	n = ApiSetRigid(SCAD_MODEL, "S0 3.52e+006 20 25 NU 0.2 RO 2.5 TMP 1e-005 Shift 493.004 61488.2 61548.5");
	for (i = 1; i <= 21; i++) ApiSetRigidElem(SCAD_MODEL, n, 1, &i);
	// металлопрокат
	n = ApiSetRigid(SCAD_MODEL, "STZ RUSSIAN p_wide_h 18 TMP 1.2e-005");
	Elem[0] = 6;   Elem[1] = 18;
	ApiSetRigidElem(SCAD_MODEL, n, 2, Elem);
	// жесткости пластин
	n = ApiSetRigid(SCAD_MODEL, "GE 2.1e+007 0.3 0.1 RO 7.85 TMP 1.2e-005 1.2e-005");
	ApiSetRigidName(SCAD_MODEL, 2, "Plate");
	Elem[0] = 22;   Elem[1] = 23;
	ApiSetRigidElem(SCAD_MODEL, n, 2, Elem);

	// связи	
	Node[0] = 1;   Node[1] = 5;  Node[2] = 9;    Node[3] = 13;
	ApiSetBound(SCAD_MODEL, SgDirectX | SgDirectZ | SgDirectUY, 4, Node, TRUE);

	// объединения связей
	Node[0] = 4;   Node[1] = 16;
	n = ApiSetBoundUnite(SCAD_MODEL, SgDirectX | SgDirectZ, 2, Node);
	ApiSetBoundUniteName(SCAD_MODEL, n, "Union 1");

	// шарниры
	ApiSetJoint(SCAD_MODEL, SgDirectUX | SgDirectUY, 4, 2, 0);
	ApiSetJoint(SCAD_MODEL, SgDirectUX | SgDirectUY, 16, 1, 0);

	// жесткие вставки
	Elem[0] = 6; Elem[1] = 18;
	Size[0] = 1;	Size[1] = 0;	Size[2] = 1;	Size[3] = -1;	Size[4] = 0;	Size[5] = 1;
	ApiSetInsert(SCAD_MODEL, 1, 3, 6, Size, 2, Elem);

	//  коэффициенты постели
	Size[0] = 0.1;	Size[1] = 1000;	Size[2] = 500;
	Size[3] = 0.2;	Size[4] = 1500;	Size[5] = 700;
	Elem[0] = 1;   Elem[1] = 7;  Elem[2] = 13;    Elem[3] = 19;
	ApiSetBed(SCAD_MODEL, ApiGroupRod, 0, 6, Size, 4, Elem);

	// Группы элементов
	Elem[0] = 22;   Elem[1] = 23;
	ApiSetGroupElem(SCAD_MODEL, "Plate", 2, 2, Elem);
	Elem[0] = 1;   Elem[1] = 7;  Elem[2] = 13;   Elem[3] = 19;
	ApiSetGroupElem(SCAD_MODEL, "Rod", 2, 4, Elem);
	Size[0] = 1200;	Size[1] = 300;
	Elem[0] = 23;
	ApiSetBed(SCAD_MODEL, ApiGroupPlate, 'I', 6, Size, 1, Elem);

	// Группы узлов	
	Node[0] = 1;   Node[1] = 5;  Node[2] = 9;    Node[3] = 13;
	ApiSetGroupNode(SCAD_MODEL, "Bound", 4, Node);

	// системы координат элементов
	Size[0] = 45;
	Elem[0] = 7;   Elem[1] = 13;
	ApiSetSystemCoordElem(SCAD_MODEL, ApiGroupRod, ApiRodCornerInDegrees, 1, Size, 2, Elem);
	Size[0] = 1;	Size[1] = 0;	Size[2] = 1;
	Elem[0] = 22;   Elem[1] = 23;
	ApiSetSystemCoordEffors(SCAD_MODEL, ApiGroupPlate, ApiPlateAxecX, 2, Size, 2, Elem);

	// 4-е загружения
	ApiSetLoadDescription(SCAD_MODEL, 1, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.05");
	ApiSetLoadName(SCAD_MODEL, 1, "Узловые нагрузки");
	ApiSetLoadDescription(SCAD_MODEL, 2, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.05");
	ApiSetLoadName(SCAD_MODEL, 2, "Распределенные нагрузки");
	ApiSetLoadDescription(SCAD_MODEL, 3, "Type=0  Mode=1  LongTime=1  ReliabilityFactor=1.05");
	ApiSetLoadName(SCAD_MODEL, 3, "Собственный вес");
	ApiSetLoadDescription(SCAD_MODEL, 4, "Type=2  ReliabilityFactor=1.1  21 5 1  1 3 0 0 0 5 18 1 0 0.3 1");
	ApiSetLoadName(SCAD_MODEL, 4, "Ветер");
	//  преобразование статических загружений в массы
	ZeroMemory(Size, sizeof(Size));
	Size[3] = 1;
	ApiSetLoadMass(SCAD_MODEL, 4, 4, Size);

	// нагрузки
	Size[0] = 1.2;
	Node[0] = 8;   Node[1] = 12;
	ApiAppendForce(SCAD_MODEL, 1, ApiFоrceNode, SgForceZ, 1, Size, 2, Node);
	Size[0] = -0.5;
	Node[0] = 2;   Node[1] = 3;  Node[2] = 4;
	ApiAppendForce(SCAD_MODEL, 1, ApiFоrceNode, SgForceX, 1, Size, 3, Node);

	Size[0] = 2.1;
	Node[0] = 6;   Node[1] = 12;  Node[1] = 18;
	ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, Size, 3, Node);

	Size[0] = 2.1;
	Node[0] = 6;   Node[1] = 12;  Node[1] = 18;
	ApiAppendForce(SCAD_MODEL, 2, ApiForceEvenlyGlobal, SgForceZ, 1, Size, 3, Node);
	for (i = 1; i <= 23; i++) ApiSetWeight(SCAD_MODEL, 3, 1, &i, 1.1, TRUE, FALSE);
	*/
#pragma endregion

	APICode Code;
	Code = ApiWriteProject(SCAD_MODEL, "TestNewProject.spr");
	if (Code != APICode_OK) { APIPhrase(SCAD_MODEL, Code); }
	ApiRelease(&SCAD_MODEL);

}


