#ifndef IPLAYER_INTERFACE_H_2016_07_27
#define IPLAYER_INTERFACE_H_2016_07_27
//��ҽӿ�
struct IplayerInterface
{
	virtual int LoginLoad() =0;
	virtual void LogOut()=0;
	virtual int Report(testsc a)=0;

};


#endif