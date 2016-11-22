#ifndef IPLAYER_H_2016_07_27
#define IPLAYER_H_2016_07_27
//
#include "../rmi/rmi.h"
#include "IplayerInterface.h"
#include "../rmi/task.h"

class CPlayer:public IplayerInterface
{
public:
	CPlayer();
	virtual ~CPlayer();
	virtual int LoginLoad(Event& event);
	virtual int Report(Event& event);
	virtual void LogOut(Event& event);
protected:
private:

};
RMI_SERVER_CLASS_DECLARE(RMI_CPlayer,CPlayer)

RMI_WRAPPER_CONSTRUCTOR
{
	Closure_Helper(PFPE_LOGIN,this,&CPlayer::LoginLoad);
	Closure_Helper(PFPE_REPORT,this,&CPlayer::Report);
}
RMI_SERVERFUNCTION_END()


#endif//IPLAYER_H_2016_07_27