#include<WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"
#include "Game.h"
#include "GameMessage.pb.h"
#include "Socket.h"
#include "GameLogin.h"
#include <Windows.h>

#define KEYDOWN(vk) (GetAsyncKeyState(vk) & 0x8000)
extern GameSocket usesocket;

CGame::CGame()
{
    m_menuSelect.m_pParent = this;
    m_menuBackup.m_pParent = this;
}

CGame::~CGame()
{
	prome over;
	over.set_state(7);
	std::string decode_use = over.SerializeAsString();
	usesocket.Gamewrite(decode_use);
}

HANDLE hMutex = NULL;//创建互斥量

DWORD WINAPI RecvSer(LPVOID lpParamter)
{
	//Sleep(50);
	((CGame *)lpParamter)->gamethread();
	return 0;
}
void CGame::gamethread()
{
	//TIMEVAL timeout;
	//struct fd_set reads, cpy_reads;
	//FD_ZERO(&reads);
	//FD_SET(usesocket.hSocket, &reads);
	//int initnum = 0;

	while (1)
	{
		//if (decode_tank02 .length()>2)
		//{
		//	usesocket.Gamewrite(decode_tank02);
		//}//向服务器传输坦克位置。
		//cpy_reads = reads;
		//timeout.tv_sec = 0;
		//timeout.tv_usec = 2;

		float X, Y;
		int dire, state;
		string code;
		prome decode;
		//
		//int fd_num;
		//fd_num = select(0, &cpy_reads, 0, 0, &timeout);

		//if (fd_num > 0)//处理服务器发送回来的信息
		//{
		//
		code = usesocket.Gameread();
		decode.ParseFromString(code);
		state = decode.state();
		if (state == 3)
		{
			{
				m_player02 = CPlayer(0, 0, _T("tank_player2.png"));
				PointF ptCenter;
				//if (!m_map.FindRandomPosition(ptCenter)) {
				   // AfxMessageBox(_T("调整Player02位置失败"));
				//}
				//else {  //
				   // m_player02.SetCenterPoint(ptCenter);
				//}
				//接收服务器发送的02坦克的位置，完成初始化。
				//std::string tankp_code;
				//prome tankp;
				//tankp_code = usesocket.Gameread();
				//tankp.ParseFromString(tankp_code);
				ptCenter.X = decode.game().tankx();
				ptCenter.Y = decode.game().tanky();

				m_player02.SetCenterPoint(ptCenter);
			}

		}
			//
			//code = usesocket.Gameread();
			//decode.ParseFromString(code);
		//	state = decode.state();
		else if (state == 4)//play2 位置的获取调整
			{
				Game game = decode.game();
				X = game.tankx();
				Y = game.tanky();
				dire = game.dire();
				if (dire == 1)
				{
					m_player02.RotateLeft();
				}
				else if (dire == 2)
				{
					m_player02.RotateRight();
				}
				else if (dire == 3)
				{
					m_player02.forward();
				}
				else
				{
					m_player02.Backward();
				}
				m_player02.gogogo(X, Y);

			}
			else if (state == 5)//play2 子弹位置的获取调整   //子弹获取位置不需要！
			{   //传递开枪信息，子弹由本地01坦克生成
				//可以进行优化
				CBullet bult;
				Bullet bu = decode.bullet();
				X = bu.bulletx();
				Y = bu.bullety();
				m_player02.Fire(bult, X, Y);//Fire（）函数是根据坦克位置确定子弹发射位置。不在需要传递子弹位置信息，Fire2有误
				m_lstBullets.push_back(bult);
				//RemoveTimeoutBullets();
			}
			//在双人大战中已经处理 01 02中弹的爆炸 不再需要处理坦克爆炸信息。
			else if (state == 6)//坦克爆炸
			{
			return;
			//m_player02.Bomb();
			//m_eStep = EGameTypeOne2BotEnd;
			//(blt).SetActive(false);
			}
	}


}


// 设置输出窗口的句柄
void CGame::SetHandle(HWND hWnd)
{
    m_hWnd = hWnd;
}

// 进入游戏帧
bool CGame::EnterFrame(DWORD dwTime)
{
    GameRunLogic();
    GameRunDraw();
    return false;
}

// 处理鼠标移动事件
void CGame::OnMouseMove(UINT nFlags, CPoint point)
{
    // 选择阶段
    if (m_eStep == EGameTypeMenu) {
        // 选择游戏类型
        m_menuSelect.OnMouseMove(nFlags, point);
    }
    else {
        // 返回主菜单
        m_menuBackup.OnMouseMove(nFlags, point);
    }
}

