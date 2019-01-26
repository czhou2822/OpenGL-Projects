using namespace std;
#include <string>
#include <algorithm>
#include "turtle_string.h"


void turtle_string::set_iter(int input) {
	iter = input;
}

void turtle_string::set_rot(float input) {
	rot = input;
}

void turtle_string::set_function(string input) {
	function = input;
}

void turtle_string::get_final_string() {
	std::string str = start;

	cout << "size: " << str.size() << endl;

	for (int i = 0; i < iter; i++) {
		str = ReplaceAll(str, std::string("F"), function);
		//cout << "iter: "<<i<< " size: " << str.size() << endl;
	}
	
	
	str = ReplaceAll(str, std::string("F"), function);
	
	final_string = str;
	//std::cout << ReplaceAll(string("ghghjghugtghty"), std::string("gh"), std::string("h")) << std::endl;

	//cout << "function: "<<str << endl;
}


void turtle_string::read_file(char *fn) {
	ifstream input_file;
	input_file.open(fn);

	std::string contend;
	input_file >> contend;

	while (contend != "iter:") {
		input_file >> contend;
	}

	input_file >> contend;

	set_iter(stoi(contend));

	input_file >> contend;
	input_file >> contend;
	set_rot(stof(contend));

	input_file >> contend;
	input_file >> contend;
	input_file >> contend;
	input_file >> contend;
	input_file >> contend;
	input_file >> contend;

	set_function(contend);
	
	//cout << contend << endl;
	get_final_string();
	return;
}

string turtle_string::ReplaceAll(std::string str, const std::string& from, const std::string& to)
{	
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
	//return string();
}

