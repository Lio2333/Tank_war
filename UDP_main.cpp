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


struct PVPay {
		    int oneuid;
			int oppo;
		    struct sockaddr_in one_adr;
			int pstate;
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
			pvpay[pnum].oneuid=-1;
			pvpay[pnum].oppo=-1;
			pvpay[pnum].pstate=0;
		}

		int serv_sock;
		struct sockaddr_in serv_adr;
		socklen_t adr_sz,adr_len;
		int str_len;
		int i=0;
		char * buf=NULL;
		buf=new char[1024];

		serv_sock=socket(PF_INET,SOCK_DGRAM,0);
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
		//adr_sz=sizeof(clnt_adr);
		
		while(1)
		{
			struct sockaddr_in clnt_adr;
			adr_sz=sizeof(clnt_adr);
			//adr_len=sizeof(clnt_adr); 

			str_len=recvfrom(serv_sock,buf,1024,0,(struct sockaddr *)&clnt_adr,&adr_sz);

			string decode_use;
			decode_use.assign(buf,str_len);
			prome code;
			code.ParseFromString(decode_use);
			int state=code.state();
			cout<<"\nnew text: "<<code.account().uid()<<endl;
			// verify account
			if(state==0)
			{
				int uid;
				cout<<"verify account\n";
				MysqlDB mb;
				mb.connect("localhost","root","password","Tankdata");
				//	cout<<code.account().name()<<" and "<<code.account().password()<<endl;
				uid=mb.checkuid(code.account().name(),code.account().password());
				cout<<"UID: "<<uid<<endl;
				if(uid!=-1)
				{
					for(int pnum=0;pnum<11;pnum++)
					{
						if(pvpay[pnum].pstate==0)
						{
							pvpay[pnum].oneuid=uid;
							pvpay[pnum].one_adr=clnt_adr;
							pvpay[pnum].pstate=1;
							break;
						}

					}	 
					Account *acc=new Account;
					acc->set_uid(uid);
					code.set_allocated_account(acc);
					code.set_state(1);
					decode_use=code.SerializeAsString();
					sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr*)&clnt_adr,adr_sz);
					cout<<"password ok\n";


				}
				else
				{
					cout<<"password error\n";
					sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr*)&clnt_adr,adr_sz);
				}
			}
			//PVP
			else if(state==2)
			{	
				cout<<"PVP \n";
				int pvpnum=0;
				for(int pnum=0;pnum<11;pnum++)
				{
					if(pvpay[pnum].oneuid==code.account().uid() && pvpay[pnum].pstate==1)
					{
						
						pvpnum=pnum;
						pvpay[pnum].pstate=2;
						break;
					}
					if(pvpay[10].pstate==2)
					{
						cout<<"pvp is full"<<endl;
					}
				}
				for(int pnum=0;pnum<11;pnum++)
				{
					cout<<"pvping"<<endl;
					cout<<pnum<<" UID: "<<pvpay[pnum].oneuid<<" oppo: "<<pvpay[pnum].oppo<<" pstate: "<<pvpay[pnum].pstate<<endl;
					if(pnum !=pvpnum && pvpay[pnum].pstate==2 && pvpay[pnum].oppo==-1)
					{
						pvpay[pnum].oppo=pvpnum;
						pvpay[pnum].pstate=3;

						pvpay[pvpnum].oppo=pnum;
						pvpay[pvpnum].pstate=3;

						cout<<pvpay[pnum].oneuid<<" VS "<<pvpay[pvpnum].oneuid<<endl;

					    code.set_state(2);
						decode_use=code.SerializeAsString();
						int i=0;
						adr_len=sizeof(pvpay[pnum].one_adr);
						i=sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr *)&pvpay[pnum].one_adr,adr_len);

					    int j=0;
					    j=sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr *)&clnt_adr,adr_sz);
						cout<<"i: "<<i<<"j: "<<j<<endl;

						cout<<"PNUM-UID:"<<pvpay[pnum].oneuid<<" OPPO:"<<pvpay[pnum].oppo<<endl;
						cout<<"PVPNUM-UID:"<<pvpay[pvpnum].oneuid<<" OPPO:"<<pvpay[pvpnum].oppo<<endl;
						break;
					}	
				}
			}
			//Gaming
			else if(state==3 ||state==4 ||state==5)
			{
				cout<<"Gaming \n";
				for(int pnum=0;pnum<11;pnum++)
				{
					if(pvpay[pnum].oneuid==code.account().uid())
					{

						int p2=pvpay[pnum].oppo;
						//decode_use=code.SerializeAsString();
						cout<<code.account().uid()<<" VSing "<<pvpay[p2].oneuid<<endl;

						sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr *)&(pvpay[p2].one_adr),adr_sz);
						
						//	cout<<pvpay[pnum].clntsock<<" gaming with "<<pvpay[p2].clntsock<<endl;
						
						//	float x=g.tankx();
						//	float y=g.tanky();
						//	cout<<"X:"<<x<<"Y: "<<y<<endl;
						break;
					}
				}
						
			}
			//Game over
			else if(state==6)
			{
				cout<<"Gmae over"<<endl;
				int p2;
				for(int pnum=0;pnum<11;pnum++)
				{
					if(pvpay[pnum].oneuid==code.account().uid())
					{
						
						p2=pvpay[pnum].oppo;
						adr_len=sizeof(pvpay[p2].one_adr);
						sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr *)&pvpay[p2].one_adr,adr_len);
						sendto(serv_sock,decode_use.c_str(),1024,0,(struct sockaddr *)&pvpay[pnum].one_adr,adr_sz);
						pvpay[pnum].oppo=-1;
						pvpay[pnum].pstate=1;
						pvpay[p2].oppo=-1;
						pvpay[p2].pstate=1;
						break;
					}

				}
			}
			//exit
			else if(state==7)
			{
				cout<<"exit"<<endl;
				for(int pnum=0;pnum<11;pnum++)
				{
					if(pvpay[pnum].oneuid==code.account().uid())
					{
						pvpay[pnum].oppo=-1;
						pvpay[pnum].pstate=0;
						pvpay[pnum].oneuid=-1;
						memset(&pvpay[pnum].one_adr,0,sizeof(pvpay[pnum].one_adr));
						break;
					}
				}

			}

					
					//Game
				//	write(ep_events[i].data.fd,buf,1024);
				//	epoll_ctl(epfd,EPOLL_CTL_DEL,ep_events[i].data.fd,NULL)
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

