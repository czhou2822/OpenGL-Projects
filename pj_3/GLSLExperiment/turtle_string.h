#pragma once
#include "Angel.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <Vector>
//void set_iter(int input);
//void set_rot(int input);
//void set_function(string input);
//void read_file(char *fn);

using std::string;
using std::ifstream;
typedef vec3 point3;

class turtle_string {
public:	
	int len;
	int iter;
	float rot;
	string start = "F";
	string function;
	string final_string;
	point3 up;
	point3 at;
	point3 eye;

public:
	void set_iter(int);
	void set_rot(float);
	void set_function(string);
	void get_final_string(void);
	void read_file(char*);
	string ReplaceAll(std::string str, const std::string& from, const std::string& to);
};