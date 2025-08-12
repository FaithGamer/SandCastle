#pragma once

#include <SandCastle.h>

using namespace SandCastle;

struct SignalData
{
	int data1;
	String data2;
};

void FunctionListener(SignalData* data)
{
	LOG_INFO("Function receive signal: {0}, {1}", data->data1, data->data2);
}

class Bar
{
public:
	void MethodListener(SignalData* data)
	{
		LOG_INFO("Method receive signal: {0}, {1}", data->data1, data->data2);
	}
	static void StaticMethod(SignalData* data)
	{
		LOG_INFO("Static Method receive signal: {0}, {1}", data->data1, data->data2);

	}
};

void Signals()
{
	Engine::Init();

	Bar obj;

	Signal<SignalData*> signal;

	signal.Listen(&Bar::MethodListener, &obj);
	signal.Listen(&Bar::MethodListener, &obj); //Double will be ignored
	signal.Listen(&Bar::StaticMethod);
	signal.Listen(&Bar::StaticMethod);//Double will be ignored
	signal.Listen(&FunctionListener);
	signal.Listen(&FunctionListener);//Double will be ignored
	auto data = SignalData(42, "value");
	LOG_INFO("Send");
	signal.Send(&data);
	LOG_INFO("");

	signal.StopListen(&obj);//Only need the object to stop listening
							//There's no reason the same object would listen
							//to the same signal on two different methods.
	LOG_INFO("Send");
	signal.Send(&data);
	LOG_INFO("");

	signal.StopListen(&Bar::StaticMethod);
	LOG_INFO("Send");
	signal.Send(&data);
	LOG_INFO("");

	signal.StopListen(&FunctionListener);
	LOG_INFO("Send");
	signal.Send(&data);
}
