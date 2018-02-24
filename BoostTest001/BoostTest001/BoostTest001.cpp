// BoostTest001.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/progress.hpp>
#include <boost\date_time\gregorian\gregorian.hpp>
#include <boost\format.hpp>
#include <boost\algorithm\string.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <iostream>
using namespace std;

class Shared
{
public:
	Shared()
	{
		std::cout << "ctor() called" << std::endl;
	}
	Shared(const Shared & other)
	{
		std::cout << "copy ctor() called" << std::endl;
	}
	~Shared()
	{
		std::cout << "dtor() called" << std::endl;
	}
	Shared & operator = (const Shared & other)
	{
		std::cout << "operator = called" << std::endl;
	}
};
#include <boost\any.hpp>
using namespace boost;
void test_any()
{
	typedef std::vector<boost::any> many;
	many a;
	a.push_back(2);
	a.push_back(string("test"));
	for (unsigned int i = 0; i<a.size(); ++i)
	{
		cout << a[i].type().name() << endl;
		try
		{
			int result = any_cast<int>(a[i]);
			cout << result << endl;
		}
		catch (boost::bad_any_cast & ex)
		{
			cout << "cast error:" << ex.what() << endl;
		}
	}
}

#include <boost\thread\thread.hpp>
void hello1()
{
	
	while (1)
	{
		Sleep(500);
		cout << "hello1 thread" << endl;
	}
	
}
void hello2()
{
	while (1)
	{
		Sleep(500);
		cout << "hello2 thread" << endl;
	}
}
#include "MyTaskPool.h"
int main()
{
	testAsyncCall2(11);
	cout << "main" << endl;
	Sleep(1000);
	return 0;
	testThreadPool();
	
	return 0;
	boost::thread th(hello1);
	boost::thread th2(hello2);
	th.join();
	th2.join();
	
	cout << "main" << endl;
	return 0;
	test_any();
	return 0;
	typedef boost::shared_ptr<Shared> sharedPT;
	std::vector<sharedPT> vt;
	vt.push_back(sharedPT(new Shared));
	vt.push_back(sharedPT(new Shared));

	boost::format f("a=%1%,b=%2%,c=%1%");
	f % "string" % 2;
	cout << f << endl;
	string str1 = "read.txt";
	boost::ends_with(str1, "txt");
	cout << boost::to_upper_copy(str1) + "UPPER" << endl;
	cout << str1 << endl;
	return 0;


	std::string strTime = boost::gregorian::to_iso_string(\
		boost::gregorian::day_clock::local_day());

	std::cout << strTime.c_str() << std::endl;
	std::vector<std::string> v(100);
	std::ofstream fs("test");
	boost::progress_display pd(v.size());
	std::vector<std::string>::iterator pos;
	for (pos = v.begin(); pos != v.end(); ++pos)
	{
		fs << *pos << std::endl;
		++pd;
		Sleep(1000);
		
	}
}