// 处理左键抬起事件
void CGame::OnLButtonUp(UINT nFlags, CPoint point)
{
    // 选择阶段
    if (m_eStep == EGameTypeMenu) {
        // 选择游戏类型
        m_menuSelect.OnLButtonUp(nFlags, point);
    }
    else {
        // 返回主菜单
        m_menuBackup.OnLButtonUp(nFlags, point);
    }
}

// 设置当前游戏所处的阶段
// 并根据步聚 初始化
void CGame::SetStep(CGame::EGameType step)
{
    m_eStep = step;
    GameInit();
}

/* 游戏初始化
生成 游戏对象，初始化地图，对象位置等
*/
bool CGame::GameInit()
{
    //  初始化随机数生成器
    srand(GetTickCount());
    // 根据不同阶段调用不同的处理函数
    return (this->*m_initFunc[m_eStep])();
}

// 游戏初始化 : 选择阶段
bool CGame::GameInitMenu()
{
    return true;
}

// 游戏初始化 : 单人对电脑 菜单阶段
bool CGame::GameInitOne2BotMenu()
{
    // 设置 玩家一的 keyMenu 位置 : 屏幕正中间
    RECT rc;
    GetWindowRect(m_hWnd, &rc);
    PointF pt;
    pt.X = rc.left + (rc.right - rc.left) / 2.0f;
    pt.Y = rc.top + (rc.bottom - rc.top) / 2.0f;
    m_keymenu01.SetCenterPoint(pt);
    m_keymenu01.SetStop(false);
    return true;
}

// 游戏初始化 : 单人对电脑
bool CGame::GameInitOne2Bot()
{
    for (; ;) {
        // 地图
        m_map.LoadMap();
        // 玩家一
        {
            m_player01 = CPlayer(0, 0, _T("tank_player1.png"));
            PointF ptCenter;
            if (!m_map.FindRandomPosition(ptCenter)) {
                AfxMessageBox(_T("调整Player01位置失败"));
            }
            else {
                m_player01.SetCenterPoint(ptCenter);
            }
        }

        // 敌军
        {
            m_bot = CBot(0, 0, _T("tank_bot.png"));
            PointF ptCenter;
            if (!m_map.FindRandomPosition(ptCenter)) {
                AfxMessageBox(_T("调整Bot位置失败"));
            }
            else {
                m_bot.SetCenterPoint(ptCenter);
            }
        }
        // 子弹
        m_lstBullets.clear();

        // 判断是否合法
        {
            // 机器人，玩家 所在的位置
            int startX, startY, targetX, targetY;
            if (!m_map.FindObjPosition(m_bot, startX, startY) || 
                !m_map.FindObjPosition(m_player01, targetX, targetY)) {
                AfxMessageBox(_T("获取坦克位置发生错误"));
                goto __Init_End;
            }
            VPath path;
            m_map.FindPath(startX, startY, targetX, targetY, path);
            if (!path.empty()) {
                goto __Init_End;
            }
        }
    }
__Init_End:
    return true;
}

// 游戏初始化 : 单人对电脑 结束
bool CGame::GameInitOne2BotEnd()
{

    return true;
}

// 游戏初始化 : 双人对战 菜单阶段
bool CGame::GameInitOne2OneMenu()
{
	//直接进入游戏 不再进入游戏选择阶段
    // 设置 两个玩家的 keyMenu 位置 : 屏幕正中间
	/*
    RECT rc;
    GetWindowRect(m_hWnd, &rc);
    PointF pt;
    pt.X = rc.left + m_keymenu01.GetRect().Width / 2.0f + 100;
    pt.Y = rc.top + (rc.bottom - rc.top) / 2.0f;
    m_keymenu01.SetCenterPoint(pt);
    m_keymenu01.SetStop(false);


    pt.X = rc.right - m_keymenu02.GetRect().Width / 2.0f - 100;
    pt.Y = rc.top + (rc.bottom - rc.top) / 2.0f;
    m_keymenu02.SetCenterPoint(pt);
    m_keymenu02.SetStop(false);
	*/

    return true;
}

