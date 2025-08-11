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
	signal.Listen(&Bar::StaticMethod);
	signal.Listen(&FunctionListener);
	auto data = SignalData(42, "value");
	signal.Send(&data);
	signal.StopListen(&obj);
	signal.Send(&data);
	signal.StopListen(&FunctionListener);
}
