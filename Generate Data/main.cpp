// Generate Data.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>
#include <cassert>

#include "compress_quicklz.h"

using namespace std;

bool generateData(const char * input, const char * output)
{
	assert(input && output);
	// Read input file
	ifstream inf(input);
	if (!inf.is_open()) {
		cerr << "Error: cannot open input file '" << input << "'\n";
		return false;
	}
	string in_buf;
	in_buf.assign(istreambuf_iterator<char>(inf), istreambuf_iterator<char>());
	inf.close();
	// Compress data
	string out_buf;
	if (!CCompressorQuickLZ().compress(in_buf, out_buf)) {
		cerr << "Error: cannot compress input data (len=" << in_buf.size() << ")\n";
		return false;
	}
	// Write output file
	ofstream outf(output, ios_base::binary);
	if (!outf.is_open()) {
		cerr << "Error: cannot open output file '" << output << "'\n";
		return false;
	}
	outf.write(out_buf.c_str(), out_buf.size());
	return true;
}

int main() {
	generateData("itemdata.txt", "../Diablo Edit2/itemdata.dat");
	generateData("property.txt", "../Diablo Edit2/property.dat");
	generateData("language.txt", "../Diablo Edit2/language.dat");
	generateData("miscdata.txt", "../Diablo Edit2/miscdata.dat");
	return 0;
}