// 游戏初始化 : 双人对战
bool CGame::GameInitOne2One()
{
	hThread = CreateThread(NULL, 0, RecvSer, this, 0, NULL);//创建线程 用于传输接收信息
    //hMutex = CreateMutex(NULL, FALSE,(LPCWSTR)"Mutex");
	CloseHandle(hThread);//关闭线程句柄
    for (;;) {
        // 地图
        m_map.LoadMap();
        //中间放置坦克
        {
            m_player01 = CPlayer(0, 0, _T("tank_player1.png"));
            PointF ptCenter;
            if (!m_map.FindRandomPosition(ptCenter)) {
                AfxMessageBox(_T("调整Player01位置失败"));
            }
            else {
                m_player01.SetCenterPoint(ptCenter);
				//加入传输给服务器的坦克位置的函数
				//充填坦克位置的传输体
				prome TankP;
				Game *gamedata = new Game;
				Account *ac = new Account;
				ac->set_uid(usesocket.uid);
				TankP.set_state(3);
				gamedata->set_tankx(ptCenter.X);
				gamedata->set_tanky(ptCenter.Y);
				pp.X = gamedata->tankx();
				pp.Y = gamedata->tanky();
				inittank.X= gamedata->tankx();//用于线程中初始化坦克位置
				inittank.Y= gamedata->tanky();//用于线程中初始化坦克位置
				TankP.set_allocated_account(ac);
				TankP.set_allocated_game(gamedata);
				std::string decode_use = TankP.SerializeAsString();

				usesocket.Gamewrite(decode_use);
				
	
            }
        }
       /* {
            m_player02 = CPlayer(0, 0, _T("tank_player2.png"));
            PointF ptCenter;
            //if (!m_map.FindRandomPosition(ptCenter)) {
               // AfxMessageBox(_T("调整Player02位置失败"));
            //}
            //else {  //
               // m_player02.SetCenterPoint(ptCenter);
            //}
			//接收服务器发送的02坦克的位置，完成初始化。
			std::string tankp_code;
			prome tankp;
			tankp_code=usesocket.Gameread();
			tankp.ParseFromString(tankp_code);
			ptCenter.X = tankp.game().tankx();
			ptCenter.Y = tankp.game().tanky();

			m_player02.SetCenterPoint(ptCenter);


        }*/

        // 子弹
        m_lstBullets.clear();
        // 判断是否合法
      {
            // 玩家 所在的位置
			Sleep(200);
            int startX, startY, targetX, targetY;
            if (!m_map.FindObjPosition(m_player02, startX, startY)  || 
                !m_map.FindObjPosition(m_player01, targetX, targetY)) {

                AfxMessageBox(_T("获取坦克位置失败"));
                break;
            }
            VPath path;
            m_map.FindPath(startX, startY, targetX, targetY, path);
            if (!path.empty()) {
                break;
            }
        }
    }

    return true;
}

// 游戏初始化 : 双人对战 结束
bool CGame::GameInitOne2OneEnd()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////

/* 游戏逻辑处理:
1. 维护子弹状态
2. 维护 机器人AI的自动移动,自动发射子弹
3. 维护 玩家 坦克的状态
以测检测包括： 撞墙，子弹命中坦克...*/
void CGame::GameRunLogic()
{
    // 根据不同阶段调用不同的处理函数
    (this->*m_logicFunc[m_eStep])();
}

// 游戏逻辑处理 : 选择阶段
void CGame::GameRunLogicOnMenu()
{
    // 什么也不做，还没开始游戏
}

// 游戏逻辑处理 : 单人对电脑 菜单阶段
void CGame::GameRunLogicOnOne2BotMenu()
{
    // 如果按下了M键，停止动画状态
    if (KEYDOWN('M')) {
        m_keymenu01.SetStop();
    }

    // 如果都按下了，正式开始游戏
    if (m_keymenu01.GetStop()) {
        SetStep(EGameTypeOne2Bot);
    }
}

