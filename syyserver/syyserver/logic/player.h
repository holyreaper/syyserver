#ifndef IPLAYER_H_2016_07_27
#define IPLAYER_H_2016_07_27
//
#include "../rmi/rmi.h"
#include "IplayerInterface.h"
class CPlayer:public IplayerInterface
{
public:
	CPlayer();
	virtual ~CPlayer();
	virtual int LoginLoad();
	virtual int Report(testsc a);
	virtual void LogOut();
protected:
private:

};
RMI_SERVER_CLASS_DECLARE(RMI_CPlayer,CPlayer)

RMI_WRAPPER_CONSTRUCTOR
{
//	REGISTER_RMI(1,RMI_CPlayer,&CPlayer::LoginLoad);
	Closure_Helper(1,this,&CPlayer::LoginLoad);
	Closure_Helper(2,this,&CPlayer::Report,testsc());
}
RMI_SERVERFUNCTION_END()

enum PLAYER_FUNC_PARAMER_ENUM
{	
	PFPE_START,
	PFPE_LOGIN,
	PFPE_REPORT,
	PFPE_MAX=0xFF,
};
#endif//IPLAYER_H_2016_07_27