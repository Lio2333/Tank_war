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

HANDLE hMutex = NULL;//����������

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
		//}//�����������̹��λ�á�
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

		//if (fd_num > 0)//������������ͻ�������Ϣ
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
				   // AfxMessageBox(_T("����Player02λ��ʧ��"));
				//}
				//else {  //
				   // m_player02.SetCenterPoint(ptCenter);
				//}
				//���շ��������͵�02̹�˵�λ�ã���ɳ�ʼ����
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
		else if (state == 4)//play2 λ�õĻ�ȡ����
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
			else if (state == 5)//play2 �ӵ�λ�õĻ�ȡ����   //�ӵ���ȡλ�ò���Ҫ��
			{   //���ݿ�ǹ��Ϣ���ӵ��ɱ���01̹������
				//���Խ����Ż�
				CBullet bult;
				Bullet bu = decode.bullet();
				X = bu.bulletx();
				Y = bu.bullety();
				m_player02.Fire(bult, X, Y);//Fire���������Ǹ���̹��λ��ȷ���ӵ�����λ�á�������Ҫ�����ӵ�λ����Ϣ��Fire2����
				m_lstBullets.push_back(bult);
				//RemoveTimeoutBullets();
			}
			//��˫�˴�ս���Ѿ����� 01 02�е��ı�ը ������Ҫ����̹�˱�ը��Ϣ��
			else if (state == 6)//̹�˱�ը
			{
			return;
			//m_player02.Bomb();
			//m_eStep = EGameTypeOne2BotEnd;
			//(blt).SetActive(false);
			}
	}


}


// ����������ڵľ��
void CGame::SetHandle(HWND hWnd)
{
    m_hWnd = hWnd;
}

// ������Ϸ֡
bool CGame::EnterFrame(DWORD dwTime)
{
    GameRunLogic();
    GameRunDraw();
    return false;
}

// ��������ƶ��¼�
void CGame::OnMouseMove(UINT nFlags, CPoint point)
{
    // ѡ��׶�
    if (m_eStep == EGameTypeMenu) {
        // ѡ����Ϸ����
        m_menuSelect.OnMouseMove(nFlags, point);
    }
    else {
        // �������˵�
        m_menuBackup.OnMouseMove(nFlags, point);
    }
}

// �������̧���¼�
void CGame::OnLButtonUp(UINT nFlags, CPoint point)
{
    // ѡ��׶�
    if (m_eStep == EGameTypeMenu) {
        // ѡ����Ϸ����
        m_menuSelect.OnLButtonUp(nFlags, point);
    }
    else {
        // �������˵�
        m_menuBackup.OnLButtonUp(nFlags, point);
    }
}

// ���õ�ǰ��Ϸ�����Ľ׶�
// �����ݲ��� ��ʼ��
void CGame::SetStep(CGame::EGameType step)
{
    m_eStep = step;
    GameInit();
}

/* ��Ϸ��ʼ��
���� ��Ϸ���󣬳�ʼ����ͼ������λ�õ�
*/
bool CGame::GameInit()
{
    //  ��ʼ�������������
    srand(GetTickCount());
    // ���ݲ�ͬ�׶ε��ò�ͬ�Ĵ�����
    return (this->*m_initFunc[m_eStep])();
}

// ��Ϸ��ʼ�� : ѡ��׶�
bool CGame::GameInitMenu()
{
    return true;
}

// ��Ϸ��ʼ�� : ���˶Ե��� �˵��׶�
bool CGame::GameInitOne2BotMenu()
{
    // ���� ���һ�� keyMenu λ�� : ��Ļ���м�
    RECT rc;
    GetWindowRect(m_hWnd, &rc);
    PointF pt;
    pt.X = rc.left + (rc.right - rc.left) / 2.0f;
    pt.Y = rc.top + (rc.bottom - rc.top) / 2.0f;
    m_keymenu01.SetCenterPoint(pt);
    m_keymenu01.SetStop(false);
    return true;
}