// 游戏逻辑处理 : 单人对电脑
void CGame::GameRunLogicOnOne2Bot()
{
    // 状态维护
    // 移动除列表中无效的子弹,并给相应的坦克增加子弹
    RemoveTimeoutBullets();

    // 检查子弹是否击中坦克 :  击中要使坦克爆炸(子弹可不用移除了，
    // 因为游戏都结束了)
    for (auto &blt : m_lstBullets) {
        // 击中机器人
        if (m_bot.IsHitted(blt)) {
            m_bot.Bomb();
            // 游戏结束
            m_eStep = EGameTypeOne2BotEnd;
            blt.SetActive(false);
        }
        // 击中玩家1
        if (m_player01.IsHitted(blt)) {
            m_player01.Bomb();
            // 游戏结束
            m_eStep = EGameTypeOne2BotEnd;
            blt.SetActive(false);
        }
        break;
    }

    // 子弹运动维护
    ProcessHitBullets();

    AI();

    //按键处理
    {
        if (KEYDOWN(VK_LEFT)) {
            m_player01.RotateLeft();
			{
				if (m_map.IsHitTheWall(m_player01, true)) {
					m_player01.ChangeDirection(true);
				}
				else {
					float x, y;
					m_player01.Forward(x,y);
				}
			}
			
        }
        if (KEYDOWN(VK_RIGHT)) {
            m_player01.RotateRight();
			{
				if (m_map.IsHitTheWall(m_player01, true)) {
					m_player01.ChangeDirection(true);
				}
				else {
					float x, y;
					m_player01.Forward(x,y);
				}
			}
        }
        if (KEYDOWN(VK_UP)) {
            // 坦克撞墙检测试
			m_player01.forward();
            {
                if (m_map.IsHitTheWall(m_player01, true)) {
                    m_player01.ChangeDirection(true);
                }
                else {
					float x, y;
                    m_player01.Forward(x,y);
                }
            }
        }
        if (KEYDOWN(VK_DOWN)) {
            {
                // 坦克撞墙检测试
                /*{
                    if (m_map.IsHitTheWall(m_player01, false)) {
                        m_player01.ChangeDirection(true);
                    }
                    else {
                        m_player01.Backward();
                    }
                }*/
				m_player01.Backward();
				{
					if (m_map.IsHitTheWall(m_player01, true)) {
						m_player01.ChangeDirection(true);
					}
					else {
						float x, y;
						m_player01.Forward(x,y);
					}
				}
            }

        }
        if (KEYDOWN('M')) {
            CBullet blt;
			float x, y;
            if (m_player01.Fire(blt,x,y)) {
                // 加入到地图列表中
                m_lstBullets.push_back(blt);
            }
        }
        if (KEYDOWN('I')) {
            // 机器人，玩家 所在的位置
            int startX, startY, targetX, targetY;
            if (!m_map.FindObjPosition(m_bot, startX, startY) || 
                !m_map.FindObjPosition(m_player01, targetX, targetY)) {
                return;
            }
            float fDirNext = 0; //机器人下一步的方向
            if (!m_map.FindNextDirection(&fDirNext, startX, startY, 
                targetX, targetY)) {
                return;
            }

            PointF ptTankCenter = m_bot.GetCenterPoint();
            PointF ptAreaCenter = m_map.GetElementAreaCenter(startX, startY);
            RectF rc(ptAreaCenter.X - 5, ptAreaCenter.Y - 5, 10, 10);


            // 判断 坦克是否己经走到了中心点位置了
            if (!rc.Contains(ptTankCenter)) {
				float x, y;
                m_bot.Forward(x,y); // 没有到达中心点，继续前进
                return;
            }
            else {
                m_bot.SetDirection(fDirNext);
				float x, y;
                m_bot.Forward(x,y);
            }
        }
    }

}

// 游戏逻辑处理 : 单人对电脑 结束
void CGame::GameRunLogicOnOne2BotEnd()
{
    //按键处理
    // 不再接受按键


    // 状态维护
    // 移动除列表中无效的子弹,并给相应的坦克增加子弹
    RemoveTimeoutBullets();


    // 子弹是否撞上墙 : 如果撞上了，改变方向等等
    ProcessHitBullets();
}

// 游戏逻辑处理 : 双人对战 菜单阶段
void CGame::GameRunLogicOnOne2OneMenu()
{
    // 如果按下了M键，停止动画状态   不在做准备 直接进入游戏阶段
   // if (KEYDOWN('M')) {
        m_keymenu01.SetStop();
		//将准备好的匹配信息发送给服务器。
		prome isok;
		PVP *pvp = new PVP;
		Account *acc = new Account;
		acc->set_uid(usesocket.uid);//输入账号uid
		isok.set_allocated_account(acc);

		pvp->set_setout(1);
		isok.set_state(2);
		isok.set_allocated_pvp(pvp);
	    std::string decode_isok = isok.SerializeAsString();
		usesocket.Gamewrite(decode_isok);

  //  }
    // 如果按下了Q键，停止动画状态
  //  if (KEYDOWN('Q')) {
     //   m_keymenu02.SetStop();
    //}
	//接收02玩家准备好的匹配信息。
	std::string code=usesocket.Gameread();
	prome decode;
	decode.ParseFromString(code);

	if (decode.state() == 2)
	{
		m_keymenu02.SetStop();
	}


    // 如果都按下了，正式开始游戏         加入接受服务器准备好的函数
    if (m_keymenu01.GetStop() && m_keymenu02.GetStop()) {
		SetStep(EGameTypeOne2One);
		//hThread = CreateThread(NULL, 0, RecvSer, this, 0, NULL);//创建线程 用于传输接收信息
		//hMutex = CreateMutex(NULL, FALSE,(LPCWSTR)"Mutex");
		//CloseHandle(hThread);//关闭线程句柄
    }
}

