#ifndef IPLAYER_INTERFACE_H_2016_07_27
#define IPLAYER_INTERFACE_H_2016_07_27
//Íæ¼Ò½Ó¿Ú
struct IplayerInterface
{
	virtual int LoginLoad(Event&event) =0;
	virtual void LogOut(Event&event)=0;
	virtual int Report(Event&event)=0;
};


#endif