#pragma once

#include <iostream>
#include <queue>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
using namespace boost;

typedef boost::function<void(void)> MyTask;
//任务队列--noncopyable
class MyTaskQueue : boost::noncopyable
{
private:
	std::queue<MyTask> m_taskQueue;
	boost::mutex m_mutex;//互斥锁
	boost::condition_variable_any m_cond;//条件变量
public:
	void push_Task(const MyTask& task) {
		//加上互斥锁
		boost::unique_lock<boost::mutex> lock(m_mutex);
		m_taskQueue.push(task);
		//通知其他线程启动
		m_cond.notify_one();
	}

	MyTask pop_Task() {
		//加上互斥锁
		boost::unique_lock<boost::mutex> lock(m_mutex);
		if (m_taskQueue.empty())
		{
			//如果队列中没有任务，则等待互斥锁 
			cout << "m_taskQueue empty" << endl;
			m_cond.wait(lock);//
			//return ;
		}
		//指向队列首部
		MyTask task(m_taskQueue.front());
		//出队列  
		m_taskQueue.pop();
		return task;
	}
	int get_size()
	{
		return m_taskQueue.size();
	}
};

class MyThreadPool : boost::noncopyable
{
private:
	//任务队列
	MyTaskQueue m_taskQueue;
	//线程组 
	boost::thread_group m_threadGroup;
	int m_threadNum;
	/*
	volatile 被设计用来修饰被不同线程访问和修改的变量。
	volatile 告诉编译器i是随时可能发生变化的，每次使用它的时候必须从i的地址中读取，
	因而编译器生成的可执行码会重新从i的地址读取数据放在k中。
	volatile可以保证对特殊地址的稳定访问，不会出错。
	*/
	volatile bool is_run;
	void run() {//线程池中线程的处理函数
		while (is_run) {
			//一直处理线程池的任务
			if (m_taskQueue.get_size())
			{
				cout << "empty run" << endl;
			}
			MyTask task = m_taskQueue.pop_Task();
			task();//运行bind的函数
		}
	}
public:
	MyThreadPool(int num) :m_threadNum(num), is_run(false)//初始化列表
	{

	}
	~MyThreadPool() {
		stop();
	}
	void init()
	{
		if (m_threadNum <= 0) return;
		is_run = true;
		for (int i = 0; i<m_threadNum; i++)
		{
			//生成多个线程，绑定run函数，添加到线程组
			m_threadGroup.add_thread(
				new boost::thread(boost::bind(&MyThreadPool::run, this)));
		}
	}
	//停止线程池
	void stop()
	{
		is_run = false;
	}
	//添加任务
	void AddNewTask(const MyTask& task) {
		m_taskQueue.push_Task(task);
	}
	void wait()
	{
		m_threadGroup.join_all();//等待线程池处理完成！
	}
};

typedef void(*pFunCallBack)(int i);
void CallBackFun(int i)
{
	std::cout << i << " call back!" << std::endl;
}

void ProcFun(int ti, pFunCallBack callback)
{
	std::cout << "I am Task: " << ti << std::endl;
	//task
	for (int i = 0; i<ti * 100000000; i++)
	{
		i*i;
	}
	if (callback != NULL)callback(ti);
}


void CallBackFun2(int i)
{
	std::cout << i << " call back! v2" << std::endl;
}

int ProcFun2(int& ti)
{
	std::cout << "I am Task " << ti << std::endl;
	//task
	for (int i = 0; i<ti * 100000000; i++)
	{
		i*i;
	}
	return ti;
}


void testThreadPool()
{
	MyThreadPool tp(2);
	int taskNum = 4;
	for (int i = 0; i<taskNum; i++)
	{
		MyTask task = boost::bind(ProcFun, i + 1, CallBackFun);
		//放到线程池中处理，bind(f , i) will produce a "nullary" function object that takes no arguments and returns f(i),调用时，可传递任何类型的函数及参数！！！
		tp.AddNewTask(task);
	}

	tp.init();
	//等待线程池处理完成！  
	tp.wait();
	cout << "add 5th task" << endl;

	MyTask task = boost::bind(ProcFun, 5 + 1, CallBackFun);
	//放到线程池中处理，bind(f , i) will produce a "nullary" function object that takes no arguments and returns f(i),调用时，可传递任何类型的函数及参数！！！
	tp.AddNewTask(task);

	tp.wait();
}


//创建一个线程，执行耗时操作，等到操作完成，调用回调函数
void testAsyncCall(int i, pFunCallBack callfun)
{
	boost::thread th(boost::bind(ProcFun, i, callfun));
}
void testAsyncCall2(int i)
{
	//bind函数嵌套,回调函数 --bind(f, bind(g, _1))(x); // f(g(x))
	boost::thread th(boost::bind(CallBackFun2, boost::bind(ProcFun2, i)));
}

template <class ParaType, class RetType>
class MyTask2 {
	typedef boost::function<RetType(ParaType&)> ProcFun;
	typedef boost::function<void(RetType)> CallBackFun;
protected:
	ProcFun m_procFun;
	CallBackFun m_callbackFun;
public:
	MyTask2() :m_procFun(NULL), m_callbackFun(NULL) {

	}
	MyTask2(ProcFun proc, CallBackFun callback) :m_procFun(proc), m_callbackFun(callback) {

	}
	~MyTask2() {

	}
	void Run(ParaType& para) {
		if (m_procFun != NULL && m_callbackFun != NULL)
		{
			m_callbackFun(m_procFun(para));
		}
	}
};


void testAsyncCall3(int para)//使用bind注册执行函数和回调函数
{
	MyTask2<int, int> tk(ProcFun2, CallBackFun2);
	//tk.Run(para);
	MyTask task = boost::bind(&MyTask2<int, int>::Run, tk, para);
	boost::thread th(task);
	//boost::thread th(boost::bind(&MyTask2<int,int>::Run,tk,para));
}