// 游戏逻辑处理 : 双人对战
void CGame::GameRunLogicOnOne2One()
{
	//HANDLE hThread = CreateThread(NULL, 0, RecvSer, this, 0, NULL);
    //按键处理
    {
        // 玩家一
        if (KEYDOWN(VK_LEFT)) {
            m_player01.RotateLeft();
			{
				if (m_map.IsHitTheWall(m_player01, true)) {
					m_player01.ChangeDirection(true);
				}
				else {
					float x, y;//获取坦克改变后的位置 传输给服务器
					m_player01.Forward(x,y);
					prome tank;
					Game *t = new Game;
					Account *acc = new Account;
					acc->set_uid(usesocket.uid);//输入账号uid
					tank.set_allocated_account(acc);
					tank.set_state(4);
					t->set_tankx(x);
					t->set_tanky(y);
					t->set_dire(1);
					tank.set_allocated_game(t);
					pp.X = x;//客户端显示坦克位置，用于调试
					pp.Y = y;
					//WaitForSingleObject(hMutex, INFINITE);//互斥锁
				    std::string decode_tank = tank.SerializeAsString();
					//ReleaseMutex(hMutex);//释放互斥锁
					usesocket.Gamewrite(decode_tank);

				}
			}
        }
        if (KEYDOWN(VK_RIGHT)) {
            m_player01.RotateRight();
			{
				if (m_map.IsHitTheWall(m_player01, true)) {
					m_player01.ChangeDirection(true);
				}
				else {
					float x, y;//获取坦克改变后的位置 传输给服务器
					m_player01.Forward(x, y);
					prome tank;
					Game *t = new Game;
					Account *acc = new Account;
					acc->set_uid(usesocket.uid);//输入账号uid
					tank.set_allocated_account(acc);
					tank.set_state(4);
					t->set_tankx(x);
					t->set_tanky(y);
					t->set_dire(2);
					tank.set_allocated_game(t);
					pp.X = x;//客户端显示坦克位置，用于调试
					pp.Y = y;
					//WaitForSingleObject(hMutex, INFINITE);//互斥锁
					 std::string decode_tank02 = tank.SerializeAsString();
					//ReleaseMutex(hMutex);//释放互斥锁
					usesocket.Gamewrite(decode_tank02);
					
				}
			}
        }
        if (KEYDOWN(VK_UP)) {
            // 坦克撞墙检测试
			m_player01.forward();
            {
                if (m_map.IsHitTheWall(m_player01, true)) {
                    m_player01.ChangeDirection(true);
                }
                else {
					float x, y;//获取坦克改变后的位置 传输给服务器
					m_player01.Forward(x, y);
					prome tank;
					Game *t = new Game;
					Account *acc = new Account;
					acc->set_uid(usesocket.uid);//输入账号uid
					tank.set_allocated_account(acc);
					tank.set_state(4);
					t->set_tankx(x);
					t->set_tanky(y);
					t->set_dire(3);
					tank.set_allocated_game(t);
					pp.X = x;//客户端显示坦克位置，用于调试
					pp.Y = y;
					//WaitForSingleObject(hMutex, INFINITE);//互斥锁
					std::string decode_tank02 = tank.SerializeAsString();
					//ReleaseMutex(hMutex);//释放互斥锁
					usesocket.Gamewrite(decode_tank02);
					
                }
            }
        }
        if (KEYDOWN(VK_DOWN)) {
            {
                // 坦克撞墙检测试
               /* {
                    if (m_map.IsHitTheWall(m_player01, false)) {
                        m_player01.ChangeDirection(false);
                    }
                    else {
                        m_player01.Backward();
                    }
                }*/
				m_player01.Backward();
				{
					if (m_map.IsHitTheWall(m_player01, true)) {
						m_player01.ChangeDirection(true);
					}
					else {
						float x, y;//获取坦克改变后的位置 传输给服务器
						m_player01.Forward(x, y);
						prome tank;
						Game *t = new Game;
						Account *acc = new Account;
						acc->set_uid(usesocket.uid);//输入账号uid
						tank.set_allocated_account(acc);
						tank.set_state(4);
						t->set_tankx(x);
						t->set_tanky(y);
						t->set_dire(4);
						tank.set_allocated_game(t);
						pp.X = x;//客户端显示坦克位置，用于调试
						pp.Y = y;
						//WaitForSingleObject(hMutex, INFINITE);//互斥锁
						std::string decode_tank02 = tank.SerializeAsString();
						//ReleaseMutex(hMutex);//释放互斥锁
						usesocket.Gamewrite(decode_tank02);
						
					}
				}
            }
        }
        if (KEYDOWN('M')) {
            CBullet blt;//子弹类
			float bulletx, bullety;
            if (m_player01.Fire(blt,bulletx,bullety)) {
				//加入传输给服务器子弹位置的函数
				//这里可能需要修改优化
                m_lstBullets.push_back(blt);
				prome bulletdata;
				Bullet *bullet = new Bullet;
				Account *acc = new Account;
				acc->set_uid(usesocket.uid);//输入账号uid
				bulletdata.set_allocated_account(acc);
				bulletdata.set_state(5);
				bullet->set_bulletx(bulletx);
				bullet->set_bullety(bullety);
				bulletdata.set_allocated_bullet(bullet);
				std::string decode_bullet1 = bulletdata.SerializeAsString();
				usesocket.Gamewrite(decode_bullet1);
            }
        }
        // 玩家二
       /* if (KEYDOWN('A')) {
            m_player02.RotateLeft();
			{
				if (m_map.IsHitTheWall(m_player02, true)) {
					m_player02.ChangeDirection(true);
				}
				else {
					float x, y;
					m_player02.Forward(x,y);
				}
			}
        }
        if (KEYDOWN('D')) {
            m_player02.RotateRight();
			{
				if (m_map.IsHitTheWall(m_player02, true)) {
					m_player02.ChangeDirection(true);
				}
				else {
					float x, y;
					m_player02.Forward(x, y);
				}
			}
        }
        if (KEYDOWN('W')) {
            // 坦克撞墙检测试
			m_player02.forward();
            {
                if (m_map.IsHitTheWall(m_player02, true)) {
                    m_player02.ChangeDirection(true);
                }
                else {
					float x, y;
					m_player02.Forward(x, y);
                }
            }
        }
        if (KEYDOWN('S')) {
            {
                // 坦克撞墙检测试
               //// {
                    if (m_map.IsHitTheWall(m_player02, false)) {
                        m_player02.ChangeDirection(false);
                    }
                    else {
                        m_player02.Backward();
                    }
                }////
				m_player02.Backward();
				{
					if (m_map.IsHitTheWall(m_player02, true)) {
						m_player02.ChangeDirection(true);
					}
					else {
						float x, y;
						m_player02.Forward(x, y);
					}
				}
            }
        }
        if (KEYDOWN('Q')) {
            CBullet blt;
			float x, y;
            if (m_player02.Fire(blt,x,y)) {
                m_lstBullets.push_back(blt);
            }
        }
		*/
        if (KEYDOWN('Z')) {
            if (m_map.IsCanKillTarget(m_player01, m_player02)) {
                AfxMessageBox(_T("可以打到"));
            }
        }
    }

    // 先判断状态
    // 移动除列表中无效的子弹,并给相应的坦克增加子弹
    RemoveTimeoutBullets();


	//
    // 检查子弹是否击中坦克 :  击中要使坦克爆炸(子弹可不用移除了，因为游戏都结束了)
    for (auto &blt : m_lstBullets) {
		
		
		if (!(blt).IsActive()) {
			
			continue;
		}
			// 击中玩家1
		if (m_player01.IsHitted(blt)) {
			m_player01.Bomb();
			//加入传输给服务器坦克被击中的函数
			prome bomb;
			bomb.set_state(6);
			Account *acc = new Account;
			acc->set_uid(usesocket.uid);//输入账号uid
			bomb.set_allocated_account(acc);
			std::string decode_bomb = bomb.SerializeAsString();
			usesocket.Gamewrite(decode_bomb);

				// 游戏结束
			m_eStep = EGameTypeOne2OneEnd;
			(blt).SetActive(false);
			}
         //击中玩家2
        if (m_player02.IsHitted(blt)) {
           m_player02.Bomb();
		   /*
		 //  TerminateThread(hThread, 00);//终止线程
		 //加入传输给服务器坦克被击中的函数
		   prome bomb;
		   bomb.set_state(4);
		   Account *acc = new Account;
		   acc->set_uid(usesocket.uid);//输入账号uid
		   bomb.set_allocated_account(acc);
		   std::string decode_bomb = bomb.SerializeAsString();
		   usesocket.Gamewrite(decode_bomb);
		   */


            // 游戏结束
            m_eStep = EGameTypeOne2OneEnd;
            blt.SetActive(false);
        }
		
    }
	
    // 撞墙
    ProcessHitBullets();
}