// ��Ϸ��ʼ�� : ���˶Ե���
bool CGame::GameInitOne2Bot()
{
    for (; ;) {
        // ��ͼ
        m_map.LoadMap();
        // ���һ
        {
            m_player01 = CPlayer(0, 0, _T("tank_player1.png"));
            PointF ptCenter;
            if (!m_map.FindRandomPosition(ptCenter)) {
                AfxMessageBox(_T("����Player01λ��ʧ��"));
            }
            else {
                m_player01.SetCenterPoint(ptCenter);
            }
        }

        // �о�
        {
            m_bot = CBot(0, 0, _T("tank_bot.png"));
            PointF ptCenter;
            if (!m_map.FindRandomPosition(ptCenter)) {
                AfxMessageBox(_T("����Botλ��ʧ��"));
            }
            else {
                m_bot.SetCenterPoint(ptCenter);
            }
        }
        // �ӵ�
        m_lstBullets.clear();

        // �ж��Ƿ�Ϸ�
        {
            // �����ˣ���� ���ڵ�λ��
            int startX, startY, targetX, targetY;
            if (!m_map.FindObjPosition(m_bot, startX, startY) || 
                !m_map.FindObjPosition(m_player01, targetX, targetY)) {
                AfxMessageBox(_T("��ȡ̹��λ�÷�������"));
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

// ��Ϸ��ʼ�� : ���˶Ե��� ����
bool CGame::GameInitOne2BotEnd()
{

    return true;
}

// ��Ϸ��ʼ�� : ˫�˶�ս �˵��׶�
bool CGame::GameInitOne2OneMenu()
{
	//ֱ�ӽ�����Ϸ ���ٽ�����Ϸѡ��׶�
    // ���� ������ҵ� keyMenu λ�� : ��Ļ���м�
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

// ��Ϸ��ʼ�� : ˫�˶�ս
bool CGame::GameInitOne2One()
{
	hThread = CreateThread(NULL, 0, RecvSer, this, 0, NULL);//�����߳� ���ڴ��������Ϣ
    //hMutex = CreateMutex(NULL, FALSE,(LPCWSTR)"Mutex");
	CloseHandle(hThread);//�ر��߳̾��
    for (;;) {
        // ��ͼ
        m_map.LoadMap();
        //�м����̹��
        {
            m_player01 = CPlayer(0, 0, _T("tank_player1.png"));
            PointF ptCenter;
            if (!m_map.FindRandomPosition(ptCenter)) {
                AfxMessageBox(_T("����Player01λ��ʧ��"));
            }
            else {
                m_player01.SetCenterPoint(ptCenter);
				//���봫�����������̹��λ�õĺ���
				//����̹��λ�õĴ�����
				prome TankP;
				Game *gamedata = new Game;
				Account *ac = new Account;
				ac->set_uid(usesocket.uid);
				TankP.set_state(3);
				gamedata->set_tankx(ptCenter.X);
				gamedata->set_tanky(ptCenter.Y);
				pp.X = gamedata->tankx();
				pp.Y = gamedata->tanky();
				inittank.X= gamedata->tankx();//�����߳��г�ʼ��̹��λ��
				inittank.Y= gamedata->tanky();//�����߳��г�ʼ��̹��λ��
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
               // AfxMessageBox(_T("����Player02λ��ʧ��"));
            //}
            //else {  //
               // m_player02.SetCenterPoint(ptCenter);
            //}
			//���շ��������͵�02̹�˵�λ�ã���ɳ�ʼ����
			std::string tankp_code;
			prome tankp;
			tankp_code=usesocket.Gameread();
			tankp.ParseFromString(tankp_code);
			ptCenter.X = tankp.game().tankx();
			ptCenter.Y = tankp.game().tanky();

			m_player02.SetCenterPoint(ptCenter);


        }*/

        // �ӵ�
        m_lstBullets.clear();
        // �ж��Ƿ�Ϸ�
      {
            // ��� ���ڵ�λ��
			Sleep(200);
            int startX, startY, targetX, targetY;
            if (!m_map.FindObjPosition(m_player02, startX, startY)  || 
                !m_map.FindObjPosition(m_player01, targetX, targetY)) {

                AfxMessageBox(_T("��ȡ̹��λ��ʧ��"));
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

// ��Ϸ��ʼ�� : ˫�˶�ս ����
bool CGame::GameInitOne2OneEnd()
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////

/* ��Ϸ�߼�����:
1. ά���ӵ�״̬
2. ά�� ������AI���Զ��ƶ�,�Զ������ӵ�
3. ά�� ��� ̹�˵�״̬
�Բ�������� ײǽ���ӵ�����̹��...*/
void CGame::GameRunLogic()
{
    // ���ݲ�ͬ�׶ε��ò�ͬ�Ĵ�����
    (this->*m_logicFunc[m_eStep])();
}

// ��Ϸ�߼����� : ѡ��׶�
void CGame::GameRunLogicOnMenu()
{
    // ʲôҲ��������û��ʼ��Ϸ
}

// ��Ϸ�߼����� : ���˶Ե��� �˵��׶�
void CGame::GameRunLogicOnOne2BotMenu()
{
    // ���������M����ֹͣ����״̬
    if (KEYDOWN('M')) {
        m_keymenu01.SetStop();
    }

    // ����������ˣ���ʽ��ʼ��Ϸ
    if (m_keymenu01.GetStop()) {
        SetStep(EGameTypeOne2Bot);
    }
}

// ��Ϸ�߼����� : ���˶Ե���
void CGame::GameRunLogicOnOne2Bot()
{
    // ״̬ά��
    // �ƶ����б�����Ч���ӵ�,������Ӧ��̹�������ӵ�
    RemoveTimeoutBullets();

    // ����ӵ��Ƿ����̹�� :  ����Ҫʹ̹�˱�ը(�ӵ��ɲ����Ƴ��ˣ�
    // ��Ϊ��Ϸ��������)
    for (auto &blt : m_lstBullets) {
        // ���л�����
        if (m_bot.IsHitted(blt)) {
            m_bot.Bomb();
            // ��Ϸ����
            m_eStep = EGameTypeOne2BotEnd;
            blt.SetActive(false);
        }
        // �������1
        if (m_player01.IsHitted(blt)) {
            m_player01.Bomb();
            // ��Ϸ����
            m_eStep = EGameTypeOne2BotEnd;
            blt.SetActive(false);
        }
        break;
    }

    // �ӵ��˶�ά��
    ProcessHitBullets();

    AI();

    //��������
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
            // ̹��ײǽ�����
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
                // ̹��ײǽ�����
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
                // ���뵽��ͼ�б���
                m_lstBullets.push_back(blt);
            }
        }
        if (KEYDOWN('I')) {
            // �����ˣ���� ���ڵ�λ��
            int startX, startY, targetX, targetY;
            if (!m_map.FindObjPosition(m_bot, startX, startY) || 
                !m_map.FindObjPosition(m_player01, targetX, targetY)) {
                return;
            }
            float fDirNext = 0; //��������һ���ķ���
            if (!m_map.FindNextDirection(&fDirNext, startX, startY, 
                targetX, targetY)) {
                return;
            }

            PointF ptTankCenter = m_bot.GetCenterPoint();
            PointF ptAreaCenter = m_map.GetElementAreaCenter(startX, startY);
            RectF rc(ptAreaCenter.X - 5, ptAreaCenter.Y - 5, 10, 10);


            // �ж� ̹���Ƿ񼺾��ߵ������ĵ�λ����
            if (!rc.Contains(ptTankCenter)) {
				float x, y;
                m_bot.Forward(x,y); // û�е������ĵ㣬����ǰ��
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

// ��Ϸ�߼����� : ���˶Ե��� ����
void CGame::GameRunLogicOnOne2BotEnd()
{
    //��������
    // ���ٽ��ܰ���


    // ״̬ά��
    // �ƶ����б�����Ч���ӵ�,������Ӧ��̹�������ӵ�
    RemoveTimeoutBullets();


    // �ӵ��Ƿ�ײ��ǽ : ���ײ���ˣ��ı䷽��ȵ�
    ProcessHitBullets();
}

// ��Ϸ�߼����� : ˫�˶�ս �˵��׶�
void CGame::GameRunLogicOnOne2OneMenu()
{
    // ���������M����ֹͣ����״̬   ������׼�� ֱ�ӽ�����Ϸ�׶�
   // if (KEYDOWN('M')) {
        m_keymenu01.SetStop();
		//��׼���õ�ƥ����Ϣ���͸���������
		prome isok;
		PVP *pvp = new PVP;
		Account *acc = new Account;
		acc->set_uid(usesocket.uid);//�����˺�uid
		isok.set_allocated_account(acc);

		pvp->set_setout(1);
		isok.set_state(2);
		isok.set_allocated_pvp(pvp);
	    std::string decode_isok = isok.SerializeAsString();
		usesocket.Gamewrite(decode_isok);

  //  }
    // ���������Q����ֹͣ����״̬
  //  if (KEYDOWN('Q')) {
     //   m_keymenu02.SetStop();
    //}
	//����02���׼���õ�ƥ����Ϣ��
	std::string code=usesocket.Gameread();
	prome decode;
	decode.ParseFromString(code);

	if (decode.state() == 2)
	{
		m_keymenu02.SetStop();
	}


    // ����������ˣ���ʽ��ʼ��Ϸ         ������ܷ�����׼���õĺ���
    if (m_keymenu01.GetStop() && m_keymenu02.GetStop()) {
		SetStep(EGameTypeOne2One);
		//hThread = CreateThread(NULL, 0, RecvSer, this, 0, NULL);//�����߳� ���ڴ��������Ϣ
		//hMutex = CreateMutex(NULL, FALSE,(LPCWSTR)"Mutex");
		//CloseHandle(hThread);//�ر��߳̾��
    }
}

// ��Ϸ�߼����� : ˫�˶�ս
void CGame::GameRunLogicOnOne2One()
{
	//HANDLE hThread = CreateThread(NULL, 0, RecvSer, this, 0, NULL);
    //��������
    {
        // ���һ
        if (KEYDOWN(VK_LEFT)) {
            m_player01.RotateLeft();
			{
				if (m_map.IsHitTheWall(m_player01, true)) {
					m_player01.ChangeDirection(true);
				}
				else {
					float x, y;//��ȡ̹�˸ı���λ�� �����������
					m_player01.Forward(x,y);
					prome tank;
					Game *t = new Game;
					Account *acc = new Account;
					acc->set_uid(usesocket.uid);//�����˺�uid
					tank.set_allocated_account(acc);
					tank.set_state(4);
					t->set_tankx(x);
					t->set_tanky(y);
					t->set_dire(1);
					tank.set_allocated_game(t);
					pp.X = x;//�ͻ�����ʾ̹��λ�ã����ڵ���
					pp.Y = y;
					//WaitForSingleObject(hMutex, INFINITE);//������
				    std::string decode_tank = tank.SerializeAsString();
					//ReleaseMutex(hMutex);//�ͷŻ�����
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
					float x, y;//��ȡ̹�˸ı���λ�� �����������
					m_player01.Forward(x, y);
					prome tank;
					Game *t = new Game;
					Account *acc = new Account;
					acc->set_uid(usesocket.uid);//�����˺�uid
					tank.set_allocated_account(acc);
					tank.set_state(4);
					t->set_tankx(x);
					t->set_tanky(y);
					t->set_dire(2);
					tank.set_allocated_game(t);
					pp.X = x;//�ͻ�����ʾ̹��λ�ã����ڵ���
					pp.Y = y;
					//WaitForSingleObject(hMutex, INFINITE);//������
					 std::string decode_tank02 = tank.SerializeAsString();
					//ReleaseMutex(hMutex);//�ͷŻ�����
					usesocket.Gamewrite(decode_tank02);
					
				}
			}
        }
        if (KEYDOWN(VK_UP)) {
            // ̹��ײǽ�����
			m_player01.forward();
            {
                if (m_map.IsHitTheWall(m_player01, true)) {
                    m_player01.ChangeDirection(true);
                }
                else {
					float x, y;//��ȡ̹�˸ı���λ�� �����������
					m_player01.Forward(x, y);
					prome tank;
					Game *t = new Game;
					Account *acc = new Account;
					acc->set_uid(usesocket.uid);//�����˺�uid
					tank.set_allocated_account(acc);
					tank.set_state(4);
					t->set_tankx(x);
					t->set_tanky(y);
					t->set_dire(3);
					tank.set_allocated_game(t);
					pp.X = x;//�ͻ�����ʾ̹��λ�ã����ڵ���
					pp.Y = y;
					//WaitForSingleObject(hMutex, INFINITE);//������
					std::string decode_tank02 = tank.SerializeAsString();
					//ReleaseMutex(hMutex);//�ͷŻ�����
					usesocket.Gamewrite(decode_tank02);
					
                }
            }
        }
        if (KEYDOWN(VK_DOWN)) {
            {
                // ̹��ײǽ�����
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
						float x, y;//��ȡ̹�˸ı���λ�� �����������
						m_player01.Forward(x, y);
						prome tank;
						Game *t = new Game;
						Account *acc = new Account;
						acc->set_uid(usesocket.uid);//�����˺�uid
						tank.set_allocated_account(acc);
						tank.set_state(4);
						t->set_tankx(x);
						t->set_tanky(y);
						t->set_dire(4);
						tank.set_allocated_game(t);
						pp.X = x;//�ͻ�����ʾ̹��λ�ã����ڵ���
						pp.Y = y;
						//WaitForSingleObject(hMutex, INFINITE);//������
						std::string decode_tank02 = tank.SerializeAsString();
						//ReleaseMutex(hMutex);//�ͷŻ�����
						usesocket.Gamewrite(decode_tank02);
						
					}
				}
            }
        }
        if (KEYDOWN('M')) {
            CBullet blt;//�ӵ���
			float bulletx, bullety;
            if (m_player01.Fire(blt,bulletx,bullety)) {
				//���봫����������ӵ�λ�õĺ���
				//���������Ҫ�޸��Ż�
                m_lstBullets.push_back(blt);
				prome bulletdata;
				Bullet *bullet = new Bullet;
				Account *acc = new Account;
				acc->set_uid(usesocket.uid);//�����˺�uid
				bulletdata.set_allocated_account(acc);
				bulletdata.set_state(5);
				bullet->set_bulletx(bulletx);
				bullet->set_bullety(bullety);
				bulletdata.set_allocated_bullet(bullet);
				std::string decode_bullet1 = bulletdata.SerializeAsString();
				usesocket.Gamewrite(decode_bullet1);
            }
        }
        // ��Ҷ�
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
            // ̹��ײǽ�����
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
                // ̹��ײǽ�����
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
                AfxMessageBox(_T("���Դ�"));
            }
        }
    }

    // ���ж�״̬
    // �ƶ����б�����Ч���ӵ�,������Ӧ��̹�������ӵ�
    RemoveTimeoutBullets();


	//
    // ����ӵ��Ƿ����̹�� :  ����Ҫʹ̹�˱�ը(�ӵ��ɲ����Ƴ��ˣ���Ϊ��Ϸ��������)
    for (auto &blt : m_lstBullets) {
		
		
		if (!(blt).IsActive()) {
			
			continue;
		}
			// �������1
		if (m_player01.IsHitted(blt)) {
			m_player01.Bomb();
			//���봫���������̹�˱����еĺ���
			prome bomb;
			bomb.set_state(6);
			Account *acc = new Account;
			acc->set_uid(usesocket.uid);//�����˺�uid
			bomb.set_allocated_account(acc);
			std::string decode_bomb = bomb.SerializeAsString();
			usesocket.Gamewrite(decode_bomb);

				// ��Ϸ����
			m_eStep = EGameTypeOne2OneEnd;
			(blt).SetActive(false);
			}
         //�������2
        if (m_player02.IsHitted(blt)) {
           m_player02.Bomb();
		   /*
		 //  TerminateThread(hThread, 00);//��ֹ�߳�
		 //���봫���������̹�˱����еĺ���
		   prome bomb;
		   bomb.set_state(4);
		   Account *acc = new Account;
		   acc->set_uid(usesocket.uid);//�����˺�uid
		   bomb.set_allocated_account(acc);
		   std::string decode_bomb = bomb.SerializeAsString();
		   usesocket.Gamewrite(decode_bomb);
		   */


            // ��Ϸ����
            m_eStep = EGameTypeOne2OneEnd;
            blt.SetActive(false);
        }
		
    }
	
    // ײǽ
    ProcessHitBullets();
}

// ��Ϸ�߼����� : ˫�˶�ս ����
void CGame::GameRunLogicOnOne2OneEnd()
{
    //��������
    // ����Ҫ��������
    // �ƶ����б�����Ч���ӵ�,������Ӧ��̹�������ӵ�
    RemoveTimeoutBullets();

    // ײǽ
    ProcessHitBullets();
}

////////////////////////////////////////////////////////////////////////////////

// ��Ϸ��ͼ
void CGame::GameRunDraw()
{
    HDC hdc = ::GetDC(m_hWnd);
    // �ͻ����Ĵ�С
    CRect rc;
    GetClientRect(m_hWnd, &rc);

    CDC *dc = CClientDC::FromHandle(hdc);

    // ˫�����ͼ��
    CDC m_dcMemory;
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
    m_dcMemory.CreateCompatibleDC(dc);
    CBitmap *pOldBitmap = m_dcMemory.SelectObject(&bmp);

    // �������
    Graphics gh(m_dcMemory.GetSafeHdc());
    // �������
    gh.Clear(Color::White);
    gh.ResetClip();

    // �����ڴ�
    (this->*m_drawFunc[m_eStep])(gh);

    // ��������Ļ
    ::BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), 
        m_dcMemory.GetSafeHdc(), 0, 0, SRCCOPY);
    // �ͷ�
    ::ReleaseDC(m_hWnd, hdc);
    return;
}

// ѡ��׶�
void CGame::GameRunDrawOnMenu(Graphics &gh)
{
    m_menuSelect.Draw(gh);
}

//���˶Ե��� : �˵��׶�
void CGame::GameRunDrawOnOne2BotMenu(Graphics &gh)
{
    // ������
    m_menu.Draw(gh);

    // ���˵�
    m_keymenu01.Draw(gh);
}

//���˶Ե���
void CGame::GameRunDrawOnOne2Bot(Graphics &gh)
{
    // �˵�
    m_menuBackup.Draw(gh);
    // ǽ
    m_map.Draw(gh);

    // ���
    m_player01.Draw(gh);

    // ������
    m_bot.Draw(gh);

    // ���ӵ� :  ���������
    for (auto b : m_lstBullets) {
        b.Draw(gh);
    }

    // ��� : FPS
    DrawFps(gh);
}

//���˶Ե��� ����
void CGame::GameRunDrawOnOne2BotEnd(Graphics &gh)
{
    // �˵�
    m_menuBackup.Draw(gh);
    // ǽ
    m_map.Draw(gh);
    // ���
    m_player01.Draw(gh);
    // ������
    m_bot.Draw(gh);
    // ���ӵ� :  ���������
    //for(auto b : m_lstBullets) {
    //    b.Draw(gh);
    //}

    // ��� : FPS
    DrawFps(gh);

    // �ж���Ϸ�������
    if (m_player01.IsBombEnd() || m_bot.IsBombEnd()) {
        m_eStep = EGameTypeMenu;
    }
}

// ˫�˶�ս : �˵��׶�
void CGame::GameRunDrawOnOne2OneMenu(Graphics &gh)
{
    // ������
    m_menu.Draw(gh);

    // ���˵�
    m_keymenu01.Draw(gh);
    m_keymenu02.Draw(gh);
}

// ˫�˶�ս
void CGame::GameRunDrawOnOne2One(Graphics &gh)
{
    // �˵�
    m_menuBackup.Draw(gh);
    // ǽ
    m_map.Draw(gh);

    // ���һ
    m_player01.Draw(gh);

    // ��Ҷ�
    m_player02.Draw(gh);

    // ���ӵ� :  ���������
    for (auto b : m_lstBullets) {
        b.Draw(gh);
    }

    // ��� : FPS
    DrawFps(gh);
}

// ˫�˶�ս ����
void CGame::GameRunDrawOnOne2OneEnd(Graphics &gh)
{
    // �˵�
    m_menuBackup.Draw(gh);
    // ǽ
    m_map.Draw(gh);
    // ���һ
    m_player01.Draw(gh);
    // ��Ҷ�
    m_player02.Draw(gh);
    // ���ӵ� :  ���������
    for (auto b : m_lstBullets) {
        b.Draw(gh);
    }
    // ��� : FPS
    DrawFps(gh);

    // �ж���Ϸ�������
    if (m_player01.IsBombEnd() || m_player02.IsBombEnd()) {
        m_eStep = EGameTypeMenu;
    }
}

// �� fps
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

    // ���fps
    {
        CString s;
        //s.Format(_T("FPS:%d"), fps);
		s.Format(_T("X:%0.0f,Y:%0.0f"), pp.X,pp.Y);
        SolidBrush brush(Color(0x00, 0x00, 0xFF));
        Gdiplus::Font font(_T("����"), 10.0);
        CRect rc;
        ::GetClientRect(m_hWnd, &rc);
        // �����Ͻ���ʾ
        PointF origin(static_cast<float>(rc.left+10), 
            static_cast<float>(rc.top + 2));
        gh.DrawString(s.GetString(), -1, &font, origin, &brush);
    }
}

// �Ƴ���ʱ�ӵ�,������Ӧ��̹��װ��
void CGame::RemoveTimeoutBullets()
{
    // ������Һ���
    auto itRemove = std::remove_if(m_lstBullets.begin(), 
        m_lstBullets.end(),
        [](CBullet & blt)->bool {return blt.IsTimeout(); });

    // ���ӵ��Ƴ�,������Ӧ��̹�������ӵ�,
    for (auto it = itRemove; it != m_lstBullets.end(); ++it) {
        // ����Ϊ��Ч
        it->SetActive(false);
        // �ӵ�ͼ�б����Ƴ�
        // ����Ӧ��̹�������ӵ�
        it->GetOwner()->AddBullet(*it);
    }
    // �ӱ��� ɾ�� �ӵ�
    m_lstBullets.erase(itRemove, m_lstBullets.end());
}

// �ӵ��˶���ά��:ײǽ����
void CGame::ProcessHitBullets()
{
    // �ӵ��Ƿ�ײ��ǽ : ���ײ���ˣ��ı䷽��ȵ�
    for (auto &blt : m_lstBullets) {
        // ����ײǽ����
        m_map.HitWallProcess(blt);
        blt.Move();
    }
}

// ά�����Ե��Զ�Ѱ·����
void CGame::AI()
{
    // �����˶�״̬̬ά��
    static CGameTimer acTimer(-1, 150);
    if (acTimer.IsTimeval()) {
        // �����ˣ���� ���ڵ�λ��
        int startX, startY, targetX, targetY;
        if (!m_map.FindObjPosition(m_bot, startX, startY) || 
            !m_map.FindObjPosition(m_player01, targetX, targetY)) {
            return;
        }
        float fDirNext = 0; //��������һ���ķ���
        if (!m_map.FindNextDirection(&fDirNext, 
            startX, startY, 
            targetX, targetY)) {
            return;
        }

        PointF ptTankCenter = m_bot.GetCenterPoint();
        PointF ptAreaCenter = m_map.GetElementAreaCenter(startX, startY);
        RectF rc(ptAreaCenter.X - 5, ptAreaCenter.Y - 5, 10, 10);


        // �ж� ̹���Ƿ񼺾��ߵ������ĵ�λ����
        if (!rc.Contains(ptTankCenter)) {
			float x, y;
            m_bot.Forward(x,y); // û�е������ĵ㣬����ǰ��
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
