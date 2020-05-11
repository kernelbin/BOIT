#include<Windows.h>
#include<stdlib.h>
#include"CommandManager.h"
#include"APITransfer.h"
#include"InlineCommand.h"
#include"Corpus.h"

int CmdMsg_donate_Proc(pBOIT_COMMAND pCmd, pBOIT_SESSION boitSession, WCHAR* Msg)
{
	switch (rand()%6)
	{
	case 0:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=我，bot，可爱，打钱！,content=_(:з」∠)_,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		SendBackMessage(boitSession, L"[CQ:image,file=8C048B6CD3FE93875DD32101364EF4C4.gif]");
		break;

	case 1:
		SendBackMessage(boitSession, L"[CQ:image,file=B7B62C976EB995D73658DDE68629EB7F.gif]");
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=然鹅kernel.bin好像还在咕咕咕,content=欢迎购买奶茶用于加速kernel.bin写代码,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;

	case 2:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=kernel.bin快穷死了,content=吃土ing＞﹏＜,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;

	case 3:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=赞助bot开发，拥有属于你自己的指令,content=qwq,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;
	case 4:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=SteveBot的服务器开销并不小。,content=kernel.bin需要像您这样的人提供资助。,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;
	case 5:
		SendBackMessage(boitSession, L"[CQ:share,url=https://afdian.net/@kernelbin,title=BOIT is made by kernel.bin with love。,content=赞助他继续开发下去！。,image=https://q1.qlogo.cn/g?b=qq&nk=1160386205&s=640]");
		break;
	}
	return 0;
}