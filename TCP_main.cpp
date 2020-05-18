#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <mysql/mysql.h>
#include "mysql_data.h"
#include "message.pb.h"

using namespace std;

void setnonblockingmode(int fd);

struct PVPay {
		    int clntsock;
			int wait;
		    int num;
		   }; 

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	pid_t sock_pid;

	sock_pid=fork();
	if(sock_pid==-1)
	{
		cout<<"pid error";
	}

/* ******* socket********* */

	else if(sock_pid==0)
	{
		PVPay *pvpay=new PVPay[11];
		for(int pnum=0;pnum<11;pnum++)
		{
			pvpay[pnum].clntsock=0;
			pvpay[pnum].wait=0;
			pvpay[pnum].num=0;
		}

		cout<<"1";
		int serv_sock,clnt_sock;
		struct sockaddr_in serv_adr,clnt_adr;
		socklen_t adr_sz;
		int str_len, i;
		char * buf=NULL;
		buf=new char[1024];

		struct epoll_event  *ep_events;
		struct epoll_event event;
		int epfd,event_clt;

		serv_sock=socket(PF_INET,SOCK_STREAM,0);
		memset(&serv_adr,0,sizeof(serv_adr));

		serv_adr.sin_family=AF_INET;
		serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
		serv_adr.sin_port=htons(atoi("7777"));

		int bind_int=0;
		bind_int=bind(serv_sock,(struct sockaddr *)&serv_adr,sizeof(serv_adr));
		if(bind_int==-1)
		{
			cout<<"bind error";
		}

		if(listen(serv_sock,10)==-1)
		{
			cout<<"listen error"<<endl;
		}

		epfd=epoll_create(50);
		ep_events=new epoll_event[50];

		setnonblockingmode(serv_sock);

		event.events=EPOLLIN;
		event.data.fd=serv_sock;
		epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event);

		
		
		while(1)
		{
			event_clt=epoll_wait(epfd,ep_events,50,1);

			if(event_clt==-1)
			{
				cout<<"epoll_wait error"<<endl;
			}

			for(i=0;i<event_clt;i++)
			{
				if(ep_events[i].data.fd==serv_sock)
				{
					adr_sz=sizeof(clnt_adr);
					clnt_sock=accept(serv_sock,(struct sockaddr *)&clnt_adr,&adr_sz);
					setnonblockingmode(clnt_sock);
					event.events=EPOLLIN|EPOLLET;
					event.data.fd=clnt_sock;
					epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);
					cout<<"connected client : "<<clnt_sock<<endl;
				}
				else
				{
					str_len=read(ep_events[i].data.fd,buf,1024);
					string decode_use;
					decode_use.assign(buf,str_len);
					prome code;
					code.ParseFromString(decode_use);
					int state=code.state();
					Game g=code.game();
					float tx=0.0f,ty=0.0f;
					tx=g.tankx();
					ty=g.tanky();
					cout<<"client: "<<ep_events[i].data.fd<<" UID :"<<code.account().uid()<<" X:"<<tx<<"Y:"<<ty<<endl;
					
					// verify account
					if(state==0)
					{
						cout<<"verify account\n";
						MysqlDB mb;
						mb.connect("localhost","root","password","Tankdata");
					//	cout<<code.account().name()<<" and "<<code.account().password()<<endl;
						if(mb.check(code.account().name(),code.account().password()))
						{
							code.set_state(5);
							decode_use=code.SerializeAsString();
							write(ep_events[i].data.fd,decode_use.c_str(),str_len);
							cout<<"password ok\n";

						}
						else
						{
						 	cout<<"password error\n";
							write(ep_events[i].data.fd,decode_use.c_str(),str_len);
						}
					}
					//PVP
					else if(state==1 && code.pvp().setout()==1 )
					{	
						cout<<"PVP \n";
						int pvpnum=0;
						for(int pnum=1;pnum<11;pnum++)
						{
							
							if(pvpay[pnum].num==0 && pvpay[pnum].wait==0)
							{
								pvpay[pnum].clntsock=ep_events[i].data.fd;
								pvpay[pnum].wait=1;
								pvpnum=pnum;
								cout<<"pvping \n";
								break;
							}
							if(pnum==10)
							{
								cout<<"space is full"<<endl;
							}
						}	
						for(int pnum=1;pnum<11;pnum++)
						{
							if(pvpay[pnum].wait==1 && pvpay[pnum].num==0 && pnum!=pvpnum)
							{
								pvpay[pnum].wait=2;
								pvpay[pnum].num=pvpnum;
								pvpay[pvpnum].wait=2;
								pvpay[pvpnum].num=pnum;
								//code.set_state(1);
								//decode_use=code.SerializeAsString();
								write(pvpay[pnum].clntsock,decode_use.c_str(),str_len);
								write(pvpay[pvpnum].clntsock,decode_use.c_str(),str_len);
								cout<<"client:"<<pvpay[pnum].clntsock<<" vs "<<pvpay[pvpnum].clntsock<<endl;
								break;
							}
						}
					}
					//Gaming
					else if(state==2 ||state==3 ||state==4)
					{
						cout<<"Gaming \n";
						for(int pnum=1;pnum<11;pnum++)
						{
							if(pvpay[pnum].clntsock==ep_events[i].data.fd)
							{

								int p2=pvpay[pnum].num;
								if(state==4)
								{
									pvpay[pnum].clntsock=0;
									pvpay[pnum].wait=0;
									pvpay[pnum].num=0;
									pvpay[p2].clntsock=0;
									pvpay[p2].wait=0;
									pvpay[p2].num=0;
								}
								else
								{
								decode_use=code.SerializeAsString();
								write(pvpay[p2].clntsock,decode_use.c_str(),str_len);
								}
							//	cout<<pvpay[pnum].clntsock<<" gaming with "<<pvpay[p2].clntsock<<endl;
							//	float x=g.tankx();
							//	float y=g.tanky();
							//	cout<<"X:"<<x<<"Y: "<<y<<endl;
								break;
							}
						}
						
					}
					
					//Game
				//	write(ep_events[i].data.fd,buf,1024);
				//	epoll_ctl(epfd,EPOLL_CTL_DEL,ep_events[i].data.fd,NULL);
				}
					
			}
		}
		


	}
/* ***************** */	
	else
	{
		cout<<"main running";
	//	MysqlDB mb;
	//	mb.connect("localhost","root","943011","Tankdata");
	//	if(mb.check("liao","liao123"))
	//	{
	//		cout<<"ok";
	//	}

		


		cin.get();
	}

	return 0;
}

void setnonblockingmode(int fd)
{
	int flag=fcntl(fd,F_GETFL,0);
	fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}