// 游戏逻辑处理 : 双人对战 结束
void CGame::GameRunLogicOnOne2OneEnd()
{
    //按键处理
    // 不需要按键处理
    // 移动除列表中无效的子弹,并给相应的坦克增加子弹
    RemoveTimeoutBullets();

    // 撞墙
    ProcessHitBullets();
}

////////////////////////////////////////////////////////////////////////////////

// 游戏绘图
void CGame::GameRunDraw()
{
    HDC hdc = ::GetDC(m_hWnd);
    // 客户区的大小
    CRect rc;
    GetClientRect(m_hWnd, &rc);

    CDC *dc = CClientDC::FromHandle(hdc);

    // 双缓冲绘图用
    CDC m_dcMemory;
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
    m_dcMemory.CreateCompatibleDC(dc);
    CBitmap *pOldBitmap = m_dcMemory.SelectObject(&bmp);

    // 构造对象
    Graphics gh(m_dcMemory.GetSafeHdc());
    // 清除背景
    gh.Clear(Color::White);
    gh.ResetClip();

    // 画入内存
    (this->*m_drawFunc[m_eStep])(gh);

    // 拷贝到屏幕
    ::BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), 
        m_dcMemory.GetSafeHdc(), 0, 0, SRCCOPY);
    // 释放
    ::ReleaseDC(m_hWnd, hdc);
    return;
}

