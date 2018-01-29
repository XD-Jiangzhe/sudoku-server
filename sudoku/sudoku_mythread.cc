#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Logging.h>

#include "laji.hpp"
#include "sudoku.h"
#include <iostream>
#include <functional>
#include <string>
#include <vector>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;
using namespace tp;

const int kcells = 81;

void solve(const TcpConnectionPtr &conn,
					const string& request,
					const string& id)
{
	string result = solveSudoku(request);
	if(id.empty())
	{
		conn->send(result+"\r\n");
	}
	else
	{
		conn->send(id+":"+result+"\r\n");
	}
}

class SudokuServer{

	public:
	SudokuServer(muduo::net::EventLoop* loop,
				const muduo::net::InetAddress &listenAddr,
				int threadnum);

	void start(){
		server_.start();
		/*threadpood_.start(threadnum_);*/
		tp.start();
	};

	private:
		void onConnection(const muduo::net::TcpConnectionPtr &con);

		void onMessage(const muduo::net::TcpConnectionPtr &con, 
						muduo::net::Buffer* buf,
						muduo::Timestamp time);
		int threadnum_;
		muduo::net::EventLoop *loop_;
		muduo::net::TcpServer server_;
		/*muduo::ThreadPool threadpood_;*/
		tp::threadpool tp;
};

SudokuServer::SudokuServer(muduo::net::EventLoop *loop,
						const muduo::net::InetAddress &listenAddr,
						int threadnum)
	:loop_(loop), server_(loop, listenAddr, "Sudoko"), threadnum_(threadnum), tp(threadnum, "mypool")
{
	server_.setConnectionCallback(bind(&SudokuServer::onConnection, this, _1));
	server_.setMessageCallback(bind(&SudokuServer::onMessage, this, _1, _2, _3));

} 

void SudokuServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	LOG_INFO<<"SudokuServer-"<<conn->peerAddress().toIpPort()<<"->"
		<<conn->localAddress().toIpPort()<<"is"
		<<(conn->connected() ? "UP":"DOWN");
}


void SudokuServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
							muduo::net::Buffer *buf,
							muduo::Timestamp time)
{
	LOG_DEBUG<<conn->name();

	size_t  len = buf->readableBytes();

	while (len >= kcells+2)
	{
		const char* c = buf->findCRLF();
		LOG_INFO<< c-buf->peek() << " "<<buf->readableBytes();
		if(c)
		{
			string id;
			string puzzle;
			
			string request(buf->peek(), c);
			buf->retrieveUntil(c+2);	//将buf中的内容输出
			len -= request.size();

			auto clon = std::find(request.begin(), request.end(), ':');
			if(clon != request.end())
			{
				id = string(request.begin(), clon);
				request.erase(request.begin(), clon+1);			//移除id
			}
			
			if(request.size() == static_cast<size_t>(kCells))
			{
				/*tp.run(std::bind(solve, conn, request, id));	*/	
				tp.push_task(std::bind(solve, conn, request, id));
				//将任务放到线程池中进行
			}
			else
			{
				conn->send("Bad Request!\r\n");
				conn->shutdown();
			}
		}
		else
		{
			break;
		}
	}

}

int main(){
	LOG_INFO<<"pid="<<getpid();

	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(9981);

	SudokuServer server(&loop, listenAddr, 10);
	server.start();
	loop.loop();

}



