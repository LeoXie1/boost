#pragma once

#include <iostream>
#include <queue>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
using namespace boost;

typedef boost::function<void(void)> MyTask;
//�������--noncopyable
class MyTaskQueue : boost::noncopyable
{
private:
	std::queue<MyTask> m_taskQueue;
	boost::mutex m_mutex;//������
	boost::condition_variable_any m_cond;//��������
public:
	void push_Task(const MyTask& task) {
		//���ϻ�����
		boost::unique_lock<boost::mutex> lock(m_mutex);
		m_taskQueue.push(task);
		//֪ͨ�����߳�����
		m_cond.notify_one();
	}

	MyTask pop_Task() {
		//���ϻ�����
		boost::unique_lock<boost::mutex> lock(m_mutex);
		if (m_taskQueue.empty())
		{
			//���������û��������ȴ������� 
			cout << "m_taskQueue empty" << endl;
			m_cond.wait(lock);//
			//return ;
		}
		//ָ������ײ�
		MyTask task(m_taskQueue.front());
		//������  
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
	//�������
	MyTaskQueue m_taskQueue;
	//�߳��� 
	boost::thread_group m_threadGroup;
	int m_threadNum;
	/*
	volatile ������������α���ͬ�̷߳��ʺ��޸ĵı�����
	volatile ���߱�����i����ʱ���ܷ����仯�ģ�ÿ��ʹ������ʱ������i�ĵ�ַ�ж�ȡ��
	������������ɵĿ�ִ��������´�i�ĵ�ַ��ȡ���ݷ���k�С�
	volatile���Ա�֤�������ַ���ȶ����ʣ��������
	*/
	volatile bool is_run;
	void run() {//�̳߳����̵߳Ĵ�����
		while (is_run) {
			//һֱ�����̳߳ص�����
			if (m_taskQueue.get_size())
			{
				cout << "empty run" << endl;
			}
			MyTask task = m_taskQueue.pop_Task();
			task();//����bind�ĺ���
		}
	}
public:
	MyThreadPool(int num) :m_threadNum(num), is_run(false)//��ʼ���б�
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
			//���ɶ���̣߳���run��������ӵ��߳���
			m_threadGroup.add_thread(
				new boost::thread(boost::bind(&MyThreadPool::run, this)));
		}
	}
	//ֹͣ�̳߳�
	void stop()
	{
		is_run = false;
	}
	//�������
	void AddNewTask(const MyTask& task) {
		m_taskQueue.push_Task(task);
	}
	void wait()
	{
		m_threadGroup.join_all();//�ȴ��̳߳ش�����ɣ�
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
		//�ŵ��̳߳��д���bind(f , i) will produce a "nullary" function object that takes no arguments and returns f(i),����ʱ���ɴ����κ����͵ĺ���������������
		tp.AddNewTask(task);
	}

	tp.init();
	//�ȴ��̳߳ش�����ɣ�  
	tp.wait();
	cout << "add 5th task" << endl;

	MyTask task = boost::bind(ProcFun, 5 + 1, CallBackFun);
	//�ŵ��̳߳��д���bind(f , i) will produce a "nullary" function object that takes no arguments and returns f(i),����ʱ���ɴ����κ����͵ĺ���������������
	tp.AddNewTask(task);

	tp.wait();
}


//����һ���̣߳�ִ�к�ʱ�������ȵ�������ɣ����ûص�����
void testAsyncCall(int i, pFunCallBack callfun)
{
	boost::thread th(boost::bind(ProcFun, i, callfun));
}
void testAsyncCall2(int i)
{
	//bind����Ƕ��,�ص����� --bind(f, bind(g, _1))(x); // f(g(x))
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


void testAsyncCall3(int para)//ʹ��bindע��ִ�к����ͻص�����
{
	MyTask2<int, int> tk(ProcFun2, CallBackFun2);
	//tk.Run(para);
	MyTask task = boost::bind(&MyTask2<int, int>::Run, tk, para);
	boost::thread th(task);
	//boost::thread th(boost::bind(&MyTask2<int,int>::Run,tk,para));
}