// 选择阶段
void CGame::GameRunDrawOnMenu(Graphics &gh)
{
    m_menuSelect.Draw(gh);
}

//单人对电脑 : 菜单阶段
void CGame::GameRunDrawOnOne2BotMenu(Graphics &gh)
{
    // 画背景
    m_menu.Draw(gh);

    // 画菜单
    m_keymenu01.Draw(gh);
}

//单人对电脑
void CGame::GameRunDrawOnOne2Bot(Graphics &gh)
{
    // 菜单
    m_menuBackup.Draw(gh);
    // 墙
    m_map.Draw(gh);

    // 玩家
    m_player01.Draw(gh);

    // 机器人
    m_bot.Draw(gh);

    // 画子弹 :  己经发射的
    for (auto b : m_lstBullets) {
        b.Draw(gh);
    }

    // 输出 : FPS
    DrawFps(gh);
}

//单人对电脑 结束
void CGame::GameRunDrawOnOne2BotEnd(Graphics &gh)
{
    // 菜单
    m_menuBackup.Draw(gh);
    // 墙
    m_map.Draw(gh);
    // 玩家
    m_player01.Draw(gh);
    // 机器人
    m_bot.Draw(gh);
    // 画子弹 :  己经发射的
    //for(auto b : m_lstBullets) {
    //    b.Draw(gh);
    //}

    // 输出 : FPS
    DrawFps(gh);

    // 判断游戏整体结束
    if (m_player01.IsBombEnd() || m_bot.IsBombEnd()) {
        m_eStep = EGameTypeMenu;
    }
}

// 双人对战 : 菜单阶段
void CGame::GameRunDrawOnOne2OneMenu(Graphics &gh)
{
    // 画背景
    m_menu.Draw(gh);

    // 画菜单
    m_keymenu01.Draw(gh);
    m_keymenu02.Draw(gh);
}

// 双人对战
void CGame::GameRunDrawOnOne2One(Graphics &gh)
{
    // 菜单
    m_menuBackup.Draw(gh);
    // 墙
    m_map.Draw(gh);

    // 玩家一
    m_player01.Draw(gh);

    // 玩家二
    m_player02.Draw(gh);

    // 画子弹 :  己经发射的
    for (auto b : m_lstBullets) {
        b.Draw(gh);
    }

    // 输出 : FPS
    DrawFps(gh);
}

// 双人对战 结束
void CGame::GameRunDrawOnOne2OneEnd(Graphics &gh)
{
    // 菜单
    m_menuBackup.Draw(gh);
    // 墙
    m_map.Draw(gh);
    // 玩家一
    m_player01.Draw(gh);
    // 玩家二
    m_player02.Draw(gh);
    // 画子弹 :  己经发射的
    for (auto b : m_lstBullets) {
        b.Draw(gh);
    }
    // 输出 : FPS
    DrawFps(gh);

    // 判断游戏整体结束
    if (m_player01.IsBombEnd() || m_player02.IsBombEnd()) {
        m_eStep = EGameTypeMenu;
    }
}

// 画 fps
void CGame::DrawFps(Graphics &gh)
{
    static int fps = 0;
    m_fps++;
    static DWORD dwLast = GetTickCount();
    if (GetTickCount() - dwLast >= 1000) {
        fps = m_fps;
        m_fps = 0;
        dwLast = GetTickCount();
    }

    // 输出fps
    {
        CString s;
        //s.Format(_T("FPS:%d"), fps);
		s.Format(_T("X:%0.0f,Y:%0.0f"), pp.X,pp.Y);
        SolidBrush brush(Color(0x00, 0x00, 0xFF));
        Gdiplus::Font font(_T("宋体"), 10.0);
        CRect rc;
        ::GetClientRect(m_hWnd, &rc);
        // 在右上角显示
        PointF origin(static_cast<float>(rc.left+10), 
            static_cast<float>(rc.top + 2));
        gh.DrawString(s.GetString(), -1, &font, origin, &brush);
    }
}

// 移除超时子弹,并给对应的坦克装弹
void CGame::RemoveTimeoutBullets()
{
    // 定义查找函数
    auto itRemove = std::remove_if(m_lstBullets.begin(), 
        m_lstBullets.end(),
        [](CBullet & blt)->bool {return blt.IsTimeout(); });

    // 把子弹移除,并给对应的坦克增加子弹,
    for (auto it = itRemove; it != m_lstBullets.end(); ++it) {
        // 设置为无效
        it->SetActive(false);
        // 从地图列表中移除
        // 给对应的坦克增加子弹
        it->GetOwner()->AddBullet(*it);
    }
    // 从本地 删除 子弹
    m_lstBullets.erase(itRemove, m_lstBullets.end());
}

// 子弹运动的维护:撞墙拐弯
void CGame::ProcessHitBullets()
{
    // 子弹是否撞上墙 : 如果撞上了，改变方向等等
    for (auto &blt : m_lstBullets) {
        // 进行撞墙处理
        m_map.HitWallProcess(blt);
        blt.Move();
    }
}

// 维护电脑的自动寻路攻击
void CGame::AI()
{
    // 电脑运动状态态维护
    static CGameTimer acTimer(-1, 150);
    if (acTimer.IsTimeval()) {
        // 机器人，玩家 所在的位置
        int startX, startY, targetX, targetY;
        if (!m_map.FindObjPosition(m_bot, startX, startY) || 
            !m_map.FindObjPosition(m_player01, targetX, targetY)) {
            return;
        }
        float fDirNext = 0; //机器人下一步的方向
        if (!m_map.FindNextDirection(&fDirNext, 
            startX, startY, 
            targetX, targetY)) {
            return;
        }

        PointF ptTankCenter = m_bot.GetCenterPoint();
        PointF ptAreaCenter = m_map.GetElementAreaCenter(startX, startY);
        RectF rc(ptAreaCenter.X - 5, ptAreaCenter.Y - 5, 10, 10);


        // 判断 坦克是否己经走到了中心点位置了
        if (!rc.Contains(ptTankCenter)) {
			float x, y;
            m_bot.Forward(x,y); // 没有到达中心点，继续前进
            return;
        }
        else {
            m_bot.SetDirection(fDirNext);
            float dir;
            if (m_map.IsCanKillTarget(m_bot, m_player01, &dir)) {
                CBullet blt;
				float x, y;
                if (m_bot.Fire(blt,x,y)) {
                    m_lstBullets.push_back(blt);
                }
                return;
            }
			float x, y;
            m_bot.Forward(x,y);
        }
    }
}
